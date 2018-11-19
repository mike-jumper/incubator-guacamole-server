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

#include <vnc/Viewer.h>

#include <string.h>

void guac_realvnc_clipboard_changed(void* data, vnc_Viewer* viewer,
        const char* text) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;

    /* FIXME: serverClipboardTextChanged is defined as accepting UTF-8 */

    /* Notify of received clipboard data */
    backend_client->callbacks.clipboard_received(text, strlen(text),
            backend_client->callbacks.data);

}

void guac_vnc_backend_send_clipboard(guac_vnc_backend_client* backend_client,
        char* text, int length) {
    /* FIXME: Implement */
}

