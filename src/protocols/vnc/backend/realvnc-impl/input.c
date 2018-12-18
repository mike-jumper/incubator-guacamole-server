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

#include "backend/client.h"
#include "client-internal.h"
#include "event.h"

#include <stdbool.h>

void guac_vnc_backend_send_key(guac_vnc_backend_client* backend_client,
        int keysym, bool pressed) {

    guac_realvnc_event event = {
        .type = GUAC_REALVNC_EVENT_CLIENT_KEYBOARD,
        .details.keyboard = {
            .keysym = keysym,
            .pressed = pressed
        }
    };

    /* Forward keyboard event via main event loop */
    guac_realvnc_event_write(backend_client->event_write_fd, &event);

}

void guac_vnc_backend_send_pointer(guac_vnc_backend_client* backend_client,
        int x, int y, int mask) {

    guac_realvnc_event event;

    /* Determine which buttons have just been pressed */
    int clicked = mask & ~backend_client->button_mask;

    /* Translate clicks of the scroll wheel into scroll events */
    if (clicked & GUAC_CLIENT_MOUSE_SCROLL_UP) {
        event.type = GUAC_REALVNC_EVENT_CLIENT_SCROLL;
        event.details.scroll.delta = -1;
    }
    else if (clicked & GUAC_CLIENT_MOUSE_SCROLL_DOWN) {
        event.type = GUAC_REALVNC_EVENT_CLIENT_SCROLL;
        event.details.scroll.delta = 1;
    }

    /* Translate all other mouse events into pointer events */
    else {
        event.type = GUAC_REALVNC_EVENT_CLIENT_POINTER;
        event.details.pointer.x = x;
        event.details.pointer.y = y;
        event.details.pointer.mask = mask & 0x7; /* Exclude scroll wheel */
    }

    /* Forward pointer/scroll event via main event loop */
    guac_realvnc_event_write(backend_client->event_write_fd, &event);

    /* Update current button state */
    backend_client->button_mask = mask;

}

