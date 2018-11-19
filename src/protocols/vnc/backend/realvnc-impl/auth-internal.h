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

#ifndef GUAC_VNC_BACKEND_AUTH_INTERNAL_H
#define GUAC_VNC_BACKEND_AUTH_INTERNAL_H

#include <vnc/Viewer.h>

/**
 * Callback invoked by the RealVNC SDK when credentials are being requested by
 * the VNC server. The credentials, if any, must be sent using
 * vnc_Viewer_sendAuthenticationResponse().
 *
 * @param data
 *     The guac_vnc_backend_client associated with the VNC connection.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the VNC connection.
 *
 * @param need_user
 *     true if a username is requested, false otherwise.
 *
 * @param need_password
 *     true if a password is requested, false otherwise.
 */
void guac_realvnc_request_credentials(void* data, vnc_Viewer* viewer,
        vnc_bool_t need_user, vnc_bool_t need_password);

#endif

