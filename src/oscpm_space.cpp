/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <benchmark/benchmark.h>
#include <oscpm/oscpm.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace oscpm_compare
{

namespace
{
    using MemoisedSpace = oscpm::AddressSpace<int>;
    using UnmemoisedSpace = oscpm::AddressSpace<int, false>;

    template <typename Space>
    std::shared_ptr<Space> filledSpace()
    {
        auto space = std::make_shared<Space>();
        for (const std::string& text : spaceAddresses())
        {
            space->add(*oscpm::Address::parse(text), 0);
        }
        return space;
    }

    void touch(std::string_view, int& value) noexcept
    {
        benchmark::DoNotOptimize(value);
    }

    struct OscpmSpace
    {
        static constexpr const char* kName = "AddressSpace";

        struct Prepared
        {
            std::shared_ptr<MemoisedSpace> memoised;
            std::shared_ptr<UnmemoisedSpace> unmemoised;
            std::string_view message;

            std::size_t operator()() const
            {
                if (memoised)
                {
                    return memoised->visit(message, touch).matched;
                }
                return unmemoised->visit(message, touch).matched;
            }
        };

        static std::optional<Prepared> prepare(const SpaceCase& spaceCase)
        {
            const bool literal = std::string_view(spaceCase.message).find_first_of("*?[{") == std::string_view::npos;
            if (literal || spaceCase.repeated)
            {
                return Prepared { filledSpace<MemoisedSpace>(), nullptr, spaceCase.message };
            }
            return Prepared { nullptr, filledSpace<UnmemoisedSpace>(), spaceCase.message };
        }
    };
}

SpaceRegistration registerOscpmSpace(const std::vector<SpaceCase>& cases)
{
    return registerDispatcher<OscpmSpace>(cases);
}

}
