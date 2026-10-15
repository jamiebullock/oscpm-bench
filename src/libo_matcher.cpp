/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <optional>

extern "C" int osc_match(const char* pattern, const char* address, int* pattern_offset, int* address_offset);

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

    int matchOnce(const char* pattern, const char* address) noexcept
    {
        int patternOffset = 0;
        int addressOffset = 0;
        return osc_match(pattern, address, &patternOffset, &addressOffset);
    }

    bool isError(int result) noexcept
    {
        return (result & ~k::complete) != 0;
    }

    struct Libo
    {
        static constexpr const char* kName = "libo";
        static constexpr Form kForm = Form::RawStrings;

        struct Prepared
        {
            const char* pattern;
            const char* address;

            bool operator()() const noexcept
            {
                return matchOnce(pattern, address) == k::complete;
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            if (isError(matchOnce(pair.pattern, pair.address)))
            {
                return std::nullopt;
            }
            return Prepared { pair.pattern, pair.address };
        }
    };
}

Registration registerLibo(const std::vector<Pair>& pairs)
{
    return registerMatcher<Libo>(pairs);
}

}
