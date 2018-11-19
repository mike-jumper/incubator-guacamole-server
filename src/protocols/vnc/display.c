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

#include "config.h"
#include "common/surface.h"
#include "vnc.h"

#include <cairo/cairo.h>
#include <guacamole/client.h>

void guac_vnc_framebuffer_updated(int x, int y, const unsigned char* image,
        int width, int height, int stride, void* data) {

    guac_client* client = (guac_client*) data;
    guac_vnc_client* vnc_client = (guac_vnc_client*) client->data;

    /* Ignore update if display not yet allocated */
    if (vnc_client->display == NULL)
        return;

    /* Create surface from received image buffer */
    cairo_surface_t* surface = cairo_image_surface_create_for_data(
            (unsigned char*) image /* We won't be modifying this data */,
            CAIRO_FORMAT_RGB24, width, height, stride);

    /* Draw directly to default layer */
    guac_common_surface_draw(vnc_client->display->default_surface,
        x, y, surface);

    /* Surface is no longer needed */
    cairo_surface_destroy(surface);

}

void guac_vnc_framebuffer_copied(int sx, int sy, int width, int height,
        int dx, int dy, void* data) {

    guac_client* client = (guac_client*) data;
    guac_vnc_client* vnc_client = (guac_vnc_client*) client->data;

    /* Ignore update if display not yet allocated */
    if (vnc_client->display == NULL)
        return;

    /* Copy specified rectangle within default layer */
    guac_common_surface_copy(vnc_client->display->default_surface,
            sx, sy, width, height,
            vnc_client->display->default_surface, dx, dy);

}

void guac_vnc_framebuffer_resized(int width, int height, void* data) {

    guac_client* client = (guac_client*) data;
    guac_vnc_client* vnc_client = (guac_vnc_client*) client->data;

    /* Ignore update if display not yet allocated */
    if (vnc_client->display == NULL)
        return;

    /* Resize surface */
    guac_common_surface_resize(vnc_client->display->default_surface,
            width, height);

}

