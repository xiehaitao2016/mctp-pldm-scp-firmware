/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */

#ifndef POLL_H
#define POLL_H

#define POLLIN 0x0001

struct pollfd {
    int fd;
    short events;
    short revents;
};

#endif /* _POLL_H */
