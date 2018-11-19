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

#ifndef GUAC_VNC_BACKEND_CLIENT_INTERNAL_H
#define GUAC_VNC_BACKEND_CLIENT_INTERNAL_H

#include "backend/callbacks.h"
#include "backend/settings.h"

#include <guacamole/client.h>
#include <vnc/Viewer.h>

#include <pthread.h>
#include <stdbool.h>
#include <sys/select.h>

/**
 * All possible states of the VNC client.
 */
typedef enum guac_realvnc_state {

    /**
     * The client is newly allocated and the connection has not yet been
     * established.
     */
    GUAC_REALVNC_CONNECTING,

    /**
     * The connection was successfully established.
     */
    GUAC_REALVNC_CONNECTED,

    /**
     * The connection failed or has been closed.
     */
    GUAC_REALVNC_DISCONNECTED

} guac_realvnc_state;

struct guac_vnc_backend_client {

    /**
     * The main RealVNC SDK client thread. This thread is the ONLY thread which
     * may call RealVNC SDK functions. The RealVNC SDK will check and
     * explicitly abort (die with SIGABRT) if functions are called in multiple
     * threads, regardless of whether mutexes, etc. are being properly used to
     * ensure threadsafety.
     */
    pthread_t realvnc_thread;

    /**
     * The guac_client associated with the VNC connection.
     */
    guac_client* client;

    /**
     * The read end of the pipe used internally for notifying the main RealVNC
     * SDK thread that a user event has occurred.
     */
    int event_read_fd;

    /**
     * The write end of the pipe used internally for notifying the main RealVNC
     * SDK thread that a user event has occurred.
     */
    int event_write_fd;

    /**
     * The set of file descriptors which should be checked within the main
     * event loop and handled if data is ready to be read. This will be any
     * file descriptors provided by the RealVNC SDK as well as the read end of
     * the internal event pipe (event_read_fd).
     */
    fd_set read_fds;

    /**
     * The set of file descriptors which should be checked within the main
     * event loop and handled if data may be written. This will consist only of
     * file descriptors provided by the RealVNC SDK.
     */
    fd_set write_fds;

    /**
     * The set of file descriptors which should be checked within the main
     * event loop and handled if an exceptional condition occurs. This will
     * consist only of file descriptors provided by the RealVNC SDK.
     */
    fd_set except_fds;

    /**
     * The largest file descriptor within the read_fds, write_fds, and
     * except_fds sets.
     */
    int max_fd;

    /**
     * The RealVNC SDK client ("viewer").
     */
    vnc_Viewer* viewer;

    /**
     * The current width of the VNC framebuffer, in pixels.
     */
    int width;

    /**
     * The current height of the VNC framebuffer, in pixels.
     */
    int height;

    /**
     * The mutex associated with the state condition and property, locked
     * whenever a thread is waiting on the condition, the condition is being
     * signalled, or the property is being changed.
     */
    pthread_mutex_t state_lock;

    /**
     * Condition which is signalled when the state property has been set.
     */
    pthread_cond_t state_cond;

    /**
     * Whether the VNC client is currently connected.
     */
    guac_realvnc_state state;

    /**
     * The mutex associated with the update_received condition and flag, locked
     * whenever a thread is waiting on the condition, the condition is being
     * signalled, or the flag is being changed.
     */
    pthread_mutex_t update_received_lock;

    /**
     * Condition which is signalled when the update_received flag has been set.
     */
    pthread_cond_t update_received_cond;

    /**
     * Flag set whenever an update has been received. When set, the
     * update_received_cond condition will be signalled. The
     * update_received_lock will always be acquired before this flag is
     * altered.
     */
    bool update_received;

    /**
     * The callbacks provided at the time this VNC client backend was allocated.
     */
    guac_vnc_backend_callbacks callbacks;

    /**
     * A copy of all settings provided at the time this VNC client backend was
     * allocated.
     */
    guac_vnc_backend_settings settings;

};

/**
 * Callback which is invoked by the RealVNC SDK when the VNC connection is
 * established. Note that this function may not actually be invoked immediately
 * upon establishing the connection, particularly if the RealVNC SDK is using
 * the VNC display to prompt the user to confirm the identity of the VNC
 * server, etc. The VNC client may well be connected to the VNC server, and the
 * RealVNC SDK may be invoking callbacks to render to the framebuffer, yet the
 * connected callback will not have been invoked.
 *
 * @param data
 *     The guac_vnc_backend_client associated with the active VNC connection.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the active VNC connection.
 */
void guac_realvnc_connected(void* data, vnc_Viewer* viewer);

/**
 * Calback which is invoked by the RealVNC SDK when the VNC connection is
 * terminated.
 *
 * @param data
 *     The guac_vnc_backend_client associated with the VNC connection that just
 *     closed.
 *
 * @param viewer
 *     The vnc_Viewer instance associated with the VNC connection that just
 *     closed.
 *
 * @param reason
 *     A string error code describing the reason the connection was closed.
 *
 * @param flags
 *     Any vnc_Viewer_DisconnectFlags which apply to the connection.
 */
void guac_realvnc_disconnected(void* data, vnc_Viewer* viewer,
        const char* reason, int flags);

#endif

