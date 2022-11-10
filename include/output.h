/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_OUTPUT_H
#define _LIBRPMINSPECT_OUTPUT_H

/**
 * @defgroup OUTPUT_FORMATS
 *
 * Output formats are reference by a constant.
 *
 * @{
 */

/**
 * @def FORMAT_TEXT
 *
 * Constant for the text output format (the default).
 */
#define FORMAT_TEXT    0

/**
 * @def FORMAT_JSON
 *
 * Constant for the JSON output format.
 */
#define FORMAT_JSON    1

/**
 * @def FORMAT_XUNIT
 *
 * Constant for the XUnit output format.
 */
#define FORMAT_XUNIT   2

/**
 * @def FORMAT_SUMMARY
 *
 * Constant for the summary output format.
 */
#define FORMAT_SUMMARY 3

/** @} */

#endif
