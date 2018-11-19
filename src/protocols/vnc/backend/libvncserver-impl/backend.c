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

#include <guacamole/client.h>
#include <rfb/rfbclient.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Static pointer to the guac_client instance that should be used for logging
 * by libvncclient.
 */
guac_client* GUAC_VNC_LOGGING_CLIENT = NULL;

/**
 * Logging handler for libvncclient messages logged at the "info" level.
 */
static void guac_vnc_client_log_info(const char* format, ...) {

    va_list args;

    va_start(args, format);
    vguac_client_log(GUAC_VNC_LOGGING_CLIENT, GUAC_LOG_INFO, format, args);
    va_end(args);

}

/**
 * Logging handler for libvncclient messages logged at the "error" level.
 */
static void guac_vnc_client_log_error(const char* format, ...) {

    va_list args;

    va_start(args, format);
    vguac_client_log(GUAC_VNC_LOGGING_CLIENT, GUAC_LOG_ERROR, format, args);
    va_end(args);

}

void guac_vnc_backend_init(guac_client* client) {

    /* Log that we're using the libvncclient backend */
    guac_client_log(client, GUAC_LOG_INFO, "VNC backend: libVNCServer (libvncclient)");

    /* Assign static pointer to client for sake of logging */
    GUAC_VNC_LOGGING_CLIENT = client;

    /* Override libvncclient's default logging, writing instead to the
     * guac_client log */
    rfbClientLog = guac_vnc_client_log_info;
    rfbClientErr = guac_vnc_client_log_error;

}

void guac_vnc_backend_shutdown() {
    /* libvncclient does not require static shutdown */
}

