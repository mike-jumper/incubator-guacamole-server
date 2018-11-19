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

#include "config.h"

#include "backend/backend.h"
#include "backend/client.h"
#include "common/recording.h"
#include "client.h"
#include "user.h"
#include "vnc.h"

#ifdef ENABLE_COMMON_SSH
#include "common-ssh/sftp.h"
#include "common-ssh/ssh.h"
#include "common-ssh/user.h"
#endif

#ifdef ENABLE_PULSE
#include "pulse/pulse.h"
#endif

#include <guacamole/client.h>

#include <stdlib.h>
#include <string.h>

int guac_client_init(guac_client* client) {

    /* Initialize VNC backend */
    guac_vnc_backend_init(client);

    /* Set client args */
    client->args = GUAC_VNC_CLIENT_ARGS;

    /* Alloc client data */
    guac_vnc_client* vnc_client = calloc(1, sizeof(guac_vnc_client));
    client->data = vnc_client;

    /* Init clipboard */
    vnc_client->clipboard = guac_common_clipboard_alloc(GUAC_VNC_CLIPBOARD_MAX_LENGTH);

    /* Set handlers */
    client->join_handler = guac_vnc_user_join_handler;
    client->leave_handler = guac_vnc_user_leave_handler;
    client->free_handler = guac_vnc_client_free_handler;

    return 0;
}

int guac_vnc_client_free_handler(guac_client* client) {

    guac_vnc_client* vnc_client = (guac_vnc_client*) client->data;
    guac_vnc_settings* settings = vnc_client->settings;

    /* Wait for client thread to finish */
    if (vnc_client->client_thread_created)
        pthread_join(vnc_client->client_thread, NULL);

    /* Clean up VNC backend client */
    if (vnc_client->backend_client)
        guac_vnc_backend_client_free(client, vnc_client->backend_client);

    /* Shutdown VNC backend */
    guac_vnc_backend_shutdown();

#ifdef ENABLE_COMMON_SSH
    /* Free SFTP filesystem, if loaded */
    if (vnc_client->sftp_filesystem)
        guac_common_ssh_destroy_sftp_filesystem(vnc_client->sftp_filesystem);

    /* Free SFTP session */
    if (vnc_client->sftp_session)
        guac_common_ssh_destroy_session(vnc_client->sftp_session);

    /* Free SFTP user */
    if (vnc_client->sftp_user)
        guac_common_ssh_destroy_user(vnc_client->sftp_user);

    guac_common_ssh_uninit();
#endif

    /* Clean up recording, if in progress */
    if (vnc_client->recording != NULL)
        guac_common_recording_free(vnc_client->recording);

    /* Free clipboard */
    if (vnc_client->clipboard != NULL)
        guac_common_clipboard_free(vnc_client->clipboard);

    /* Free display */
    if (vnc_client->display != NULL)
        guac_common_display_free(vnc_client->display);

#ifdef ENABLE_PULSE
    /* If audio enabled, stop streaming */
    if (vnc_client->audio)
        guac_pa_stream_free(vnc_client->audio);
#endif

    /* Free parsed settings */
    if (settings != NULL)
        guac_vnc_settings_free(settings);

    /* Free generic data struct */
    free(client->data);

    return 0;
}

