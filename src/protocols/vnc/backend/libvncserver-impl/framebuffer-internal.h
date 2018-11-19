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

#ifndef GUAC_VNC_BACKEND_FRAMEBUFFER_INTERNAL_H
#define GUAC_VNC_BACKEND_FRAMEBUFFER_INTERNAL_H

#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>

/**
 * Callback invoked by libVNCServer when it receives a new binary image data.
 * the VNC server. The image itself will be stored in the designated sub-
 * rectangle of rfb_client->framebuffer.
 *
 * @param rfb_client
 *     The VNC client associated with the VNC session in which the new image
 *     was received.
 *
 * @param x
 *     The X coordinate of the upper-left corner of the destination rectangle
 *     in which the image should be drawn, in pixels.
 *
 * @param y
 *     The Y coordinate of the upper-left corner of the destination rectangle
 *     in which the image should be drawn, in pixels.
 *
 * @param width
 *     The width of the image, in pixels.
 *
 * @param height
 *     The height of the image, in pixels.
 */
void guac_libvncclient_framebuffer_update(rfbClient* rfb_client, int x, int y,
        int width, int height);

/**
 * Callback invoked by libVNCServer when it receives a CopyRect message.
 * CopyRect specified a rectangle of source data within the display and a
 * set of X/Y coordinates to which that rectangle should be copied.
 *
 * @param rfb_client
 *     The VNC client associated with the VNC session in which the CopyRect
 *     was received.
 *
 * @param sx
 *     The X coordinate of the upper-left corner of the source rectangle
 *     from which the image data should be copied, in pixels.
 *
 * @param sy
 *     The Y coordinate of the upper-left corner of the source rectangle
 *     from which the image data should be copied, in pixels.
 *
 * @param width
 *     The width of the source and destination rectangles, in pixels.
 *
 * @param height
 *     The height of the source and destination rectangles, in pixels.
 *
 * @param dx
 *     The X coordinate of the upper-left corner of the destination rectangle
 *     in which the copied image data should be drawn, in pixels.
 *
 * @param dy
 *     The Y coordinate of the upper-left corner of the destination rectangle
 *     in which the copied image data should be drawn, in pixels.
 */
void guac_libvncclient_copyrect(rfbClient* rfb_client, int sx, int sy,
        int width, int height, int dx, int dy);

/**
 * Overridden implementation of the rfb_MallocFrameBuffer function invoked by
 * libVNCServer when the display is being resized (or initially allocated).
 *
 * @param rfb_client
 *     The VNC client associated with the VNC session whose display needs to be
 *     allocated or reallocated.
 *
 * @return
 *     The original value returned by rfb_MallocFrameBuffer().
 */
rfbBool guac_libvncclient_malloc_framebuffer(rfbClient* rfb_client);

#endif

