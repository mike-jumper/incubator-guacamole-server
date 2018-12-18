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

#ifndef GUAC_VNC_BACKEND_EVENT_H
#define GUAC_VNC_BACKEND_EVENT_H

#include "backend/client.h"

#include <vnc/Viewer.h>

#include <stdbool.h>

/**
 * All possible types of events which may be represented by a
 * guac_realvnc_event struct.
 */
typedef enum guac_realvnc_event_type {

    /**
     * Keyboard event (press/release).
     */
    GUAC_REALVNC_EVENT_CLIENT_KEYBOARD,

    /**
     * Pointer event (mouse movement or click).
     */
    GUAC_REALVNC_EVENT_CLIENT_POINTER,

    /**
     * Scroll event (mouse scroll wheel).
     */
    GUAC_REALVNC_EVENT_CLIENT_SCROLL,

    /**
     * Clipboard event.
     */
    GUAC_REALVNC_EVENT_CLIENT_CLIPBOARD,

    /**
     * Disconnection event.
     */
    GUAC_REALVNC_EVENT_CLIENT_DISCONNECT

} guac_realvnc_event_type;

/**
 * Event details specific to keyboard events.
 */
typedef struct guac_realvnc_event_keyboard {

    /**
     * The keysym of the key which was pressed or released.
     */
    int keysym;

    /**
     * Whether the key was pressed (true) or released (false).
     */
    bool pressed;

} guac_realvnc_event_keyboard;

/**
 * Event details specific to pointer (mouse) events.
 */
typedef struct guac_realvnc_event_pointer {

    /**
     * The X-coordinate of the mouse pointer, in pixels.
     */
    int x;

    /**
     * The Y-coordinate of the mouse pointer, in pixels.
     */
    int y;

    /**
     * An integer value representing the current state of each button, where
     * the Nth bit within the integer is set to 1 if and only if the Nth button
     * on the pointer device is currently pressed. The lowest-order bit is the
     * leftmost button, followed by the middle button and right button. The
     * scroll wheel is NOT included here. These mask values exactly correspond
     * to a subset of the mask values used by the Guacamole protocol.
     *
     * @see GUAC_CLIENT_MOUSE_LEFT
     * @see GUAC_CLIENT_MOUSE_MIDDLE
     * @see GUAC_CLIENT_MOUSE_RIGHT
     */
    int mask;

} guac_realvnc_event_pointer;

/**
 * Event details specific to mouse scroll wheel events.
 */
typedef struct guac_realvnc_event_scroll {

    /**
     * The number of "ticks" that the mouse has scrolled. This should be 1 for
     * a downward scroll and -1 for an upward scroll.
     */
    int delta;

} guac_realvnc_event_scroll;


/**
 * Event details specific to clipboard events.
 */
typedef struct guac_realvnc_event_clipboard{

    /**
     * The new UTF-8 contents of the clipboard. This value must be manually
     * freed using free() after the event has been handled.
     */
    char* text;

} guac_realvnc_event_clipboard;


/**
 * An arbitrary VNC-related event which must be eventually processed by the
 * main event loop and forwarded to the single-threaded RealVNC SDK.
 */
typedef struct guac_realvnc_event {

    /**
     * The type of this event. The value of this property dictates which
     * details are applicable.
     */
    guac_realvnc_event_type type;

    /**
     * Type-specific event details.
     */
    union {

        /**
         * Details which apply to this event if the type is
         * GUAC_REALVNC_EVENT_CLIENT_KEYBOARD.
         */
        guac_realvnc_event_keyboard keyboard;

        /**
         * Details which apply to this event if the type is
         * GUAC_REALVNC_EVENT_CLIENT_POINTER.
         */
        guac_realvnc_event_pointer pointer;

        /**
         * Details which apply to this event if the type is
         * GUAC_REALVNC_EVENT_CLIENT_SCROLL.
         */
        guac_realvnc_event_scroll scroll;

        /**
         * Details which apply to this event if the type is
         * GUAC_REALVNC_EVENT_CLIENT_CLIPBOARD.
         */
        guac_realvnc_event_clipboard clipboard;

    } details;

} guac_realvnc_event;

/**
 * Reads a single event from the given file descriptor. In practice, this file
 * descriptor will always be the event_read_fd of the guac_vnc_backend_client.
 *
 * @param fd
 *     The file descriptor to read the event from.
 *
 * @param event
 *     The event structure to populate with data from the read event.
 *
 * @return
 *     Non-zero if an event was successfully read from the file descriptor,
 *     zero otherwise.
 */
int guac_realvnc_event_read(int fd, guac_realvnc_event* event);

/**
 * Writes a single event to the given file descriptor. In practice, this file
 * descriptor will always be the event_write_fd of the guac_vnc_backend_client.
 *
 * @param fd
 *     The file descriptor to write the event to.
 *
 * @param event
 *     The event structure containing data describing the event to be written.
 *
 * @return
 *     Non-zero if the event was successfully written to the file descriptor,
 *     zero otherwise.
 */
int guac_realvnc_event_write(int fd, guac_realvnc_event* event);

/**
 * Callback invoked by the RealVNC SDK when a file descriptor related to an
 * event should be monitored or cease being monitored for readiness (as with
 * select()).
 *
 * @param data
 *     The guac_vnc_backend_client associated with the VNC connection. Note
 *     that the VNC connection may not yet be active at the time this callback
 *     is invoked.
 *
 * @param fd
 *     The file descriptor which should be monitored / cease being monitored.
 *
 * @param mask
 *     A bitwise OR of vnc_EventLoopFd_Read, vnc_EventLoopFd_Write, or
 *     vnc_EventLoopFd_Except, whichever is applicable to the file descriptor
 *     provided. If the file descriptor is no longer being used or should cease
 *     being monitored, this will be zero.
 */
void guac_realvnc_event_updated(void* data, int fd, int mask);

/**
 * Processes the given event, invoking any necessary RealVNC SDK functions.
 * This function must be called ONLY from within the main RealVNC SDK thread.
 *
 * @param backend_client
 *     The guac_vnc_backend_client associated with the VNC connection.
 *
 * @param event
 *     The event that should be processed.
 */
void guac_realvnc_event_process(guac_vnc_backend_client* backend_client,
        guac_realvnc_event* event);

#endif

