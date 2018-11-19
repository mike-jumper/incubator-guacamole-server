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

#ifndef GUAC_VNC_BACKEND_CLIPBOARD_INTERNAL_H
#define GUAC_VNC_BACKEND_CLIPBOARD_INTERNAL_H

#include <rfb/rfbclient.h>

/**
 * Handler for clipboard data received via VNC, invoked by libVNCServer
 * whenever text has been copied or cut within the VNC session.
 *
 * @param rfb_client
 *     The VNC client associated with the session in which the user cut or
 *     copied text.
 *
 * @param text
 *     A buffer containing the cut/copied text.
 *
 * @param length
 *     The number of bytes of cut/copied text in the buffer.
 */
void guac_libvncclient_cut_text(rfbClient* rfb_client,
        const char* text, int length);

#endif

