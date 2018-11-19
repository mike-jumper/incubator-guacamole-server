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

#ifndef GUAC_VNC_BACKEND_CALLBACKS_H
#define GUAC_VNC_BACKEND_CALLBACKS_H

/**
 * Callback which is invoked whenever clipboard data is received from the VNC
 * server. VNC provides only a text clipboard, and all text is encoded in ISO
 * 8859-1.
 *
 * @param text
 *     The clipboard text received, encoded in ISO 8859-1.
 *
 * @param length
 *     The length of the received text, in bytes.
 *
 * @param data
 *     The pointer to arbitrary data from the guac_vnc_backend_callbacks
 *     structure provided when the guac_vnc_backend_client was allocated.
 */
typedef void guac_vnc_backend_clipboard_received(const char* text, int length,
        void* data);

/**
 * Callback which is invoked whenever the remote mouse cursor has changed.
 *
 * @param x
 *     The X coordinate of the hotspot within the mouse cursor image.
 *
 * @param y
 *     The Y coordinate of the hotspot within the mouse cursor image.
 *
 * @param image
 *     A pointer to raw 32-bit ARGB image data, where the highest-order byte is
 *     the alpha component and the remaining 24 bits are the red, green, and
 *     blue components (in that order).
 *
 * @param width
 *     The width of the given image data, in pixels.
 *
 * @param height
 *     The height of the given image data, in pixels.
 *
 * @param stride
 *     The number of bytes in each row of image data.
 *
 * @param data
 *     The pointer to arbitrary data from the guac_vnc_backend_callbacks
 *     structure provided when the guac_vnc_backend_client was allocated.
 */
typedef void guac_vnc_backend_cursor_updated(int x, int y,
        const unsigned char* image, int width, int height, int stride,
        void* data);

/**
 * Callback which is invoked whenever the VNC framebuffer is changing size.
 *
 * @param width
 *     The new width of the framebuffer, in pixels.
 *
 * @param height
 *     The new height of the framebuffer, in pixels.
 *
 * @param data
 *     The pointer to arbitrary data from the guac_vnc_backend_callbacks
 *     structure provided when the guac_vnc_backend_client was allocated.
 */
typedef void guac_vnc_backend_framebuffer_resized(int width, int height,
        void* data);

/**
 * Callback which is invoked whenever a region of the VNC framebuffer has
 * been copied from one location to another (CopyRect).
 *
 * @param sx
 *     The X coordinate of the upper-left corner of the rectangular region
 *     that was copied.
 *
 * @param sy
 *     The Y coordinate of the upper-left corner of the rectangular region
 *     that was copied.
 *
 * @param width
 *     The width of the rectangular region that was copied, in pixels.
 *
 * @param height
 *     The height of the rectangular region that was copied, in pixels.
 *
 * @param dx
 *     The X coordinate of the upper-left corner of the rectangular region
 *     that received the copied data.
 *
 * @param dy
 *     The Y coordinate of the upper-left corner of the rectangular region
 *     that received the copied data.
 *
 * @param data
 *     The pointer to arbitrary data from the guac_vnc_backend_callbacks
 *     structure provided when the guac_vnc_backend_client was allocated.
 */
typedef void guac_vnc_backend_framebuffer_copied(int sx, int sy,
        int width, int height, int dx, int dy, void* data);

/**
 * Callback which is invoked whenever a region of the VNC framebuffer has
 * changed. This callback WILL NOT be invoked in when a region changes due
 * to a CopyRect update. For regions changing due to data being copied,
 * see guac_vnc_backend_framebuffer_copied.
 *
 * @param x
 *     The X coordinate of the upper-left corner of the rectangular region
 *     that changed.
 *
 * @param y
 *     The Y coordinate of the upper-left corner of the rectangular region
 *     that changed.
 *
 * @param image
 *     A pointer to raw 32-bit RGB image data, where highest-order byte is
 *     ignored and the remaining 24 bits are red, green, and blue components
 *     (in that order).
 *
 * @param width
 *     The width of the given image data, in pixels.
 *
 * @param height
 *     The height of the given image data, in pixels.
 *
 * @param stride
 *     The number of bytes in each row of image data.
 *
 * @param data
 *     The pointer to arbitrary data from the guac_vnc_backend_callbacks
 *     structure provided when the guac_vnc_backend_client was allocated.
 */
typedef void guac_vnc_backend_framebuffer_updated(int x, int y,
        const unsigned char* image, int width, int height, int stride,
        void* data);

/**
 * Various callbacks which should be invoked by the VNC client in response to
 * received data or events. An implementation of each function MUST be
 * provided.
 */
typedef struct guac_vnc_backend_callbacks {

    /**
     * The callback to invoke when clipboard data is received.
     */
    guac_vnc_backend_clipboard_received* clipboard_received;

    /**
     * The callback to invoke when the mouse cursor has changed.
     */
    guac_vnc_backend_cursor_updated* cursor_updated;

    /**
     * The callback to invoke when the VNC framebuffer has changed size.
     */
    guac_vnc_backend_framebuffer_resized* framebuffer_resized;

    /**
     * The callback to invoke when data has been copied from one region of the
     * VNC framebuffer to another (CopyRect).
     */
    guac_vnc_backend_framebuffer_copied* framebuffer_copied;

    /**
     * The callback to invoke when data within a region of the VNC framebuffer
     * has changed. This callback will not be invoked for regions which change
     * due to data being copied through a CopyRect update.
     */
    guac_vnc_backend_framebuffer_updated* framebuffer_updated;

    /**
     * Arbitrary data which should be passed to each callback.
     */
    void* data;

} guac_vnc_backend_callbacks;

#endif

