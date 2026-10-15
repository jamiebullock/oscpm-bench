/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#pragma once

#include <vector>

namespace oscpm_compare
{

/// A pattern, an address and whether the one matches the other under the OSC
/// 1.0 rules plus the OSC 1.1 `//` operator.
struct Pair
{
    const char* id;
    const char* pattern;
    const char* address;
    bool matches;
};

/// Every case the comparison times, one per pattern construct plus a literal,
/// a miss and a pattern that makes a backtracking matcher work hardest.
const std::vector<Pair>& pairs();

}
