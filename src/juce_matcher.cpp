/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <juce_osc/juce_osc.h>

#include <optional>

namespace oscpm_compare
{

namespace
{
    struct Juce
    {
        static constexpr const char* kName = "JUCE";
        static constexpr Form kForm = Form::Prepared;

        struct Prepared
        {
            juce::OSCAddressPattern pattern;
            juce::OSCAddress address;

            bool operator()() const noexcept
            {
                return pattern.matches(address);
            }
        };

        static std::optional<Prepared> prepare(const Pair& pair)
        {
            try
            {
                return Prepared { juce::OSCAddressPattern(pair.pattern), juce::OSCAddress(pair.address) };
            }
            catch (const juce::OSCFormatError&)
            {
                return std::nullopt;
            }
        }
    };
}

Registration registerJuce(const std::vector<Pair>& pairs)
{
    return registerMatcher<Juce>(pairs);
}

}
