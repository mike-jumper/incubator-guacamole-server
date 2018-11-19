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

#include <vnc/Viewer.h>

/**
 * Callback which is invoked by the RealVNC SDK when the clipboard changes
 * remotely.
 *
 * @param data
 *     The guac_vnc_backend_client associated with the active VNC connection.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the active VNC connection.
 *
 * @param text
 *     The contents of the remote clipboard as a null-terminated, UTF-8 string.
 */
void guac_realvnc_clipboard_changed(void* data, vnc_Viewer* viewer,
        const char* text);

#endif

