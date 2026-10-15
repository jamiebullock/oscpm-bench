/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <rtosc/rtosc.h>

#include <optional>
#include <string_view>

namespace oscpm_compare
{

namespace
{
    bool starEndsSegment(std::string_view pattern) noexcept
    {
        for (std::size_t i = pattern.find('*'); i != std::string_view::npos; i = pattern.find('*', i + 1))
        {
            const bool atEnd = i + 1 == pattern.size();
            if (!atEnd && pattern[i + 1] != '/')
            {
                return false;
            }
        }
        return true;
    }

    bool takesPattern(std::string_view pattern) noexcept
    {
        const bool hasLiteralOnlyConstruct = pattern.find_first_of("?[#:") != std::string_view::npos;
        return !hasLiteralOnlyConstruct && starEndsSegment(pattern);
    }

    struct Rtosc
    {
        static constexpr const char* kName = "rtosc";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            const char* pattern;
            const char* address;

            bool operator()() const noexcept
            {
                return rtosc_match(pattern, address, nullptr);
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            if (!takesPattern(pair.pattern))
            {
                return std::nullopt;
            }
            return Prepared { pair.pattern, pair.address };
        }
    };
}

Registration registerRtosc(const std::vector<Pair>& pairs)
{
    return registerMatcher<Rtosc>(pairs);
}

}
