/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <optional>

extern "C" int lo_pattern_match(const char* str, const char* p);

namespace oscpm_compare
{

namespace
{
    struct Liblo
    {
        static constexpr const char* kName = "liblo";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            const char* pattern;
            const char* address;

            bool operator()() const noexcept
            {
                return lo_pattern_match(address, pattern) != 0;
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            return Prepared { pair.pattern, pair.address };
        }
    };
}

Registration registerLiblo(const std::vector<Pair>& pairs)
{
    return registerMatcher<Liblo>(pairs);
}

}
