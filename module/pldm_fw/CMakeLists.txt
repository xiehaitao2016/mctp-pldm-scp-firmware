#
# Arm SCP/MCP Software
# Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(
    ${SCP_MODULE_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/internal")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_pldm_fw.c"
                       "${CMAKE_CURRENT_SOURCE_DIR}/src/bmc.c"
                       "${CMAKE_CURRENT_SOURCE_DIR}/src/mcp.c")

target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-pldm)
target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-mctp_fw)
