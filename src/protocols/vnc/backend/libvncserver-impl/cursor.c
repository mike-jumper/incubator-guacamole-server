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

#include <guacamole/client.h>
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>

#include <stdint.h>
#include <stdlib.h>

void guac_libvncclient_cursor(rfbClient* rfb_client,
        int x, int y, int width, int height, int bpp) {

    guac_vnc_backend_client* backend_client =
        rfbClientGetClientData(rfb_client, GUAC_VNC_BACKEND_CLIENT_KEY);

    /* Output image buffer */
    int stride = width * 4;
    unsigned char* buffer = malloc(height * stride);
    unsigned char* buffer_row_current = buffer;

    /* VNC image buffer */
    unsigned int fb_stride = bpp * width;
    unsigned char* fb_row_current = rfb_client->rcSource;
    unsigned char* fb_mask = rfb_client->rcMask;

    /* Copy image data from VNC client to RGBA buffer */
    for (int dy = 0; dy < height; dy++) {

        /* Get current buffer row, advance to next */
        uint32_t* buffer_current = (uint32_t*) buffer_row_current;
        buffer_row_current += stride;

        /* Get current framebuffer row, advance to next */
        unsigned char* fb_current = fb_row_current;
        fb_row_current += fb_stride;

        for (int dx = 0; dx < width; dx++) {

            unsigned char alpha, red, green, blue;
            unsigned int v;

            /* Read current pixel value */
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

            /* Translate mask to alpha */
            if (*(fb_mask++)) alpha = 0xFF;
            else              alpha = 0x00;

            /* Translate value to RGB */
            red   = (v >> rfb_client->format.redShift)   * 0x100 / (rfb_client->format.redMax  + 1);
            green = (v >> rfb_client->format.greenShift) * 0x100 / (rfb_client->format.greenMax+ 1);
            blue  = (v >> rfb_client->format.blueShift)  * 0x100 / (rfb_client->format.blueMax + 1);

            /* Output ARGB */
            if (backend_client->swap_red_blue)
                *(buffer_current++) = (alpha << 24) | (blue << 16) | (green << 8) | red;
            else
                *(buffer_current++) = (alpha << 24) | (red  << 16) | (green << 8) | blue;

            /* Next VNC pixel */
            fb_current += bpp;

        }
    }

    /* Notify of updated cursor */
    backend_client->callbacks.cursor_updated(x, y, buffer, width, height,
            stride, backend_client->callbacks.data);

    /* Free surface */
    free(buffer);

    /* libvncclient does not free rcMask as it does rcSource */
    free(rfb_client->rcMask);

}

