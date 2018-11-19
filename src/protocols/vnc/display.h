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

#ifndef GUAC_VNC_DISPLAY_H
#define GUAC_VNC_DISPLAY_H

#include "backend/callbacks.h"

/**
 * Callback invoked by the VNC backend when a rectangle of framebuffer data is
 * changed.
 *
 * @see guac_vnc_backend_framebuffer_updated
 */
guac_vnc_backend_framebuffer_updated guac_vnc_framebuffer_updated;

/**
 * Callback invoked by the VNC backend when a rectangle of framebuffer data is
 * copied from one location to another.
 *
 * @see guac_vnc_backend_framebuffer_copied
 */
guac_vnc_backend_framebuffer_copied guac_vnc_framebuffer_copied;

/**
 * Callback invoked by the VNC backend when the framebuffer size is changed.
 *
 * @see guac_vnc_backend_framebuffer_resized
 */
guac_vnc_backend_framebuffer_resized guac_vnc_framebuffer_resized;

#endif

