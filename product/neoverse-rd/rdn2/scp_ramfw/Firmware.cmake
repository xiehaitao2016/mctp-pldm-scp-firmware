#
# Arm SCP/MCP Software
# Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Configure the build system.
#

set(SCP_FIRMWARE "rdn2-bl2")

set(SCP_FIRMWARE_TARGET "rdn2-bl2")

set(SCP_TOOLCHAIN_INIT "GNU")

set(SCP_GENERATE_FLAT_BINARY_INIT TRUE)
set(SCP_ENABLE_NOTIFICATIONS_INIT TRUE)

set(SCP_ENABLE_IPO_INIT FALSE)

set(SCP_ARCHITECTURE "arm-m")

set(SCP_ENABLE_NEWLIB_NANO FALSE)

set(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS TRUE)

set(SCP_TARGET_EXCLUDE_SCMI_PERF_PROTOCOL_OPS TRUE)

set(SCP_ENABLE_OUTBAND_MSG_SUPPORT_INIT TRUE)

set(SCP_ENABLE_CXL_SUPPORT TRUE)

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
     option(BUILD_HAS_MOD_TRANSPORT_FC
            "SCMI-PERF fast channel requires transport layer to be enabled" ON)
endif()

list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/platform_system")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/cmn700")
list(PREPEND SCP_MODULE_PATHS
    "${CMAKE_CURRENT_LIST_DIR}/../module/apremap")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../module/pcie_integ_ctrl")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_CURRENT_LIST_DIR}/../../module/pcie_setup")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/pcie_discovery")
list(PREPEND SCP_MODULE_PATHS
     "${CMAKE_SOURCE_DIR}/module/cxl")

# The order of the modules in the following list is the order in which the
# modules are initialized, bound, started during the pre-runtime phase.
# any change in the order will cause firmware initialization errors.

list(APPEND SCP_MODULES "armv7m-mpu")
list(APPEND SCP_MODULES "apremap")
list(APPEND SCP_MODULES "sid")
list(APPEND SCP_MODULES "system-info")
list(APPEND SCP_MODULES "pcid")
list(APPEND SCP_MODULES "pl011")
list(APPEND SCP_MODULES "ppu-v1")
list(APPEND SCP_MODULES "system-power")
list(APPEND SCP_MODULES "power-domain")
list(APPEND SCP_MODULES "pik-clock")
list(APPEND SCP_MODULES "css-clock")
list(APPEND SCP_MODULES "clock")
list(APPEND SCP_MODULES "gtimer")
list(APPEND SCP_MODULES "timer")
list(APPEND SCP_MODULES "cmn700")
list(APPEND SCP_MODULES "apcontext")
list(APPEND SCP_MODULES "mhu2")
list(APPEND SCP_MODULES "transport")
list(APPEND SCP_MODULES "scmi")
list(APPEND SCP_MODULES "sds")
list(APPEND SCP_MODULES "system-pll")
list(APPEND SCP_MODULES "scmi-power-domain")
list(APPEND SCP_MODULES "scmi-system-power")
list(APPEND SCP_MODULES "pcie_discovery")
list(APPEND SCP_MODULES "pcie-setup")
list(APPEND SCP_MODULES "pcie_integ_ctrl")
list(APPEND SCP_MODULES "cxl")
list(APPEND SCP_MODULES "platform-system")
list(APPEND SCP_MODULES "sp805")
list(APPEND SCP_MODULES "psu")
list(APPEND SCP_MODULES "mock-psu")
list(APPEND SCP_MODULES "dvfs")
list(APPEND SCP_MODULES "scmi-perf")

if(SCP_ENABLE_SCMI_PERF_FAST_CHANNELS)
     list(APPEND SCP_MODULES "fch-polled")
endif()
