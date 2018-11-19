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

#include "addon.h"
#include "auth-internal.h"
#include "backend/callbacks.h"
#include "backend/client.h"
#include "backend/settings.h"
#include "client-internal.h"
#include "clipboard-internal.h"
#include "event.h"
#include "framebuffer-internal.h"
#include "log.h"
#include "main.h"

#include <guacamole/client.h>
#include <vnc/DataStore.h>
#include <vnc/DirectTcp.h>
#include <vnc/EventLoopFd.h>
#include <vnc/Init.h>
#include <vnc/Logger.h>
#include <vnc/Viewer.h>

#include <stdbool.h>
#include <stdlib.h>
#include <sys/select.h>

/**
 * RealVNC SDK mapping for event file descriptor callbacks.
 */
vnc_EventLoopFd_Callback guac_realvnc_event_loop_fd_callback = {
    .eventUpdated = guac_realvnc_event_updated
};

/**
 * RealVNC SDK mapping for framebuffer update callbacks.
 */
vnc_Viewer_FramebufferCallback guac_realvnc_framebuffer_callback = {
    .serverFbSizeChanged = guac_realvnc_framebuffer_resized,
    .viewerFbUpdated = guac_realvnc_framebuffer_updated
};

/**
 * RealVNC SDK mapping for authentication callbacks.
 */
vnc_Viewer_AuthenticationCallback guac_realvnc_auth_callback = {
    .requestUserCredentials = guac_realvnc_request_credentials
};

/**
 * RealVNC SDK mapping for server event callbacks. The only relevant server
 * event is receipt of clipboard data.
 */
vnc_Viewer_ServerEventCallback guac_realvnc_server_event_callback = {
    .serverClipboardTextChanged = guac_realvnc_clipboard_changed
};

/**
 * RealVNC SDK mapping for connection state change callbacks.
 */
vnc_Viewer_ConnectionCallback guac_realvnc_connection_callback = {
    .connected = guac_realvnc_connected,
    .disconnected = guac_realvnc_disconnected
};

/**
 * RealVNC SDK mapping for logging callbacks.
 */
vnc_Logger_Callback logger_callback = {
    .logMessage = guac_realvnc_log_message
};

void* guac_realvnc_event_loop(void* data) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;
    guac_vnc_backend_settings* settings = &backend_client->settings;
    guac_client* client = backend_client->client;

    vnc_init();

    /* Write to pre-defined file for RealVNC datastore */
    vnc_DataStore_createFileStore(GUAC_REALVNC_DATASTORE_PATH);

    /* Enable all add-ons defined in /etc/guacamole/realvnc/ */
    guac_realvnc_enable_addons(client);

    /* Track event file descriptors as they are created/destroyed */
    vnc_EventLoopFd_setCallback(&guac_realvnc_event_loop_fd_callback, backend_client);

    /* Log using guac_client_log() */
    vnc_Logger_createCustomLogger(&logger_callback, client);

    /* Allocate and initialize RealVNC SDK client */
    vnc_Viewer* viewer = vnc_Viewer_create();
    backend_client->viewer = viewer;

    /* Allocate space for the initial framebuffer */
    vnc_Viewer_setViewerFb(viewer, NULL, 0, vnc_PixelFormat_rgb888(),
            backend_client->width, backend_client->height, 0);

    /* Set all VNC client callbacks */
    vnc_Viewer_setFramebufferCallback(viewer, &guac_realvnc_framebuffer_callback, backend_client);
    vnc_Viewer_setAuthenticationCallback(viewer, &guac_realvnc_auth_callback, backend_client);
    vnc_Viewer_setServerEventCallback(viewer, &guac_realvnc_server_event_callback, backend_client);
    vnc_Viewer_setConnectionCallback(viewer, &guac_realvnc_connection_callback, backend_client);

    /* Obtain TCP connector (may not be enabled) */
    vnc_DirectTcpConnector* tcp_connector = vnc_DirectTcpConnector_create();
    if (tcp_connector == NULL) {
        guac_client_log(client, GUAC_LOG_ERROR, "Cannot connect via TCP: "
                "The installed RealVNC SDK does not have the Direct TCP "
                "add-on enabled.");
        vnc_Viewer_destroy(viewer);
        free(backend_client);
        return NULL;
    }

    /* Attempt to connect */
    if (!vnc_DirectTcpConnector_connect(tcp_connector, settings->hostname,
            settings->port, vnc_Viewer_getConnectionHandler(viewer))) {
        vnc_DirectTcpConnector_destroy(tcp_connector);
        vnc_Viewer_destroy(viewer);
        free(backend_client);
        return NULL;
    }

    /* Connector is not needed any longer */
    vnc_DirectTcpConnector_destroy(tcp_connector);

    /* Continue handling events until the Guacamole client or VNC client
     * disconnect */
    int next_timeout = 0;
    while (client->state == GUAC_CLIENT_RUNNING
            && vnc_Viewer_getConnectionStatus(viewer) != vnc_Viewer_Disconnected) {

        fd_set read_fds = backend_client->read_fds;
        fd_set write_fds = backend_client->write_fds;
        fd_set except_fds = backend_client->except_fds;

        /* Convert millisecond timeout value to timeval struct */
        struct timeval timeout = {
            .tv_sec = next_timeout / 1000,
            .tv_usec = (next_timeout % 1000) * 1000
        };

        /* Wait for the internal event pipe file descriptor or any of the
         * RealVNC SDK file descriptors to be ready */
        int result = select(backend_client->max_fd + 1, &read_fds, &write_fds,
                &except_fds, &timeout);

        /* Process data on all ready file descriptors */
        if (result > 0) {
            for (int fd = 0; fd <= backend_client->max_fd; fd++) {

                /* Handle any events received along the internal event pipe */
                if (fd == backend_client->event_read_fd
                        && FD_ISSET(fd, &read_fds)) {

                    guac_realvnc_event event;

                    /* Read and process single event from internal pipe */
                    if (guac_realvnc_event_read(fd, &event))
                        guac_realvnc_event_process(backend_client, &event);

                    /* This file descriptor is not from the RealVNC SDK and
                     * should not be marked */
                    continue;

                }

                /* For all other file descriptors, apply the relevant mark for
                 * later processing by vnc_EventLoopFd_handleEvents() */

                if (FD_ISSET(fd, &read_fds))
                    vnc_EventLoopFd_markEvents(fd, vnc_EventLoopFd_Read);

                if (FD_ISSET(fd, &write_fds))
                    vnc_EventLoopFd_markEvents(fd, vnc_EventLoopFd_Write);

                if (FD_ISSET(fd, &except_fds))
                    vnc_EventLoopFd_markEvents(fd, vnc_EventLoopFd_Except);

            }
        }

        /* Abort if an error prevents further I/O */
        else if (result < 0)
            break;

        /* Request that all marked events be handled */
        next_timeout = vnc_EventLoopFd_handleEvents();

    }

    /* Disconnect (if connected) */
    vnc_Viewer_disconnect(backend_client->viewer);

    /* Free associated resources */
    vnc_Viewer_destroy(viewer);

    /* Shutdown RealVNC SDK */
    vnc_shutdown();

    return NULL;

}

