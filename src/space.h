/* Part of oscpm-bench
 *
 * SPDX-FileCopyrightText: 2026 Jamie Bullock
 * SPDX-License-Identifier: Zlib
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace oscpm_compare
{

/// The addresses every dispatcher registers: 8 synths of 16 voices with 4
/// oscillators and a filter each, plus 16 mixer channels; 1,856 in all.
const std::vector<std::string>& spaceAddresses();

/// A message dispatched into the space and the number of registered
/// addresses it reaches. `repeated` marks the second dispatch of the same
/// message, which a dispatcher with a memo answers from it.
struct SpaceCase
{
    const char* id;
    const char* message;
    std::size_t numMatches;
    bool repeated;
};

/// Every case the address space comparison times.
const std::vector<SpaceCase>& spaceCases();

}
