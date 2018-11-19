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

#ifndef GUAC_VNC_BACKEND_CLIENT_H
#define GUAC_VNC_BACKEND_CLIENT_H

#include <guacamole/client.h>

#include "backend/callbacks.h"
#include "backend/settings.h"

/**
 * The VNC client. The actual contents of this structure are intentionally
 * opaque, known only to the backend implementation.
 */
typedef struct guac_vnc_backend_client guac_vnc_backend_client;

/**
 * Creates and connects a new VNC client instance, applying the given settings
 * to the underlying connection. If the connection fails, NULL is returned,
 * appropriate messages describing the failure should be logged, and the
 * provided guac_client is aborted.
 *
 * If a provided setting is not supported by the underlying VNC implementation,
 * an appropriate warning should be logged. It is up to the implementation to
 * decide whether lack of support for a particular setting should be considered
 * a connection failure.
 *
 * @param client
 *     The guac_client associated with the VNC connection being established.
 *
 * @param settings
 *     A pointer to a guac_vnc_backend_settings structure defining the nature
 *     of the VNC connection to be established.
 *
 * @return
 *     A newly-allocated guac_vnc_backend_client instance which must eventually
 *     be freed through a call to guac_vnc_backend_free(), or NULL if the
 *     connection fails.
 */
guac_vnc_backend_client* guac_vnc_backend_client_create(guac_client* client,
        guac_vnc_backend_settings* settings,
        guac_vnc_backend_callbacks* callbacks);

/**
 * Frees the given guac_vnc_backend_client instance, disconnecting the
 * underlying VNC connection if it is still established.
 *
 * @param client
 *     The guac_client associated with the VNC connection.
 *
 * @param backend_client
 *     The VNC client to disconnect (if connected) and free.
 */
void guac_vnc_backend_client_free(guac_client* client,
        guac_vnc_backend_client* backend_client);

#endif

