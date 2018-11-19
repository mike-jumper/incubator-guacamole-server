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

#ifndef GUAC_VNC_BACKEND_CLIPBOARD_H
#define GUAC_VNC_BACKEND_CLIPBOARD_H

#include "backend/client.h"

/**
 * Sends the given text to the VNC server, setting the clipboard within the VNC
 * session. The VNC standard requires that all clipboard is encoded in ISO
 * 8859-1.
 *
 * @param backend_client
 *     The VNC client to use to send the clipboard data.
 *
 * @param text
 *     The clipboard data to send, encoded in ISO 8859-1.
 *
 * @param length
 *     The length of the data to send, in bytes.
 */
void guac_vnc_backend_send_clipboard(guac_vnc_backend_client* backend_client,
        char* text, int length);

#endif

