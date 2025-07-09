#
# Arm SCP/MCP Software
# Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Create the firmware target.
#
add_executable(rdv3r1-mcp-bl2)

# SCP_PLATFORM_VARIANT value for supported RD-V3-R1 platform variants:
# - '0' for RD-V3-R1 platform
# - '1' for RD-V3-R1-Cfg1 platform

if (SCP_PLATFORM_VARIANT STREQUAL "0")
      target_compile_definitions(rdv3r1-mcp-bl2
                                 PUBLIC -DPLATFORM_VARIANT=0
				 PRIVATE BUILD_HAS_DEBUGGER)
elseif (SCP_PLATFORM_VARIANT STREQUAL "1")
      target_compile_definitions(rdv3r1-mcp-bl2
                                 PUBLIC -DPLATFORM_VARIANT=1)
else()
      message(FATAL_ERROR "Please set SCP_PLATFORM_VARIANT to \n"
                                "'0' for RD-V3-R1\n"
                                "'1' for RD-V3-R1-Cfg1\n")
endif()

target_compile_definitions(rdv3r1-mcp-bl2 PRIVATE BUILD_HAS_ATU_MANAGE)

target_include_directories(
    rdv3r1-mcp-bl2
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
           "${CMAKE_CURRENT_SOURCE_DIR}/../../common/include"
           "${CMAKE_CURRENT_SOURCE_DIR}/../include")

set(SCP_LOG_LEVEL "DEBUG" PARENT_SOPE)

# cmake-lint: disable=E1122

target_sources(
    rdv3r1-mcp-bl2
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config_armv7m_mpu.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_clock.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_pl011.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/config_timer.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_gtimer.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_debugger_cli.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_mctp.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_mctp_serial.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_mctp_fw.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_pldm_fw.c"
	    "${CMAKE_CURRENT_SOURCE_DIR}/config_pldm.c")

#
# Use for libpldm
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -mcpu=cortex-m7")

#
# Some of our firmware includes require CMSIS.
#

target_link_libraries(rdv3r1-mcp-bl2 PUBLIC cmsis::core-m)

#
# We explicitly add the CMSIS include directories to our interface include
# directories. Each module target adds these include directories to their own,
# allowing them to include any firmware includes we expose.
#

target_include_directories(
    rdv3r1-mcp-bl2
    PUBLIC $<TARGET_PROPERTY:cmsis::core-m,INTERFACE_INCLUDE_DIRECTORIES>)
