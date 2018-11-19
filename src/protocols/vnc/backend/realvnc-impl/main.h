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

#ifndef GUAC_VNC_BACKEND_MAIN_H
#define GUAC_VNC_BACKEND_MAIN_H

/**
 * The full path to the file that the RealVNC SDK should use for storage of
 * arbitrary, sensitive data, including RSA keys. This file will be created
 * automatically, however the path containing the file must already exist.
 */
#define GUAC_REALVNC_DATASTORE_PATH "/etc/guacamole/realvnc/datastore"

/**
 * The main RealVNC thread. This is the ONLY thread which may invoke any
 * RealVNC SDK functions whatsoever. The RealVNC SDK explicitly checks the
 * current thread ID at the beginning of many function calls and asserts that
 * it matches the thread ID of the original call to vnc_init(). Regardless of
 * whether proper mutexes, etc. are used to use the RealVNC structures or
 * functions in a threadsafe manner, the functions themselves will
 * intentionally bail out if they detect multithreaded use.
 *
 * Other threads which must effectively invoke RealVNC functions must instead
 * use guac_realvnc_event_write() to write an event packet to the
 * event_write_fd of the guac_vnc_backend_client. This main thread will then
 * read that packet and call the required function.
 *
 * @param data
 *     The guac_vnc_backend_client instance associated with the active VNC
 *     connection.
 *
 * @return
 *     Always NULL.
 */
void* guac_realvnc_event_loop(void* data);

#endif

