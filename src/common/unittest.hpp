// SPDX-License-Identifier: GPL-3.0-only
/*
 * TCPPUMP <https://github.com/amartin755/tcppump>
 * Copyright (C) 2012-2026 Andreas Martin (netnag@mailbox.org)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UNITTEST_HPP
#define UNITTEST_HPP

#ifdef WITH_UNITTESTS
#include "bug.hpp"

/**
 * MUST_THROW macro is used to test if an expression throws an exception. 
 * If the expression does not throw an exception, the macro will call BUG() to indicate a failure in the test.
 * @param expr The expression to be tested for exception throwing.
 * @note This macro is intended for use in unit tests to ensure that certain code paths correctly throw exceptions when expected.
 */
#define MUST_THROW(expr) do { \
    bool catched = false; \
    try { \
        expr; \
    } catch (...) { \
        catched = true; \
    } \
    if (!catched) { \
        BUG ("expected to throw an exception"); \
    } \
} while (0)

#define MUST_NOT_THROW(expr) do { \
    bool catched = false; \
    try { \
        expr; \
    } catch (...) { \
        catched = true; \
    } \
    if (catched) { \
        BUG ("expected not to throw an exception"); \
    } \
} while (0)

#endif /* WITH_UNITTESTS */

#endif