/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#pragma once

#include "pairs.h"
#include "space.h"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace oscpm_compare
{

/// Whether a matcher is timed on the two strings as received, or on objects
/// it parsed from them before the timed loop.
enum class Form
{
    RawStrings,
    Prepared,
};

constexpr const char* formName(Form form) noexcept
{
    return form == Form::RawStrings ? "raw" : "prepared";
}

/// A matcher's answer for one pair; empty when the matcher cannot take the
/// pattern or the address.
struct Answer
{
    const char* pairId;
    std::optional<bool> matches;
};

/// The outcome of registering one matcher: its name, its form and its answer
/// for every pair, in the order of `pairs()`.
struct Registration
{
    const char* name;
    Form form;
    std::vector<Answer> answers;
};

/// A dispatcher's count of reached addresses for one case; empty when the
/// dispatcher cannot take the message.
struct SpaceAnswer
{
    const char* caseId;
    std::optional<std::size_t> numMatches;
};

/// The outcome of registering one dispatcher: its name and its count for
/// every case, in the order of `spaceCases()`.
struct SpaceRegistration
{
    const char* name;
    std::vector<SpaceAnswer> answers;
};

template <typename Prepared>
void timePrepared(benchmark::State& state, Prepared prepared)
{
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(prepared);
        benchmark::DoNotOptimize(prepared());
    }
}

/// Registers a benchmark named `<form>/<pair>/<matcher>` for every pair the
/// matcher takes and answers correctly. `Matcher` provides `kName`, `kForm`
/// and `static std::optional<P> prepare(const Pair&)`, where `P` is callable
/// with no arguments and returns the match result.
template <typename Matcher>
Registration registerMatcher(const std::vector<Pair>& pairs)
{
    Registration registration { Matcher::kName, Matcher::kForm, { } };
    for (const Pair& pair : pairs)
    {
        const auto prepared = Matcher::prepare(pair);
        if (!prepared)
        {
            registration.answers.push_back({ pair.id, std::nullopt });
            continue;
        }
        const bool answer = (*prepared)();
        registration.answers.push_back({ pair.id, answer });
        if (answer != pair.matches)
        {
            continue;
        }
        const std::string name = std::string(formName(Matcher::kForm)) + "/" + pair.id + "/" + Matcher::kName;
        benchmark::RegisterBenchmark(name, [prepared = *prepared](benchmark::State& state)
            { timePrepared(state, prepared); });
    }
    return registration;
}

/// Registers a benchmark named `space/<case>/<dispatcher>` for every case the
/// dispatcher takes and reaches the expected number of addresses on.
/// `Dispatcher` provides `kName` and `static std::optional<P>
/// prepare(const SpaceCase&)`, where `P` is callable with no arguments and
/// returns the number of addresses reached.
template <typename Dispatcher>
SpaceRegistration registerDispatcher(const std::vector<SpaceCase>& cases)
{
    SpaceRegistration registration { Dispatcher::kName, { } };
    for (const SpaceCase& spaceCase : cases)
    {
        const auto prepared = Dispatcher::prepare(spaceCase);
        if (!prepared)
        {
            registration.answers.push_back({ spaceCase.id, std::nullopt });
            continue;
        }
        const std::size_t answer = (*prepared)();
        registration.answers.push_back({ spaceCase.id, answer });
        if (answer != spaceCase.numMatches)
        {
            continue;
        }
        const std::string name = std::string("space/") + spaceCase.id + "/" + Dispatcher::kName;
        benchmark::RegisterBenchmark(name, [prepared = *prepared](benchmark::State& state)
            { timePrepared(state, prepared); });
    }
    return registration;
}

}
