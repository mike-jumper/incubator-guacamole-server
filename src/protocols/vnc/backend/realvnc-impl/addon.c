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

#include "addon.h"

#include <guacamole/client.h>
#include <vnc/AddOn.h>

#include <errno.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads the entire contents of the file having the given filename, storing the
 * result in a dynamically-allocated, null-terminated string. If the file
 * cannot be read for any reason, errno is set appropriately and NULL is
 * returned. The returned string must eventually be freed with a call to
 * free().
 *
 * @param filename
 *     The filename of the file to read.
 *
 * @return
 *     A dynamically-allocated, null-terminated string containing the full
 *     contents of the specified file, or NULL if the file cannot be read.
 */
static char* guac_realvnc_slurp_file(const char* filename) {

    char* buffer = NULL;
    size_t length = 0;

    /* Attempt to open input file */
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    /* Read entire file into dynamically-allocated string */
    if (getdelim(&buffer, &length, '\0', file) == -1) {
        fclose(file);
        free(buffer);
        return NULL;
    }

    /* File read successfully */
    fclose(file);
    return buffer;

}

void guac_realvnc_enable_addons(guac_client* client) {

    glob_t addons;
    if (glob(GUAC_REALVNC_ADDON_CODE_GLOB, 0, NULL, &addons))
        return;

    /* Attempt to read each file matching the predefined pattern */
    for (int i = 0; i < addons.gl_pathc; i++) {

        const char* filename = addons.gl_pathv[i];
        char* addon_code = guac_realvnc_slurp_file(filename);

        /* Warn if the add-on code could not be read */
        if (addon_code == NULL) {
            guac_client_log(client, GUAC_LOG_WARNING, "Could not read RealVNC "
                    "add-on code from file \"%s\": %s",
                    filename, strerror(errno));
            continue;
        }

        /* Attempt to enable add-on using code read from file */
        if (vnc_enableAddOn(addon_code))
            guac_client_log(client, GUAC_LOG_INFO, "Enabled RealVNC "
                    "add-on \"%s\".", filename);
        else
            guac_client_log(client, GUAC_LOG_WARNING, "RealVNC SDK did not "
                    "accept the add-on code in \"%s\". Corresponding "
                    "add-on will not be enabled.", filename);

        free(addon_code);

    }

    globfree(&addons);

}

