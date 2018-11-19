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

#include <guacamole/client.h>

#include <vnc/Logger.h>

void guac_realvnc_log_message(void* data, vnc_Logger_Level level,
        const char* message) {

    guac_client* client = (guac_client*) data;
    guac_client_log_level guac_level;

    switch (level) {

        /* Log errors at the error level */
        case vnc_Logger_Error:
            guac_level = GUAC_LOG_ERROR;
            break;

        /* Log generic messages at the info level */
        case vnc_Logger_Basic:
            guac_level = GUAC_LOG_INFO;
            break;

        /* Log all other messages at the debug level */
        default:
            guac_level = GUAC_LOG_DEBUG;
            break;

    }

    guac_client_log(client, guac_level, "%s", message);

}

