# oscpm-bench

Benchmarks of [oscpm](https://github.com/jamiebullock/oscpm) against other OSC address pattern matchers. The tables in oscpm's README are produced here.

## What is compared

Each matcher answers the same question, whether an OSC address pattern matches an OSC address, on the same pairs:

| Matcher | Call | Form |
| --- | --- | --- |
| oscpm | `oscpm::match(pattern, address)` | two strings in, one bool out |
| liblo | `lo_pattern_match(address, pattern)` | two strings in, one bool out |
| CNMAT libo | `osc_match(pattern, address, ...)` | two strings in, one bool out |
| CNMAT OSC for Arduino | `osc_match(pattern, address, ...)` | two strings in, one bool out |
| rtosc | `rtosc_match(pattern, address, nullptr)` | two strings in, one bool out |
| oscpkt | `fullPatternMatch(pattern, address)` | two strings in, one bool out |
| oscpm | `Pattern::matches(address)` | `Pattern` and `Address` parsed before the timed loop |
| JUCE | `OSCAddressPattern::matches(address)` | `OSCAddressPattern` and `OSCAddress` constructed before the timed loop |

The two forms are reported in separate tables. Within the first, only `oscpm::match` validates its inputs; the other matchers take the strings as they are. Within the second, both calls work on objects their library has already validated.

The cases are one per pattern construct of OSC 1.0, the `//` operator of OSC 1.1, a literal, a combined pattern with a hit and a miss, and a pattern of repeated `*` against a long run of one byte, which is the shape that makes a backtracking matcher work hardest. They are defined in `src/pairs.cpp`:

| Case | Pattern | Address |
|---|---|---|
| literal | `/synth3/voice/7/osc/saw/freq` | the same |
| question | `/mixer/ch?/gain` | `/mixer/ch3/gain` |
| star | `/synth2/voice/*/filter/cutoff` | `/synth2/voice/12/filter/cutoff` |
| class | `/synth[3-6]/voice/1/osc/saw/freq` | `/synth4/voice/1/osc/saw/freq` |
| list | `/synth1/voice/1/osc/{saw,square}/freq` | `/synth1/voice/1/osc/square/freq` |
| descendant | `//freq` | `/synth4/voice/12/osc/square/freq` |
| combined | `/synth[3-6]/voice/*/osc/{saw,square}/freq` | `/synth4/voice/12/osc/square/freq` |
| combined-miss | the same | `/synth4/voice/12/osc/square/gain` |
| backtracking | `/*a*a*a*a*a*b` | `/` and 24 `a` |

A third table dispatches a message into a space of 1,856 registered addresses, 8 synths of 16 voices with 4 oscillators and a filter each plus 16 mixer channels, and counts the addresses reached:

| Dispatcher | Registration | Dispatch |
| --- | --- | --- |
| oscpm | `AddressSpace<int>::add` per address | `visit(message, visitor)` |
| liblo | `lo_server_add_method` per address | `lo_server_dispatch_data` on a serialised message with no arguments |
| rtosc | a `Ports` tree with `#N` ports for the synth, voice and channel indices | `Ports::dispatch` on a serialised message with no arguments, with a location buffer |

Its cases dispatch `/synth3/voice/7/osc/saw/freq`, which reaches one address, and `/synth[3-6]/voice/*/osc/{saw,square}/freq`, which reaches 128, once and then a second time, which `AddressSpace` answers from its memo. The addresses and cases are defined in `src/space.cpp`.

## Fairness

Before anything is timed, every matcher is run once on every case. A matcher that cannot take a case, or whose answer differs from the expected one, is left blank in that row and the reason is printed. The expected answers are those of the OSC 1.0 specification and the OSC 1.1 `//` operator.

Known gaps in the pattern tables: rtosc's matcher takes `?` and `[` as literal bytes and `*` only at the end of a path segment, so it is timed only on the cases its syntax covers; JUCE, libo, CNMAT OSC and rtosc have no `//`.

Known differences in the dispatch table:

- liblo deserialises the message and allocates a source address on every dispatch; both are part of what a `lo_server` user pays. Its handlers return 0, so a literal address stops at its first match and the cost depends on where the method sits in the server's list; a pattern is matched against every method
- rtosc dispatches a literal address only; a pattern in a message is compared byte for byte
- Neither comparator has a memo, so the repeated row is oscpm's alone

Every matcher is timed through the same loop, with Google Benchmark's `DoNotOptimize` on the inputs and the result each iteration.

## Not compared

- oscpack has no pattern matching
- osc++ (dimitry-ishenko-cpp/osc) matches addresses with `std::regex` and does not take OSC pattern syntax
- libossia uses an extended syntax with no standalone matching function
- SuperCollider's matching is in sclang, not in a C++ call

## Running

```
python3 report.py
```

This configures and builds the `release` preset, runs every benchmark ten times and prints the setup and the tables as Markdown. `--skip-build` reuses the binary, `--repetitions` changes the count, and `--preset release-no-juce` leaves JUCE out, which saves the largest fetch. `-DFETCHCONTENT_SOURCE_DIR_OSCPM=<path>` on the configure line measures a local oscpm checkout in place of the pinned tag.

The binary itself is `build/release/oscpm_compare` and takes Google Benchmark's usual flags.

Timings are only taken on a quiet machine. CI builds the comparison and runs each case once to catch a comparator that no longer builds or no longer agrees; it does not time anything.

### Comparing two oscpm revisions

Two runs of `report.py` taken one after the other differ by up to a tenth on rows whose code did not change, so a change to oscpm is measured against its base with both binaries in one interleaved run:

```
cmake --preset release -DFETCHCONTENT_SOURCE_DIR_OSCPM=<path to the changed checkout>
cmake --build --preset release
cmake -S . -B build/base -G Ninja -DCMAKE_BUILD_TYPE=Release -DFETCHCONTENT_SOURCE_DIR_OSCPM=<path to the base checkout>
cmake --build build/base
python3 compare.py build/base/oscpm_compare build/release/oscpm_compare
```

Each round runs both binaries, alternating which goes first, and the table gives each benchmark's median time per side and the median of the per-round ratios. The default rows are oscpm's own; `--filter` selects others, and `--rounds` and `--repetitions` change the counts. A second build directory fetches the comparators again unless `-DFETCHCONTENT_SOURCE_DIR_<NAME>=build/release/_deps/<name>-src` points each at the first one's copy.

## Dependencies and licences

`DEPENDENCIES.md` lists every fetched project, its pinned revision and its licence. The comparators are fetched at configure time; none is copied into this repository. This repository's own code is under the zlib licence in `LICENSE`.
