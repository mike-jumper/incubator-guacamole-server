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


#ifndef GUAC_VNC_BACKEND_SETTINGS_H
#define GUAC_VNC_BACKEND_SETTINGS_H

#include <stdbool.h>

/**
 * Settings which should apply to the VNC connection established by the backend
 * implementation. Not all settings will be supported by all backends. It is up
 * to the backend to properly log warnings if setting values will be ignored
 * due to lack of support.
 */
typedef struct guac_vnc_backend_settings {

    /**
     * The hostname of the VNC server (or repeater) to connect to.
     */
    char* hostname;

    /**
     * The port of the VNC server (or repeater) to connect to.
     */
    int port;

    /**
     * The password to use to authenticate with the VNC server, or NULL to not
     * supply a password.
     */
    char* password;

    /**
     * Space-separated list of encodings to use within the VNC session.
     */
    char* encodings;

    /**
     * The color depth to request, in bits.
     */
    int color_depth;

    /**
     * Whether this connection is read-only, and user input should be dropped.
     */
    bool read_only;

    /**
     * The VNC host to connect to, if using a repeater. If not using a
     * repeated, this value should be NULL.
     */
    char* dest_host;

    /**
     * The VNC port to connect to, if using a repeater. If dest_host is NULL,
     * this value will be ignored. If dest_host is not NULL, this value is
     * required.
     */
    int dest_port;

    /**
     * Whether not actually connecting to a VNC server, but rather listening
     * for a connection from the VNC server (reverse connection).
     */
    bool reverse_connect;

    /**
     * The maximum amount of time to wait when listening for connections, in
     * milliseconds. If revese_connect is not set to true, this value will be
     * ignored.
     */
    int listen_timeout;

    /**
     * Whether the cursor should be rendered on the server (remote) or on the
     * client (local).
     */
    bool remote_cursor;
   
} guac_vnc_backend_settings;

#endif

