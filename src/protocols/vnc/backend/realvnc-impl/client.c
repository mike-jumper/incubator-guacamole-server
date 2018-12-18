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

#include "backend/callbacks.h"
#include "backend/client.h"
#include "backend/settings.h"
#include "client-internal.h"
#include "framebuffer-internal.h"
#include "event.h"
#include "main.h"

#include <guacamole/client.h>
#include <vnc/Viewer.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

guac_vnc_backend_client* guac_vnc_backend_client_create(guac_client* client,
        guac_vnc_backend_settings* settings,
        guac_vnc_backend_callbacks* callbacks) {

    /* Create pipe for sending events to the main event loop */
    int event_pipe[2];
    if (pipe(event_pipe) == -1)
        return NULL;

    /* Repeaters not currently supported */
    if (settings->dest_host != NULL) {
        guac_client_log(client, GUAC_LOG_ERROR, "VNC repeaters are not "
                "currently supported by the RealVNC SDK backend.");
        return NULL;
    }

    /* Reverse connections not currently supported */
    if (settings->reverse_connect) {
        guac_client_log(client, GUAC_LOG_ERROR, "Reverse VNC connections "
                "are not currently supported by the RealVNC SDK backend.");
        return NULL;
    }

    /* The cursor is always remote if using the RealVNC SDK to proxy VNC */
    if (!settings->remote_cursor) {
        guac_client_log(client, GUAC_LOG_WARNING, "The RealVNC SDK does not "
                "provide support for proxying cursor updates. Cursor "
                "rendering will effectively always be remote.");
    }

    /* Specifying the encodings is not supported */
    if (strcmp(settings->encodings, "zrle ultra copyrect hextile "
                "zlib corre rre raw") != 0) {
        guac_client_log(client, GUAC_LOG_WARNING, "The RealVNC SDK does not "
                "support directly specifying the VNC encodings used. "
                "Explicitly specified encodings will be ignored.");
    }

    /* Specifying the color depth is not supported */
    if (settings->color_depth != 0) {
        guac_client_log(client, GUAC_LOG_WARNING, "The RealVNC SDK does not "
                "support directly specifying the color depth. Explicitly "
                "specified color depth will be ignored.");
    }

    /* Allocate and initialize backend structure */
    guac_vnc_backend_client* backend_client = malloc(sizeof(guac_vnc_backend_client));
    backend_client->client = client;
    backend_client->settings = *settings;
    backend_client->callbacks = *callbacks;
    backend_client->event_read_fd = event_pipe[0];
    backend_client->event_write_fd = event_pipe[1];
    backend_client->width = GUAC_REALVNC_INITIAL_WIDTH;
    backend_client->height = GUAC_REALVNC_INITIAL_HEIGHT;

    FD_ZERO(&backend_client->read_fds);
    FD_ZERO(&backend_client->write_fds);
    FD_ZERO(&backend_client->except_fds);

    /* The only file descriptor available for reading initially is the read end
     * of the event pipe */
    backend_client->max_fd = backend_client->event_read_fd;
    FD_SET(backend_client->event_read_fd, &backend_client->read_fds);

    /* No updates yet received */
    backend_client->update_received = false;
    pthread_cond_init(&(backend_client->update_received_cond), NULL);
    pthread_mutex_init(&(backend_client->update_received_lock), NULL);

    pthread_mutex_t* state_lock = &(backend_client->state_lock);
    pthread_cond_t* state_cond = &(backend_client->state_cond);

    /* Client is currently connecting */
    backend_client->state = GUAC_REALVNC_CONNECTING;
    pthread_cond_init(state_cond, NULL);
    pthread_mutex_init(state_lock, NULL);

    /* Start event loop and connection process */
    pthread_create(&backend_client->realvnc_thread, NULL,
            guac_realvnc_event_loop, backend_client);

    /* Wait for connection process to complete */
    pthread_mutex_lock(state_lock);
    if (backend_client->state == GUAC_REALVNC_CONNECTING) {
        pthread_cond_wait(state_cond, state_lock);
    }
    pthread_mutex_unlock(state_lock);

    /* Fail if client could not connect */
    if (backend_client->state == GUAC_REALVNC_DISCONNECTED)
        return NULL;

    /* Client connected successfully */
    return backend_client;

}

void guac_vnc_backend_client_free(guac_client* client,
        guac_vnc_backend_client* backend_client) {

    guac_realvnc_event event = {
        .type = GUAC_REALVNC_EVENT_CLIENT_DISCONNECT
    };

    /* Request disconnect */
    guac_realvnc_event_write(backend_client->event_write_fd, &event);

    /* Wait for main thread to terminate */
    pthread_join(backend_client->realvnc_thread, NULL);

    /* Close event pipe */
    close(backend_client->event_read_fd);
    close(backend_client->event_write_fd);

    free(backend_client);

}

void guac_realvnc_connected(void* data, vnc_Viewer* viewer) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;

    pthread_mutex_t* state_lock = &(backend_client->state_lock);
    pthread_cond_t* state_cond = &(backend_client->state_cond);

    /* Signal that connection is established */
    pthread_mutex_lock(state_lock);
    backend_client->state = GUAC_REALVNC_CONNECTED;
    pthread_cond_signal(state_cond);
    pthread_mutex_unlock(state_lock);

}

void guac_realvnc_disconnected(void* data, vnc_Viewer* viewer,
        const char* reason, int flags) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;
    guac_client* client = backend_client->client;

    pthread_mutex_t* state_lock = &(backend_client->state_lock);
    pthread_cond_t* state_cond = &(backend_client->state_cond);

    /* Signal that connection is closed */
    pthread_mutex_lock(state_lock);
    backend_client->state = GUAC_REALVNC_DISCONNECTED;
    pthread_cond_signal(state_cond);
    pthread_mutex_unlock(state_lock);

    /* Log disconnect reason */
    guac_client_log(client, GUAC_LOG_INFO, "RealVNC SDK "
            "disconnected: %s", reason);

}

