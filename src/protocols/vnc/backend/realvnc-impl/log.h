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

#ifndef GUAC_VNC_BACKEND_LOG_H
#define GUAC_VNC_BACKEND_LOG_H

#include <guacamole/client.h>

#include <vnc/Logger.h>

/**
 * RealVNC SDK logger implementation which logs messages using the logging
 * facilities of guac_client.
 *
 * @param data
 *     The guac_client whose logging facilities should be used.
 *
 * @param level
 *     The log level at which the message should be logged.
 *
 * @param message
 *     The message to log.
 */
void guac_realvnc_log_message(void* data, vnc_Logger_Level level,
        const char* message);

#endif

