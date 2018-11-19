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

#ifndef GUAC_VNC_BACKEND_ADDON_H
#define GUAC_VNC_BACKEND_ADDON_H

#include <guacamole/client.h>

/**
 * The pattern which matches all RealVNC add-on code files. These files will be
 * read automatically if guac_realvnc_enable_addons() is invoked.
 */
#define GUAC_REALVNC_ADDON_CODE_GLOB "/etc/guacamole/realvnc/*.addon"

/**
 * Reads and enables the add-ons corresponding to the RealVNC add-on codes in
 * each of the files matching GUAC_REALVNC_ADDON_CODE_GLOB. If any file cannot
 * be read, a warning is logged and the operation proceeds with the next
 * matching file.
 *
 * @param client
 *     The guac_client associated with the VNC connection leveraging the
 *     RealVNC SDK.
 */
void guac_realvnc_enable_addons(guac_client* client);

#endif

