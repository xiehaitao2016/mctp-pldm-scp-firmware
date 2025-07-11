/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_string.h>
#include <fwk_test.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

void test_fwk_str_memset(void)
{
    char dest[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    size_t count = 8;
    int ch = 0x11;

    fwk_str_memset((void *)&dest[0], ch, count);

    for (size_t i = 0; i < count; i++) {
        assert(dest[i] == ch);
    }
}

static void test_fwk_str_memcpy(void)
{
    const char src[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    char dest[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    size_t count = 8;

    fwk_str_memcpy((void *)&dest[0], (const void *)&src[0], count);

    for (size_t i = 0; i < count; i++) {
        assert(dest[i] == src[i]);
    }
}

static void test_fwk_str_strncpy(void)
{
    const char src[] = "FWK_STRING_TEST";
    size_t count = sizeof(src) - 1;
    char dest[15];

    fwk_str_strncpy(&dest[0], &src[0], count);

    for (size_t i = 0; i < count; i++) {
        assert(dest[i] == src[i]);
    }
}

static void test_fwk_str_is_in_boundry_invalid_param_large_size(void)
{
    size_t offset = 50;
    size_t buffer_size = 100;

    bool result =
        fwk_str_is_in_boundry(buffer_size, offset, SIZE_MAX - (offset / 2));

    assert(result == false);
}

static void test_fwk_str_is_in_boundry_invalid_param_large_offset(void)
{
    size_t buffer_size = 100;
    size_t size = 5;

    bool result = fwk_str_is_in_boundry(buffer_size, SIZE_MAX - 2, size);

    assert(result == false);
}

static void test_fwk_str_is_in_boundry_invalid_param_beyond_end(void)
{
    size_t buffer_size = 100;
    size_t size = 1;

    bool result = fwk_str_is_in_boundry(buffer_size, buffer_size, size);

    assert(result == false);
}

static void test_fwk_str_is_in_boundry_valid_param_before_end(void)
{
    size_t buffer_size = 100;
    size_t size = 1;

    bool result = fwk_str_is_in_boundry(buffer_size, buffer_size - 1, size);

    assert(result == true);
}

static const struct fwk_test_case_desc test_case_table[] = {
    FWK_TEST_CASE(test_fwk_str_memset),
    FWK_TEST_CASE(test_fwk_str_memcpy),
    FWK_TEST_CASE(test_fwk_str_strncpy),
    FWK_TEST_CASE(test_fwk_str_is_in_boundry_invalid_param_large_size),
    FWK_TEST_CASE(test_fwk_str_is_in_boundry_invalid_param_large_offset),
    FWK_TEST_CASE(test_fwk_str_is_in_boundry_invalid_param_beyond_end),
    FWK_TEST_CASE(test_fwk_str_is_in_boundry_valid_param_before_end),
};

struct fwk_test_suite_desc test_suite = {
    .name = "fwk_string",
    .test_case_count = FWK_ARRAY_SIZE(test_case_table),
    .test_case_table = test_case_table,
};
