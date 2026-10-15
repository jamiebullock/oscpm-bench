/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#pragma once

#include "pairs.h"
#include "registration.h"

#include <vector>

namespace oscpm_compare
{

/// `oscpm::match` on the two strings.
Registration registerOscpmMatch(const std::vector<Pair>& pairs);

/// `oscpm::Pattern::matches` on a parsed `Pattern` and `Address`.
Registration registerOscpmPrepared(const std::vector<Pair>& pairs);

/// liblo's `lo_pattern_match` on the two strings.
Registration registerLiblo(const std::vector<Pair>& pairs);

/// CNMAT libo's `osc_match` on the two strings.
Registration registerLibo(const std::vector<Pair>& pairs);

/// rtosc's `rtosc_match` on the two strings.
Registration registerRtosc(const std::vector<Pair>& pairs);

#if OSCPM_BENCH_JUCE
/// JUCE's `OSCAddressPattern::matches` on a constructed pattern and address.
Registration registerJuce(const std::vector<Pair>& pairs);
#endif

}
