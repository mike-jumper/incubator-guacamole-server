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

#ifndef GUAC_VNC_BACKEND_INPUT_H
#define GUAC_VNC_BACKEND_INPUT_H

#include "backend/client.h"

#include <stdbool.h>

/**
 * Sends a key event to the VNC server, updating the state of the remote
 * keyboard.
 *
 * @param backend_client
 *     The VNC client to use to send the keyboard event.
 *
 * @param keysym
 *     The keysym of the key that was pressed or released.
 *
 * @param pressed
 *     true if the key was pressed, false if the key was released.
 */
void guac_vnc_backend_send_key(guac_vnc_backend_client* backend_client,
        int keysym, bool pressed);

/**
 * Sends a pointer (mouse) event to the VNC server, updating the state of the
 * remote pointer device.
 *
 * @param backend_client
 *     The VNC client to use to send the pointer event.
 *
 * @param x
 *     The new X coordinate of the pointer device, in pixels, which may be
 *     unchanged from the last pointer event.
 *
 * @param y
 *     The new Y coordinate of the pointer device, in pixels, which may be
 *     unchanged from the last pointer event.
 *
 * @param mask
 *     An integer value representing the current state of each button, where
 *     the Nth bit within the integer is set to 1 if and only if the Nth button
 *     on the pointer device is currently pressed. The lowest-order bit is the
 *     leftmost button, followed by the middle button, right button, and
 *     finally the up and down buttons of the scroll wheel. These mask values
 *     exactly correspond to the mask values used by the Guacamole protocol.
 *
 *     @see GUAC_CLIENT_MOUSE_LEFT
 *     @see GUAC_CLIENT_MOUSE_MIDDLE
 *     @see GUAC_CLIENT_MOUSE_RIGHT
 *     @see GUAC_CLIENT_MOUSE_SCROLL_UP
 *     @see GUAC_CLIENT_MOUSE_SCROLL_DOWN
 */
void guac_vnc_backend_send_pointer(guac_vnc_backend_client* backend_client,
        int x, int y, int mask);

#endif

