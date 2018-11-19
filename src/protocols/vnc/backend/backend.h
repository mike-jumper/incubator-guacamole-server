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

#ifndef GUAC_VNC_BACKEND_H
#define GUAC_VNC_BACKEND_H

#include <guacamole/client.h>

/**
 * Performs any necessary static initialization of the backend which must occur
 * before any connection attempt is made. This function MUST be invoked before
 * any other VNC backend function is called.
 *
 * @param client
 *     The guac_client associated with the VNC connection that will eventually
 *     be established.
 */
void guac_vnc_backend_init(guac_client* client);

/**
 * Performs any necessary static cleanup tasks required by the backend after it
 * will no longer be used by the current process. This function MUST be invoked
 * after the VNC backend will no longer be used.
 */
void guac_vnc_backend_shutdown();

#endif

