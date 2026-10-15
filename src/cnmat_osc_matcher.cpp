/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <optional>

extern "C" int cnmat_osc_match(const char* pattern, const char* address, int* pattern_offset, int* address_offset);

namespace oscpm_compare
{

namespace
{
    namespace k
    {
        constexpr int addressComplete = 1;
        constexpr int patternComplete = 2;
        constexpr int complete = addressComplete | patternComplete;
    }

    struct CnmatOsc
    {
        static constexpr const char* kName = "CNMAT OSC";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            const char* pattern;
            const char* address;

            bool operator()() const noexcept
            {
                int patternOffset = 0;
                int addressOffset = 0;
                return cnmat_osc_match(pattern, address, &patternOffset, &addressOffset) == k::complete;
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            return Prepared { pair.pattern, pair.address };
        }
    };
}

Registration registerCnmatOsc(const std::vector<Pair>& pairs)
{
    return registerMatcher<CnmatOsc>(pairs);
}

}
