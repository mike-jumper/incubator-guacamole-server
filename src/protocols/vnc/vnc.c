/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "config.h"

#include "backend/callbacks.h"
#include "backend/client.h"
#include "backend/clipboard.h"
#include "backend/framebuffer.h"
#include "backend/settings.h"
#include "client.h"
#include "clipboard.h"
#include "common/cursor.h"
#include "common/display.h"
#include "common/recording.h"
#include "cursor.h"
#include "display.h"
#include "settings.h"
#include "vnc.h"

#ifdef ENABLE_PULSE
#include "pulse/pulse.h"
#endif

#ifdef ENABLE_COMMON_SSH
#include "common-ssh/sftp.h"
#include "common-ssh/ssh.h"
#include "sftp.h"
#endif

#include <guacamole/client.h>
#include <guacamole/protocol.h>
#include <guacamole/socket.h>
#include <guacamole/timestamp.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

static guac_vnc_backend_client* guac_vnc_connect(guac_client* client,
        guac_vnc_settings* settings) {

    guac_vnc_backend_callbacks callbacks = {
        .clipboard_received = guac_vnc_clipboard_received,
        .cursor_updated = guac_vnc_cursor_updated,
        .framebuffer_resized = guac_vnc_framebuffer_resized,
        .framebuffer_copied = guac_vnc_framebuffer_copied,
        .framebuffer_updated = guac_vnc_framebuffer_updated,
        .data = client
    };

    /* Copy settings which apply to VNC backend */
    guac_vnc_backend_settings backend_settings = {
        .hostname = settings->hostname,
        .port = settings->port,
        .password = settings->password,
        .encodings = settings->encodings,
        .swap_red_blue = settings->swap_red_blue,
        .color_depth = settings->color_depth,
        .read_only = settings->read_only,
        .dest_host = settings->dest_host,
        .dest_port = settings->dest_port,
        .reverse_connect = settings->reverse_connect,
        .listen_timeout = settings->listen_timeout,
        .remote_cursor = settings->remote_cursor
    };

    /* Make initial connection attempt */
    guac_vnc_backend_client* backend_client = guac_vnc_backend_client_create(
            client, &backend_settings, &callbacks);

    /* If unsuccessful, retry as many times as specified */
    for (int retries_remaining = settings->retries;
            !backend_client && retries_remaining > 0; retries_remaining--) {

        guac_client_log(client, GUAC_LOG_INFO,
                "Connect failed. Waiting %ims before retrying...",
                GUAC_VNC_CONNECT_INTERVAL);

        /* Wait for given interval then retry */
        guac_timestamp_msleep(GUAC_VNC_CONNECT_INTERVAL);
        backend_client = guac_vnc_backend_client_create(client,
                &backend_settings, &callbacks);

    }

    return backend_client;

}

void* guac_vnc_client_thread(void* data) {

    guac_client* client = (guac_client*) data;
    guac_vnc_client* vnc_client = (guac_vnc_client*) client->data;
    guac_vnc_settings* settings = vnc_client->settings;

    /* Ensure connection is kept alive during lengthy connects */
    guac_socket_require_keep_alive(client->socket);

    /* Attempt to connect to VNC server */
    guac_vnc_backend_client* backend_client = vnc_client->backend_client =
        guac_vnc_connect(client, settings);

    /* Abort immediately if unable to connect */
    if (!backend_client) {
        guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_NOT_FOUND,
                "Unable to connect to VNC server.");
        return NULL;
    }

    /* Retrieve encoding to be used for clipboard data */
    const char* clipboard_encoding = guac_vnc_backend_get_clipboard_encoding(backend_client);
    if (clipboard_encoding == NULL)
        clipboard_encoding = settings->clipboard_encoding;

    /* Configure clipboard encoding */
    if (guac_vnc_set_clipboard_encoding(client, clipboard_encoding)) {
        guac_client_log(client, GUAC_LOG_INFO, "Using non-standard VNC "
                "clipboard encoding: '%s'.", clipboard_encoding);
    }

#ifdef ENABLE_PULSE
    /* If audio is enabled, start streaming via PulseAudio */
    if (settings->audio_enabled)
        vnc_client->audio = guac_pa_stream_alloc(client, 
                settings->pa_servername);
#endif

#ifdef ENABLE_COMMON_SSH
    guac_common_ssh_init(client);

    /* Connect via SSH if SFTP is enabled */
    if (settings->enable_sftp) {

        /* Abort if username is missing */
        if (settings->sftp_username == NULL) {
            guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR,
                    "SFTP username is required if SFTP is enabled.");
            return NULL;
        }

        guac_client_log(client, GUAC_LOG_DEBUG,
                "Connecting via SSH for SFTP filesystem access.");

        vnc_client->sftp_user =
            guac_common_ssh_create_user(settings->sftp_username);

        /* Import private key, if given */
        if (settings->sftp_private_key != NULL) {

            guac_client_log(client, GUAC_LOG_DEBUG,
                    "Authenticating with private key.");

            /* Abort if private key cannot be read */
            if (guac_common_ssh_user_import_key(vnc_client->sftp_user,
                        settings->sftp_private_key,
                        settings->sftp_passphrase)) {
                guac_client_abort(client, GUAC_PROTOCOL_STATUS_SERVER_ERROR,
                        "Private key unreadable.");
                return NULL;
            }

        }

        /* Otherwise, use specified password */
        else {
            guac_client_log(client, GUAC_LOG_DEBUG,
                    "Authenticating with password.");
            guac_common_ssh_user_set_password(vnc_client->sftp_user,
                    settings->sftp_password);
        }

        /* Attempt SSH connection */
        vnc_client->sftp_session =
            guac_common_ssh_create_session(client, settings->sftp_hostname,
                    settings->sftp_port, vnc_client->sftp_user, settings->sftp_server_alive_interval,
                    settings->sftp_host_key);

        /* Fail if SSH connection does not succeed */
        if (vnc_client->sftp_session == NULL) {
            /* Already aborted within guac_common_ssh_create_session() */
            return NULL;
        }

        /* Load filesystem */
        vnc_client->sftp_filesystem =
            guac_common_ssh_create_sftp_filesystem(vnc_client->sftp_session,
                    settings->sftp_root_directory, NULL);

        /* Expose filesystem to connection owner */
        guac_client_for_owner(client,
                guac_common_ssh_expose_sftp_filesystem,
                vnc_client->sftp_filesystem);

        /* Abort if SFTP connection fails */
        if (vnc_client->sftp_filesystem == NULL) {
            guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_ERROR,
                    "SFTP connection failed.");
            return NULL;
        }

        /* Configure destination for basic uploads, if specified */
        if (settings->sftp_directory != NULL)
            guac_common_ssh_sftp_set_upload_path(
                    vnc_client->sftp_filesystem,
                    settings->sftp_directory);

        guac_client_log(client, GUAC_LOG_DEBUG,
                "SFTP connection succeeded.");

    }
#endif

    /* Set up screen recording, if requested */
    if (settings->recording_path != NULL) {
        vnc_client->recording = guac_common_recording_create(client,
                settings->recording_path,
                settings->recording_name,
                settings->create_recording_path,
                !settings->recording_exclude_output,
                !settings->recording_exclude_mouse,
                settings->recording_include_keys);
    }

    /* Create display */
    vnc_client->display = guac_common_display_alloc(client,
            guac_vnc_backend_framebuffer_get_width(backend_client),
            guac_vnc_backend_framebuffer_get_height(backend_client));

    /* If not read-only, set an appropriate cursor */
    if (settings->read_only == 0) {
        if (settings->remote_cursor)
            guac_common_cursor_set_dot(vnc_client->display->cursor);
        else
            guac_common_cursor_set_pointer(vnc_client->display->cursor);

    }

    guac_socket_flush(client->socket);

    guac_timestamp last_frame_end = guac_timestamp_current();

    /* Handle messages from VNC server while client is running */
    while (client->state == GUAC_CLIENT_RUNNING) {

        /* Wait for start of frame */
        int wait_result = guac_vnc_backend_framebuffer_wait(backend_client,
                GUAC_VNC_FRAME_START_TIMEOUT);
        if (wait_result > 0) {

            int processing_lag = guac_client_get_processing_lag(client);
            guac_timestamp frame_start = guac_timestamp_current();

            /* Read server messages until frame is built */
            do {

                guac_timestamp frame_end;
                int frame_remaining;

                /* Calculate time remaining in frame */
                frame_end = guac_timestamp_current();
                frame_remaining = frame_start + GUAC_VNC_FRAME_DURATION
                                - frame_end;

                /* Calculate time that client needs to catch up */
                int time_elapsed = frame_end - last_frame_end;
                int required_wait = processing_lag - time_elapsed;

                /* Increase the duration of this frame if client is lagging */
                if (required_wait > GUAC_VNC_FRAME_TIMEOUT)
                    wait_result = guac_vnc_backend_framebuffer_wait(
                            backend_client, required_wait);

                /* Wait again if frame remaining */
                else if (frame_remaining > 0)
                    wait_result = guac_vnc_backend_framebuffer_wait(
                            backend_client, GUAC_VNC_FRAME_TIMEOUT);
                else
                    break;

            } while (wait_result > 0);

            /* Record end of frame, excluding server-side rendering time (we
             * assume server-side rendering time will be consistent between any
             * two subsequent frames, and that this time should thus be
             * excluded from the required wait period of the next frame). */
            last_frame_end = frame_start;

        }

        /* If an error occurs, log it and fail */
        if (wait_result < 0)
            guac_client_abort(client, GUAC_PROTOCOL_STATUS_UPSTREAM_ERROR, "Connection closed.");

        /* Flush frame */
        guac_common_surface_flush(vnc_client->display->default_surface);
        guac_client_end_frame(client);
        guac_socket_flush(client->socket);

    }

    /* Kill client and finish connection */
    guac_client_stop(client);
    guac_client_log(client, GUAC_LOG_INFO, "Internal VNC client disconnected");
    return NULL;

}

