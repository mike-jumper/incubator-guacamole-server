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

#include <vnc/Viewer.h>

/**
 * The width of the initial framebuffer, in pixels. This framebuffer size will
 * be updated almost immediately following successful connection.
 */
#define GUAC_REALVNC_INITIAL_WIDTH 1024

/**
 * The height of the initial framebuffer, in pixels. This framebuffer size will
 * be updated almost immediately following successful connection.
 */
#define GUAC_REALVNC_INITIAL_HEIGHT 768

/**
 * Signals that the framebuffer of the client has been updated and the next or
 * current call to guac_vnc_backend_framebuffer_wait() should not block.
 *
 * @param backend_client
 *     The guac_vnc_backend_client whose framebuffer has been updated.
 */
void guac_realvnc_notify_update(guac_vnc_backend_client* backend_client);

/**
 * Callback invoked by the RealVNC SDK when the framebuffer is being resized.
 *
 * @param backend_client
 *     The guac_vnc_backend_client associated with the active VNC connection.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the active VNC connection.
 *
 * @param width
 *     The new framebuffer width, in pixels.
 *
 * @param height
 *     The new framebuffer height, in pixels.
 */
void guac_realvnc_framebuffer_resized(void* data, vnc_Viewer* viewer,
        int width, int height);

/**
 * Callback invoked by the RealVNC SDK when a region of the VNC framebuffer has
 * been updated.
 *
 * @param backend_client
 *     The guac_vnc_backend_client associated with the active VNC connection.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the active VNC connection.
 *
 * @param x
 *     The X-coordinate of the upper-left corner of the rectangular region of
 *     the framebuffer that has been updated.
 *
 * @param y
 *     The Y-coordinate of the upper-left corner of the rectangular region of
 *     the framebuffer that has been updated.
 *
 * @param width
 *     The width of the rectangular region of the framebuffer that has been
 *     updated, in pixels.
 *
 * @param height
 *     The height of the rectangular region of the framebuffer that has been
 *     updated, in pixels.
 */
void guac_realvnc_framebuffer_updated(void* data, vnc_Viewer* viewer,
        int x, int y, int width, int height);

#endif

