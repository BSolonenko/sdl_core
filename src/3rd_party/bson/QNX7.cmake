# Copyright (c) 2019, Ford Motor Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the
# distribution.
#
# Neither the name of the Ford Motor Company nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

include(ExternalProject)

set(CONFIGURE_FLAGS
    "--host=${CMAKE_SYSTEM_PROCESSOR}-nto-qnx"
    "--target=qnxnto"
    "--bindir=${QNX_HOST}/usr/bin/"
    "--prefix=${3RD_PARTY_INSTALL_PREFIX}"
    "LDFLAGS=-L${QNX_HOST}/usr/lib"
    "CPPFLAGS=-I${QNX_HOST}/usr/include"
    "MAKE=${QNX_HOST}/usr/bin/make${HOST_EXECUTABLE_SUFFIX}"
    "STRIP=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-strip${HOST_EXECUTABLE_SUFFIX}"
    "OBJDUMP=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-objdump${HOST_EXECUTABLE_SUFFIX}"
    "OBJCOPY=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-objcopy${HOST_EXECUTABLE_SUFFIX}"
    "LINKER=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ld"
    "NM=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-nm${HOST_EXECUTABLE_SUFFIX}"
    "RANLIB=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ranlib${HOST_EXECUTABLE_SUFFIX}"
    "AR=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-ar${HOST_EXECUTABLE_SUFFIX}"
    "CXX=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-g++${HOST_EXECUTABLE_SUFFIX}"
    "CC=${QNX_HOST}/usr/bin/nto${CMAKE_SYSTEM_PROCESSOR}-gcc${HOST_EXECUTABLE_SUFFIX}"
)

set(CONFIGURE_COMMAND
  touch aclocal.m4 configure.ac Makefile.am Makefile.in configure config.h.in && ./configure ${CONFIGURE_FLAGS})


ExternalProject_Add(
  libbson
  GIT_REPOSITORY "http://github.com/smartdevicelink/bson_c_lib.git"
  GIT_TAG "master"
  DOWNLOAD_DIR ${BSON_LIB_SOURCE_DIRECTORY}
  SOURCE_DIR ${BSON_LIB_SOURCE_DIRECTORY}
  CONFIGURE_COMMAND ${CONFIGURE_COMMAND}
  BUILD_IN_SOURCE true
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
)

set(INSTALL_COMMAND "make install")

if (${3RD_PARTY_INSTALL_PREFIX} MATCHES "/usr/local")
  set(INSTALL_COMMAND "sudo ${INSTALL_COMMAND}")
endif()

install(
  CODE "execute_process(
    WORKING_DIRECTORY ${BSON_LIB_SOURCE_DIRECTORY}
    COMMAND /bin/bash -c \"${INSTALL_COMMAND}\"
  )"
)
