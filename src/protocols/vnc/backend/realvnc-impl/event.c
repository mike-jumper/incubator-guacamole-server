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

#include <guacamole/client.h>
#include <vnc/EventLoopFd.h>
#include <vnc/Viewer.h>

#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

int guac_realvnc_event_read(int fd, guac_realvnc_event* event) {

    char* buffer = (char*) event;
    int remaining = sizeof(guac_realvnc_event);

    do {

        /* Read as much event data as possible */
        int result = read(fd, buffer, remaining);
        if (result <= 0)
            return 0;

        /* Continue reading until event struct is filled */
        buffer += result;
        remaining -= result;

    } while (remaining > 0);

    /* Read successfully */
    return 1;

}

int guac_realvnc_event_write(int fd, guac_realvnc_event* event) {

    char* buffer = (char*) event;
    int remaining = sizeof(guac_realvnc_event);

    do {

        /* Write as much event data as possible */
        int result = write(fd, buffer, remaining);
        if (result <= 0)
            return 0;

        /* Continue writing until entire struct is written */
        buffer += result;
        remaining -= result;

    } while (remaining > 0);

    /* Written successfully */
    return 1;

}

void guac_realvnc_event_updated(void* data, int fd, int mask) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;

    /* Update maximum file descriptor for sake of future calls to select() */
    if (fd > backend_client->max_fd)
        backend_client->max_fd = fd;

    /* Add/remove provided file descriptor from any applicable set */

    if (mask & vnc_EventLoopFd_Read)
        FD_SET(fd, &backend_client->read_fds);
    else
        FD_CLR(fd, &backend_client->read_fds);

    if (mask & vnc_EventLoopFd_Write)
        FD_SET(fd, &backend_client->write_fds);
    else
        FD_CLR(fd, &backend_client->write_fds);

    if (mask & vnc_EventLoopFd_Except)
        FD_SET(fd, &backend_client->except_fds);
    else
        FD_CLR(fd, &backend_client->except_fds);

}

void guac_realvnc_event_process(guac_vnc_backend_client* backend_client,
        guac_realvnc_event* event) {

    guac_realvnc_event_pointer* pointer;
    guac_realvnc_event_keyboard* keyboard;
    guac_realvnc_event_clipboard* clipboard;

    switch (event->type) {

        /* Send updated pointer device state as pointer event */
        case GUAC_REALVNC_EVENT_CLIENT_POINTER:
            pointer = &event->details.pointer;
            vnc_Viewer_sendPointerEvent(backend_client->viewer,
                    pointer->x, pointer->y, pointer->mask, vnc_false);
            break;

        /* Send updated keyboard state as key event */
        case GUAC_REALVNC_EVENT_CLIENT_KEYBOARD:
            keyboard = &event->details.keyboard;
            if (keyboard->pressed)
                vnc_Viewer_sendKeyDown(backend_client->viewer, keyboard->keysym,
                        keyboard->keysym /* Arbitrary "key code" which must match future
                                            corresponding vnc_Viewer_sendKeyUp() call */);
            else
                vnc_Viewer_sendKeyUp(backend_client->viewer,
                        keyboard->keysym /* Key code from vnc_Viewer_sendKeyDown() call */);
            break;

        /* Send received clipboard text */
        case GUAC_REALVNC_EVENT_CLIENT_CLIPBOARD:
            clipboard = &event->details.clipboard;
            vnc_Viewer_sendClipboardText(backend_client->viewer, clipboard->text);
            free(clipboard->text);
            break;

        /* Disconnect upon request */
        case GUAC_REALVNC_EVENT_CLIENT_DISCONNECT:
            guac_client_stop(backend_client->client);
            break;

    }

}

