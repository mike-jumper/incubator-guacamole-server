#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# NOTE: Parts of this file (Makefile.am) are automatically transcluded verbatim
# into Makefile.in. Though the build system (GNU Autotools) automatically adds
# its own license boilerplate to the generated Makefile.in, that boilerplate
# does not apply to the transcluded portions of Makefile.am which are licensed
# to you by the ASF under the Apache License, Version 2.0, as described above.
#

AUTOMAKE_OPTIONS = foreign 
ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libguac_vnc_backend.la

#
# Common interface
#

noinst_HEADERS =  \
    backend.h     \
    callbacks.h   \
    client.h      \
    clipboard.h   \
    framebuffer.h \
    input.h       \
    settings.h

libguac_vnc_backend_la_CFLAGS = \
    -Werror -Wall -pedantic     \
    @LIBGUAC_INCLUDE@           \
    @VNC_BACKEND_INCLUDE@

libguac_vnc_backend_la_LIBADD = \
    @LIBGUAC_LTLIB@             \
    @CAIRO_LIBS@                \
    @VNC_LIBS@

#
# RealVNC SDK implementation
#

if ENABLE_REALVNC
noinst_HEADERS +=                  \
    realvnc-impl/client-internal.h

libguac_vnc_backend_la_SOURCES = \
    realvnc-impl/backend.c       \
    realvnc-impl/client.c        \
    realvnc-impl/clipboard.c     \
    realvnc-impl/framebuffer.c   \
    realvnc-impl/input.c
endif

#
# libVNCServer (libvncclient) implementation
#

if ENABLE_LIBVNCSERVER
noinst_HEADERS +=                            \
    libvncserver-impl/auth-internal.h        \
    libvncserver-impl/client-internal.h      \
    libvncserver-impl/clipboard-internal.h   \
    libvncserver-impl/cursor-internal.h      \
    libvncserver-impl/framebuffer-internal.h

libguac_vnc_backend_la_SOURCES =    \
    libvncserver-impl/auth.c        \
    libvncserver-impl/backend.c     \
    libvncserver-impl/client.c      \
    libvncserver-impl/clipboard.c   \
    libvncserver-impl/cursor.c      \
    libvncserver-impl/framebuffer.c \
    libvncserver-impl/input.c
endif

