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

#ifndef GUAC_VNC_BACKEND_FRAMEBUFFER_H
#define GUAC_VNC_BACKEND_FRAMEBUFFER_H

#include "backend/client.h"

/**
 * Waits for framebuffer updates from the VNC server. Callbacks for handling
 * framebuffer updates will be automatically invoked if updates are available.
 * This function will block until at least one framebuffer-related callback has
 * been invoked, the client has disconnected, or until the timeout elapses. If
 * the timeout elapses before at least one callback is invoked, zero is
 * returned.
 *
 * Note that framebuffer callbacks may be invoked at any time, even while this
 * function is not running. If a callback is invoked while this function is not
 * running, the next call to this function will not block.
 *
 * @param backend_client
 *     The VNC client to wait for.
 *
 * @param timeout
 *     The maximum amount of time to wait, in milliseconds.
 *
 * @returns
 *     A positive value if the framebuffer has changed, zero if the timeout
 *     elapses before a change occurred, or a negative value if the connection
 *     has been closed.
 */
int guac_vnc_backend_framebuffer_wait(guac_vnc_backend_client* backend_client,
        int timeout);

/**
 * Returns the current width of the framebuffer in pixels. If the width of the
 * framebuffer is not yet known, this may be 0.
 *
 * @param backend_client
 *     The VNC client to retrieve the width of.
 *
 * @returns
 *     The width of the framebuffer of the given VNC client, in pixels.
 */
int guac_vnc_backend_framebuffer_get_width(guac_vnc_backend_client* backend_client);

/**
 * Returns the current height of the framebuffer in pixels. If the height of the
 * framebuffer is not yet known, this may be 0.
 *
 * @param backend_client
 *     The VNC client to retrieve the height of.
 *
 * @returns
 *     The height of the framebuffer of the given VNC client, in pixels.
 */
int guac_vnc_backend_framebuffer_get_height(guac_vnc_backend_client* backend_client);

#endif

