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

#include <vnc/DataBuffer.h>
#include <vnc/Viewer.h>

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

void guac_realvnc_notify_update(guac_vnc_backend_client* backend_client) {

    pthread_mutex_t* update_lock = &(backend_client->update_received_lock);
    pthread_cond_t* update_cond = &(backend_client->update_received_cond);

    /* Signal that an update has been received */
    pthread_mutex_lock(update_lock);
    backend_client->update_received = true;
    pthread_cond_signal(update_cond);
    pthread_mutex_unlock(update_lock);

    pthread_mutex_t* state_lock = &(backend_client->state_lock);
    pthread_cond_t* state_cond = &(backend_client->state_cond);

    /* Signal that connection is established ("connected" event will not
     * necessarily fire immediately upon establishing a connection, and
     * graphical updates may well be received before that event fires,
     * particularly if the RealVNC SDK needs to prompt the user to confirm the
     * server's identity) */
    pthread_mutex_lock(state_lock);
    if (backend_client->state == GUAC_REALVNC_CONNECTING)
        backend_client->state = GUAC_REALVNC_CONNECTED;
    pthread_cond_signal(state_cond);
    pthread_mutex_unlock(state_lock);

}

void guac_realvnc_framebuffer_resized(void* data, vnc_Viewer* viewer,
        int width, int height) {

    guac_vnc_backend_client* backend_client =
        (guac_vnc_backend_client*) data;

    /* Update internal tracking of width/height */
    backend_client->width = width;
    backend_client->height = height;

    /* Notify of resize */
    backend_client->callbacks.framebuffer_resized(width, height,
            backend_client->callbacks.data);

    /* Allocate framebuffer with new size */
    vnc_Viewer_setViewerFb(viewer, NULL, 0, vnc_PixelFormat_rgb888(),
            width, height, 0);

    /* Notify that an update has been received */
    guac_realvnc_notify_update(backend_client);

}

void guac_realvnc_framebuffer_updated(void* data, vnc_Viewer* viewer,
        int x, int y, int width, int height) {

    guac_vnc_backend_client* backend_client =
        (guac_vnc_backend_client*) data;

    const vnc_DataBuffer* data_buffer = vnc_Viewer_getViewerFbData(viewer,
            x, y, width, height);

    int buffer_size;
    const unsigned char* buffer = vnc_DataBuffer_getData(data_buffer, &buffer_size);

    /* Notify of update */
    backend_client->callbacks.framebuffer_updated(x, y, buffer, width, height,
            vnc_Viewer_getViewerFbStride(viewer) * 4,
            backend_client->callbacks.data);

    /* Notify that an update has been received */
    guac_realvnc_notify_update(backend_client);

}

/**
 * Populate the given timespec with the current time, plus the given offset.
 *
 * @param ts
 *     The timespec structure to populate.
 *
 * @param offset_sec
 *     The offset from the current time to use when populating the given
 *     timespec, in seconds.
 *
 * @param offset_usec
 *     The offset from the current time to use when populating the given
 *     timespec, in microseconds.
 */
static void guac_realvnc_get_absolute_time(struct timespec* ts,
        int offset_sec, int offset_usec) {

    /* Get timeval */
    struct timeval tv;
    gettimeofday(&tv, NULL);

    /* Update with offset */
    tv.tv_sec  += offset_sec;
    tv.tv_usec += offset_usec;

    /* Wrap to next second if necessary */
    if (tv.tv_usec >= 1000000) {
        tv.tv_sec++;
        tv.tv_usec -= 1000000;
    }

    /* Convert to timespec */
    ts->tv_sec  = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;

}

int guac_vnc_backend_framebuffer_wait(guac_vnc_backend_client* backend_client,
        int timeout) {

    int retval = 1;

    pthread_mutex_t* update_lock = &(backend_client->update_received_lock);
    pthread_cond_t* update_cond = &(backend_client->update_received_cond);

    /* Split provided milliseconds into microseconds and whole seconds */
    int secs  =  timeout / 1000;
    int usecs = (timeout % 1000) * 1000;

    /* Calculate absolute timestamp from provided relative timeout */
    struct timespec ts_timeout;
    guac_realvnc_get_absolute_time(&ts_timeout, secs, usecs);

    /* Test whether an update has already been received */
    pthread_mutex_lock(update_lock);
    if (backend_client->update_received)
        goto wait_complete;

    /* If update not yet received, wait for condition to be signaled */
    retval = pthread_cond_timedwait(update_cond,
            update_lock, &ts_timeout) != ETIMEDOUT;

wait_complete:

    /* Reset flag */
    backend_client->update_received = false;
    pthread_mutex_unlock(update_lock);
    return retval;

}

int guac_vnc_backend_framebuffer_get_width(guac_vnc_backend_client* backend_client) {
    return backend_client->width;
}

int guac_vnc_backend_framebuffer_get_height(guac_vnc_backend_client* backend_client) {
    return backend_client->height;
}

