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

#include "client.h"
#include "backend/client.h"
#include "client-internal.h"

#include <cairo/cairo.h>
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Define cairo_format_stride_for_width() if missing */
#ifndef HAVE_CAIRO_FORMAT_STRIDE_FOR_WIDTH
#define cairo_format_stride_for_width(format, width) (width*4)
#endif

void guac_libvncclient_framebuffer_update(rfbClient* rfb_client, int x, int y,
        int width, int height) {

    guac_vnc_backend_client* backend_client =
        rfbClientGetClientData(rfb_client, GUAC_VNC_BACKEND_CLIENT_KEY);

    /* Ignore extra update if already handled by copyrect */
    if (backend_client->copy_rect_used) {
        backend_client->copy_rect_used = false;
        return;
    }

    /* Raw output buffer */
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width);
    unsigned char* buffer = malloc(height * stride);
    unsigned char* buffer_row_current = buffer;

    /* VNC framebuffer */
    unsigned int bpp = rfb_client->format.bitsPerPixel / 8;
    unsigned int fb_stride = bpp * rfb_client->width;
    unsigned char* fb_row_current = rfb_client->frameBuffer + (y * fb_stride) + (x * bpp);

    /* Copy image data from VNC client to raw RGB buffer */
    for (int dy = y; dy < y + height; dy++) {

        /* Get current buffer row, advance to next */
        uint32_t* buffer_current = (uint32_t*) buffer_row_current;
        buffer_row_current += stride;

        /* Get current framebuffer row, advance to next */
        unsigned char* fb_current = fb_row_current;
        fb_row_current += fb_stride;

        for (int dx = x; dx < x + width; dx++) {

            unsigned char red, green, blue;
            unsigned int v;

            switch (bpp) {
                case 4:
                    v = *((uint32_t*) fb_current);
                    break;

                case 2:
                    v = *((uint16_t*) fb_current);
                    break;

                default:
                    v = *((uint8_t*)  fb_current);
            }

            /* Translate value to RGB */
            red   = (v >> rfb_client->format.redShift)   * 0x100 / (rfb_client->format.redMax  + 1);
            green = (v >> rfb_client->format.greenShift) * 0x100 / (rfb_client->format.greenMax+ 1);
            blue  = (v >> rfb_client->format.blueShift)  * 0x100 / (rfb_client->format.blueMax + 1);

            /* Output RGB */
            if (backend_client->swap_red_blue)
                *(buffer_current++) = (blue << 16) | (green << 8) | red;
            else
                *(buffer_current++) = (red  << 16) | (green << 8) | blue;

            fb_current += bpp;

        }
    }

    /* Notify of update */
    backend_client->callbacks.framebuffer_updated(x, y, buffer, width, height,
            stride, backend_client->callbacks.data);

    /* Done with raw output buffer */
    free(buffer);

}

void guac_libvncclient_copyrect(rfbClient* rfb_client, int sx, int sy,
        int width, int height, int dx, int dy) {

    guac_vnc_backend_client* backend_client =
        rfbClientGetClientData(rfb_client, GUAC_VNC_BACKEND_CLIENT_KEY);

    /* Notify of CopyRect update */
    backend_client->callbacks.framebuffer_copied(sx, sy, width, height,
            dx, dy, backend_client->callbacks.data);

    /* Ignore upcoming invocation of generic update callback */
    backend_client->copy_rect_used = true;

}

rfbBool guac_libvncclient_malloc_framebuffer(rfbClient* rfb_client) {

    guac_vnc_backend_client* backend_client =
        rfbClientGetClientData(rfb_client, GUAC_VNC_BACKEND_CLIENT_KEY);

    /* Notify of resize */
    backend_client->callbacks.framebuffer_resized(rfb_client->width,
            rfb_client->height, backend_client->callbacks.data);

    /* Complete libvncclient resize operation */
    return backend_client->rfb_MallocFrameBuffer(rfb_client);

}

int guac_vnc_backend_framebuffer_wait(guac_vnc_backend_client* backend_client,
        int timeout) {

    int wait_result;

    rfbClient* rfb_client = backend_client->rfb_client;

    /* Explicitly wait only if data is not on the buffer */
    if (rfb_client->buffered)
        wait_result = 1;
    else
        wait_result = WaitForMessage(rfb_client, timeout * 1000);

    /* Request that libvncclient process messages only if doing so will not
     * block */
    if (wait_result > 0) {

        /* Abort the connection if the VNC server sent us erroneous data */
        if (!HandleRFBServerMessage(rfb_client)) {
            guac_client_abort(backend_client->client,
                    GUAC_PROTOCOL_STATUS_UPSTREAM_ERROR,
                    "Error handling message from VNC server.");
            return -1;
        }

    }

    return wait_result;

}

int guac_vnc_backend_framebuffer_get_width(guac_vnc_backend_client* backend_client) {
    return backend_client->rfb_client->width;
}

int guac_vnc_backend_framebuffer_get_height(guac_vnc_backend_client* backend_client) {
    return backend_client->rfb_client->height;
}

