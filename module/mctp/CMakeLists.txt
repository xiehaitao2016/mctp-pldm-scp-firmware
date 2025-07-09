#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-type-limits")

add_definitions (-DMCTP_HAVE_FILEIO)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libmctp")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/alloc.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/crc32.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/core.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/log.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/serial.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libmctp/crc-16-ccitt.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pcc.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_mctp.c")
