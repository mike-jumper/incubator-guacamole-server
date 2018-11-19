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
#include "backend/input.h"
#include "common/cursor.h"
#include "common/recording.h"
#include "vnc.h"

#include <guacamole/user.h>

int guac_vnc_user_mouse_handler(guac_user* user, int x, int y, int mask) {

    guac_vnc_client* vnc_client = (guac_vnc_client*) user->client->data;
    guac_vnc_backend_client* backend_client = vnc_client->backend_client;

    /* Store current mouse location/state */
    guac_common_cursor_update(vnc_client->display->cursor, user, x, y, mask);

    /* Report mouse position within recording */
    if (vnc_client->recording != NULL)
        guac_common_recording_report_mouse(vnc_client->recording, x, y, mask);

    /* Send VNC event only if finished connecting */
    if (backend_client != NULL)
        guac_vnc_backend_send_pointer(backend_client, x, y, mask);

    return 0;
}

int guac_vnc_user_key_handler(guac_user* user, int keysym, int pressed) {

    guac_vnc_client* vnc_client = (guac_vnc_client*) user->client->data;
    guac_vnc_backend_client* backend_client = vnc_client->backend_client;

    /* Report key state within recording */
    if (vnc_client->recording != NULL)
        guac_common_recording_report_key(vnc_client->recording,
                keysym, pressed);

    /* Send VNC event only if finished connecting */
    if (backend_client != NULL)
        guac_vnc_backend_send_key(backend_client, keysym, pressed);

    return 0;
}

