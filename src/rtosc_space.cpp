/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#include "matchers.h"

#include <rtosc/ports.h>
#include <rtosc/rtosc.h>

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>

namespace oscpm_compare
{

namespace
{
    namespace k
    {
        constexpr std::size_t messageCapacity = 256;
        constexpr std::size_t locationCapacity = 1024;
    }

    void countHit(const char*, rtosc::RtData& data)
    {
        ++*static_cast<std::size_t*>(data.obj);
    }

    void enterSubtree(const rtosc::Ports& subtree, const char* message, rtosc::RtData& data)
    {
        while (*message && *message != '/')
        {
            ++message;
        }
        if (*message)
        {
            ++message;
        }
        subtree.dispatch(message, data);
    }

    rtosc::Port leaf(const char* name)
    {
        return { name, nullptr, nullptr, countHit };
    }

    rtosc::Port branch(const char* name, const rtosc::Ports& subtree)
    {
        return { name, nullptr, &subtree, [&subtree](const char* message, rtosc::RtData& data)
            { enterSubtree(subtree, message, data); } };
    }

    /// The space as an rtosc port tree. A `#N` port takes indices below N, so
    /// N is one above the highest index registered.
    struct Tree
    {
        rtosc::Ports oscillatorParameters { leaf("freq"), leaf("gain"), leaf("detune") };
        rtosc::Ports oscillators { branch("saw/", oscillatorParameters), branch("square/", oscillatorParameters), branch("sine/", oscillatorParameters), branch("tri/", oscillatorParameters) };
        rtosc::Ports filterParameters { leaf("cutoff"), leaf("res") };
        rtosc::Ports voice { branch("osc/", oscillators), branch("filter/", filterParameters) };
        rtosc::Ports voiceIndex { branch("#17/", voice) };
        rtosc::Ports synth { leaf("gain"), leaf("pan"), branch("voice/", voiceIndex) };
        rtosc::Ports channelParameters { leaf("gain"), leaf("pan"), leaf("mute") };
        rtosc::Ports mixer { branch("ch#17/", channelParameters) };
        rtosc::Ports root { branch("synth#9/", synth), branch("mixer/", mixer) };
    };

    struct Dispatcher
    {
        Tree tree;
        std::array<char, k::messageCapacity> message { };
        std::array<char, k::locationCapacity> location { };
        std::size_t count = 0;
        rtosc::RtData data;

        explicit Dispatcher(const char* path)
        {
            rtosc_message(message.data(), message.size(), path, "");
            data.loc = location.data();
            data.loc_size = location.size();
            data.obj = &count;
        }
    };

    struct RtoscSpace
    {
        static constexpr const char* kName = "rtosc";

        struct Prepared
        {
            std::shared_ptr<Dispatcher> dispatcher;

            std::size_t operator()() const
            {
                dispatcher->count = 0;
                dispatcher->tree.root.dispatch(dispatcher->message.data(), dispatcher->data, true);
                return dispatcher->count;
            }
        };

        static std::optional<Prepared> prepare(const SpaceCase& spaceCase)
        {
            const bool literal = std::string_view(spaceCase.message).find_first_of("*?[{") == std::string_view::npos;
            if (!literal || spaceCase.repeated)
            {
                return std::nullopt;
            }
            return Prepared { std::make_shared<Dispatcher>(spaceCase.message) };
        }
    };
}

SpaceRegistration registerRtoscSpace(const std::vector<SpaceCase>& cases)
{
    return registerDispatcher<RtoscSpace>(cases);
}

}
