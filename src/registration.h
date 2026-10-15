/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#pragma once

#include "pairs.h"

#include <benchmark/benchmark.h>

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

}
