#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
add_library(${SCP_MODULE_TARGET} SCP_MODULE)

# SCP_PLATFORM_VARIANT value for supported RD-V3-R1 platform variants:
# - '0' for RD-V3-R1 platform
# - '1' for RD-V3-R1-Cfg1 platform

if (SCP_PLATFORM_VARIANT STREQUAL "0")
     target_compile_definitions(${SCP_MODULE_TARGET}
                                 PUBLIC -DPLATFORM_VARIANT=0)
elseif (SCP_PLATFORM_VARIANT STREQUAL "1")
     target_compile_definitions(${SCP_MODULE_TARGET}
                                 PUBLIC -DPLATFORM_VARIANT=1)
else()
      message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                "'0' for RD-V3-R1-Cfg1\n"
                                "'1' for RD-V3-R1\n")
endif()

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(
    ${SCP_MODULE_TARGET}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scp_platform.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_lcp.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_power_mgmt.c"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/platform_rse.c")

target_link_libraries(
    ${SCP_MODULE_TARGET}
    PRIVATE module-power-domain module-system-info
            module-system-power module-transport module-timer)
