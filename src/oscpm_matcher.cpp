/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <oscpm/oscpm.h>

#include <optional>
#include <string_view>

namespace oscpm_compare
{

namespace
{
    struct OscpmMatch
    {
        static constexpr const char* kName = "oscpm::match";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            std::string_view pattern;
            std::string_view address;

            bool operator()() const noexcept
            {
                return oscpm::match(pattern, address);
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            return Prepared { pair.pattern, pair.address };
        }
    };

    struct OscpmPrepared
    {
        static constexpr const char* kName = "Pattern::matches";
        static constexpr Form kForm = Form::Prepared;

        struct Prepared
        {
            oscpm::Pattern pattern;
            oscpm::Address address;

            bool operator()() const noexcept
            {
                return pattern.matches(address);
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            const auto pattern = oscpm::Pattern::parse(pair.pattern);
            const auto address = oscpm::Address::parse(pair.address);
            if (!pattern || !address)
            {
                return std::nullopt;
            }
            return Prepared { *pattern, *address };
        }
    };
}

Registration registerOscpmMatch(const std::vector<Pair>& pairs)
{
    return registerMatcher<OscpmMatch>(pairs);
}

Registration registerOscpmPrepared(const std::vector<Pair>& pairs)
{
    return registerMatcher<OscpmPrepared>(pairs);
}

}
