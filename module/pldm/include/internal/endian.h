/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ENDIAN_H
#define ENDIAN_H

#include <config.h>

#ifndef htole16
#    define htole16(x) (x)
#endif
#ifndef htole32
#    define htole32(x) (x)
#endif
#ifndef htole64
#    define htole64(x) (x)
#endif

#ifndef le16toh
#    define le16toh(x) (x)
#endif

#ifndef le32toh
#    define le32toh(x) (x)
#endif

#ifndef le64toh
#    define le64toh(x) (x)
#endif

#endif /* ENDIAN_H */
