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

#ifndef GUAC_VNC_BACKEND_CLIENT_INTERNAL_H
#define GUAC_VNC_BACKEND_CLIENT_INTERNAL_H

#include "backend/callbacks.h"

#include <guacamole/client.h>
#include <rfb/rfbclient.h>

#include <stdbool.h>

/**
 * Key which can be used with the rfbClientGetClientData function to return
 * the associated guac_vnc_backend_client.
 */
extern char* GUAC_VNC_BACKEND_CLIENT_KEY;

struct guac_vnc_backend_client {

    /**
     * The underlying VNC client instance from libvncclient.
     */
    rfbClient* rfb_client;

    /**
     * The original framebuffer malloc procedure provided by the initialized
     * rfbClient.
     */
    MallocFrameBufferProc rfb_MallocFrameBuffer;

    /**
     * The password to use to authenticate with the VNC server, or NULL to not
     * supply a password.
     */
    char* password;

    /**
     * Whether the red and blue components of each color should be swapped.
     * This is mainly used for VNC servers that do not properly handle
     * colors.
     */
    bool swap_red_blue;

    /**
     * Whether copyrect  was used to produce the latest update received
     * by the VNC server.
     */
    bool copy_rect_used;

    /**
     * The guac_client instance associated with the VNC connection.
     */
    guac_client* client;

    /**
     * The callbacks provided at the time this VNC client backend was allocated.
     */
    guac_vnc_backend_callbacks callbacks;

};

#endif

