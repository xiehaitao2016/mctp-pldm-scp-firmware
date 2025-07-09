#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/include"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/internal"
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src/dsp/pdr.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src/dsp/platform.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src/dsp/base.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src/dsp/fru.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libpldm/src/utils.c"
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_pldm.c")
