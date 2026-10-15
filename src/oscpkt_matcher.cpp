/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <oscpkt.hh>

#include <optional>
#include <string>

namespace oscpm_compare
{

namespace
{
    struct Oscpkt
    {
        static constexpr const char* kName = "oscpkt";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            std::string pattern;
            std::string address;

            bool operator()() const
            {
                return oscpkt::fullPatternMatch(pattern, address);
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            return Prepared { pair.pattern, pair.address };
        }
    };
}

Registration registerOscpkt(const std::vector<Pair>& pairs)
{
    return registerMatcher<Oscpkt>(pairs);
}

}
