/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "space.h"

#include <array>
#include <string_view>

namespace oscpm_compare
{

namespace
{
    namespace k
    {
        constexpr std::size_t numSynths = 8;
        constexpr std::size_t numVoices = 16;
        constexpr std::size_t numChannels = 16;
        constexpr std::array<std::string_view, 4> oscillators = { "saw", "square", "sine", "tri" };
        constexpr std::array<std::string_view, 3> oscillatorParameters = { "freq", "gain", "detune" };
        constexpr std::array<std::string_view, 2> filterParameters = { "cutoff", "res" };
        constexpr std::array<std::string_view, 2> synthParameters = { "gain", "pan" };
        constexpr std::array<std::string_view, 3> channelParameters = { "gain", "pan", "mute" };
    }

    std::vector<std::string> generateAddresses()
    {
        std::vector<std::string> addresses;
        for (std::size_t synth = 1; synth <= k::numSynths; ++synth)
        {
            const std::string synthPrefix = "/synth" + std::to_string(synth);
            for (const std::string_view parameter : k::synthParameters)
            {
                addresses.push_back(synthPrefix + "/" + std::string(parameter));
            }
            for (std::size_t voice = 1; voice <= k::numVoices; ++voice)
            {
                const std::string voicePrefix = synthPrefix + "/voice/" + std::to_string(voice);
                for (const std::string_view oscillator : k::oscillators)
                {
                    for (const std::string_view parameter : k::oscillatorParameters)
                    {
                        addresses.push_back(voicePrefix + "/osc/" + std::string(oscillator) + "/" + std::string(parameter));
                    }
                }
                for (const std::string_view parameter : k::filterParameters)
                {
                    addresses.push_back(voicePrefix + "/filter/" + std::string(parameter));
                }
            }
        }
        for (std::size_t channel = 1; channel <= k::numChannels; ++channel)
        {
            for (const std::string_view parameter : k::channelParameters)
            {
                addresses.push_back("/mixer/ch" + std::to_string(channel) + "/" + std::string(parameter));
            }
        }
        return addresses;
    }
}

const std::vector<std::string>& spaceAddresses()
{
    static const std::vector<std::string> all = generateAddresses();
    return all;
}

const std::vector<SpaceCase>& spaceCases()
{
    static const std::vector<SpaceCase> all = {
        { "literal", "/synth3/voice/7/osc/saw/freq", 1, false },
        { "pattern", "/synth[3-6]/voice/*/osc/{saw,square}/freq", 128, false },
        { "pattern-repeated", "/synth[3-6]/voice/*/osc/{saw,square}/freq", 128, true },
    };
    return all;
}

}
