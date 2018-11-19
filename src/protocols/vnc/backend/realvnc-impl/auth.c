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

void guac_realvnc_request_credentials(void* data, vnc_Viewer* viewer,
        vnc_bool_t need_user, vnc_bool_t need_password) {

    guac_vnc_backend_client* backend_client = (guac_vnc_backend_client*) data;

    /* Provide password upon request */
    if (backend_client->settings.password != NULL)
        vnc_Viewer_sendAuthenticationResponse(viewer, vnc_true, "",
                backend_client->settings.password);

    /* Cancel authentication if password required but none provided */
    else
        vnc_Viewer_sendAuthenticationResponse(viewer, vnc_false, "", "");

}

