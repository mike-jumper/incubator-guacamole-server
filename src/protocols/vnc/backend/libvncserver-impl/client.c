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

#include "backend/callbacks.h"
#include "backend/client.h"
#include "auth-internal.h"
#include "client-internal.h"
#include "clipboard-internal.h"
#include "cursor-internal.h"
#include "framebuffer-internal.h"

#include <guacamole/client.h>
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>

#include <stdlib.h>

char* GUAC_VNC_BACKEND_CLIENT_KEY = "GUAC_VNC";

/**
 * Sets the pixel format to request of the VNC server. The request will be made
 * during the connection handshake with the VNC server using the values
 * specified by this function. Note that the VNC server is not required to
 * honor this request.
 *
 * @param client
 *     The VNC client associated with the VNC session whose desired pixel
 *     format should be set.
 *
 * @param color_depth
 *     The desired new color depth, in bits per pixel. Valid values are 8, 16,
 *     24, and 32.
 */
static void guac_libvncclient_set_pixel_format(rfbClient* client, int color_depth) {

    client->format.trueColour = 1;

    switch (color_depth) {

        case 8:
            client->format.depth        = 8;
            client->format.bitsPerPixel = 8;
            client->format.blueShift    = 6;
            client->format.redShift     = 0;
            client->format.greenShift   = 3;
            client->format.blueMax      = 3;
            client->format.redMax       = 7;
            client->format.greenMax     = 7;
            break;

        case 16:
            client->format.depth        = 16;
            client->format.bitsPerPixel = 16;
            client->format.blueShift    = 0;
            client->format.redShift     = 11;
            client->format.greenShift   = 5;
            client->format.blueMax      = 0x1f;
            client->format.redMax       = 0x1f;
            client->format.greenMax     = 0x3f;
            break;

        case 24:
        case 32:
        default:
            client->format.depth        = 24;
            client->format.bitsPerPixel = 32;
            client->format.blueShift    = 0;
            client->format.redShift     = 16;
            client->format.greenShift   = 8;
            client->format.blueMax      = 0xff;
            client->format.redMax       = 0xff;
            client->format.greenMax     = 0xff;

    }

}

guac_vnc_backend_client* guac_vnc_backend_client_create(guac_client* client,
        guac_vnc_backend_settings* settings,
        guac_vnc_backend_callbacks* callbacks) {

    /* Allocate RFB client initialized for 32 bits per pixel */
    rfbClient* rfb_client = rfbGetClient(8, 3, 4);

    /* Allocate and initialize backend structure */
    guac_vnc_backend_client* backend_client = malloc(sizeof(guac_vnc_backend_client));
    backend_client->rfb_client = rfb_client;
    backend_client->copy_rect_used = false;
    backend_client->callbacks = *callbacks;

    /* Copy applicable settings */
    backend_client->password = settings->password;
    backend_client->swap_red_blue = settings->swap_red_blue;

    /* Store client structure in libvncclient's rfbClient */
    rfbClientSetClientData(rfb_client,
            GUAC_VNC_BACKEND_CLIENT_KEY,
            backend_client);

    /* Framebuffer update handler */
    rfb_client->GotFrameBufferUpdate = guac_libvncclient_framebuffer_update;
    rfb_client->GotCopyRect = guac_libvncclient_copyrect;

    /* Do not handle clipboard and local cursor if read-only */
    if (settings->read_only == 0) {

        /* Clipboard */
        rfb_client->GotXCutText = guac_libvncclient_cut_text;

        /* Set remote cursor */
        if (settings->remote_cursor) {
            rfb_client->appData.useRemoteCursor = FALSE;
        }

        else {
            /* Enable client-side cursor */
            rfb_client->appData.useRemoteCursor = TRUE;
            rfb_client->GotCursorShape = guac_libvncclient_cursor;
        }

    }

    /* Password */
    rfb_client->GetPassword = guac_libvncclient_get_password;

    /* Depth */
    guac_libvncclient_set_pixel_format(rfb_client, settings->color_depth);

    /* Hook into allocation so we can handle resize. */
    backend_client->rfb_MallocFrameBuffer = rfb_client->MallocFrameBuffer;
    rfb_client->MallocFrameBuffer = guac_libvncclient_malloc_framebuffer;
    rfb_client->canHandleNewFBSize = 1;

    /* Set hostname and port */
    rfb_client->serverHost = strdup(settings->hostname);
    rfb_client->serverPort = settings->port;

#ifdef ENABLE_VNC_REPEATER
    /* Set repeater parameters if specified */
    if (settings->dest_host) {
        rfb_client->destHost = strdup(settings->dest_host);
        rfb_client->destPort = settings->dest_port;
    }
#endif

#ifdef ENABLE_VNC_LISTEN
    /* If reverse connection enabled, start listening */
    if (settings->reverse_connect) {

        guac_client_log(client, GUAC_LOG_INFO, "Listening for connections on port %i", settings->port);

        /* Listen for connection from server */
        rfb_client->listenPort = settings->port;
        if (listenForIncomingConnectionsNoFork(rfb_client, settings->listen_timeout*1000) <= 0)
            return NULL;

    }
#endif

    /* Set encodings if provided */
    if (settings->encodings)
        rfb_client->appData.encodingsString = strdup(settings->encodings);

    /* Connect */
    if (rfbInitClient(rfb_client, NULL, NULL))
        return backend_client;

    /* If connection fails, return NULL */
    return NULL;

}

void guac_vnc_backend_client_free(guac_client* client,
        guac_vnc_backend_client* backend_client) {

    rfbClient* rfb_client = backend_client->rfb_client;

    /* Free memory not free'd by libvncclient's rfbClientCleanup() */
    if (rfb_client->frameBuffer != NULL) free(rfb_client->frameBuffer);
    if (rfb_client->raw_buffer != NULL) free(rfb_client->raw_buffer);
    if (rfb_client->rcSource != NULL) free(rfb_client->rcSource);

    /* Free VNC rfbClientData linked list (not free'd by rfbClientCleanup()) */
    while (rfb_client->clientData != NULL) {
        rfbClientData* next = rfb_client->clientData->next;
        free(rfb_client->clientData);
        rfb_client->clientData = next;
    }

    rfbClientCleanup(rfb_client);
    free(backend_client);

}

