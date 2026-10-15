/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"
#include "pairs.h"
#include "registration.h"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <iostream>
#include <vector>

namespace
{

const char* boolName(bool value) noexcept
{
    return value ? "true" : "false";
}

void printNotes(const std::vector<oscpm_compare::Registration>& registrations, const std::vector<oscpm_compare::Pair>& pairs)
{
    for (const oscpm_compare::Registration& registration : registrations)
    {
        for (std::size_t i = 0; i < pairs.size(); ++i)
        {
            const oscpm_compare::Answer& answer = registration.answers[i];
            if (!answer.matches)
            {
                std::cerr << "note: " << registration.name << " does not take " << pairs[i].id << '\n';
            }
            else if (*answer.matches != pairs[i].matches)
            {
                std::cerr << "note: " << registration.name << " answers " << boolName(*answer.matches) << " for " << pairs[i].id << ", expected " << boolName(pairs[i].matches) << "; not timed\n";
            }
        }
    }
}

}

int main(int argc, char** argv)
{
    const std::vector<oscpm_compare::Pair>& pairs = oscpm_compare::pairs();
    std::vector<oscpm_compare::Registration> registrations;
    registrations.push_back(oscpm_compare::registerOscpmMatch(pairs));
    registrations.push_back(oscpm_compare::registerLiblo(pairs));
    registrations.push_back(oscpm_compare::registerLibo(pairs));
    registrations.push_back(oscpm_compare::registerRtosc(pairs));
    registrations.push_back(oscpm_compare::registerOscpmPrepared(pairs));
#if OSCPM_BENCH_JUCE
    registrations.push_back(oscpm_compare::registerJuce(pairs));
#endif

    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }
    std::cerr << "compiler: " << OSCPM_COMPARE_COMPILER << '\n';
    printNotes(registrations, pairs);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}
