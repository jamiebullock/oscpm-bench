/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "pairs.h"

namespace oscpm_compare
{

const std::vector<Pair>& pairs()
{
    static const std::vector<Pair> all = {
        { "literal", "/synth3/voice/7/osc/saw/freq", "/synth3/voice/7/osc/saw/freq", true },
        { "question", "/mixer/ch?/gain", "/mixer/ch3/gain", true },
        { "star", "/synth2/voice/*/filter/cutoff", "/synth2/voice/12/filter/cutoff", true },
        { "class", "/synth[3-6]/voice/1/osc/saw/freq", "/synth4/voice/1/osc/saw/freq", true },
        { "list", "/synth1/voice/1/osc/{saw,square}/freq", "/synth1/voice/1/osc/square/freq", true },
        { "descendant", "//freq", "/synth4/voice/12/osc/square/freq", true },
        { "combined", "/synth[3-6]/voice/*/osc/{saw,square}/freq", "/synth4/voice/12/osc/square/freq", true },
        { "combined-miss", "/synth[3-6]/voice/*/osc/{saw,square}/freq", "/synth4/voice/12/osc/square/gain", false },
        { "backtracking", "/*a*a*a*a*a*b", "/aaaaaaaaaaaaaaaaaaaaaaaa", false },
    };
    return all;
}

}
