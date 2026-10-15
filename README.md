# oscpm-bench

Benchmarks of [oscpm](https://github.com/jamiebullock/oscpm) against other OSC address pattern matchers. The table in oscpm's README is produced here.

## What is compared

Each matcher answers the same question, whether an OSC address pattern matches an OSC address, on the same pairs:

| Matcher | Call | Form |
| --- | --- | --- |
| oscpm | `oscpm::match(pattern, address)` | two strings in, one bool out |
| liblo | `lo_pattern_match(address, pattern)` | two strings in, one bool out |
| CNMAT libo | `osc_match(pattern, address, ...)` | two strings in, one bool out |
| rtosc | `rtosc_match(pattern, address, nullptr)` | two strings in, one bool out |
| oscpm | `Pattern::matches(address)` | `Pattern` and `Address` parsed before the timed loop |
| JUCE | `OSCAddressPattern::matches(address)` | `OSCAddressPattern` and `OSCAddress` constructed before the timed loop |

The two forms are reported in separate tables. Within the first, only `oscpm::match` validates its inputs; the C matchers take the strings as they are. Within the second, both calls work on objects their library has already validated.

The cases are one per pattern construct of OSC 1.0 (`?`, `*`, `[]`, `{}`), the `//` operator of OSC 1.1, a literal, a combined pattern with a hit and a miss, and a pattern of repeated `*` against a long run of one byte, which is the shape that makes a backtracking matcher work hardest. They are listed in `src/pairs.cpp`.

## Fairness

Before anything is timed, every matcher is run once on every case. A matcher that cannot take a case, or whose answer differs from the expected one, is left blank in that row and the reason is printed. The expected answers are those of the OSC 1.0 specification and the OSC 1.1 `//` operator.

Known gaps: rtosc's matcher takes `?` and `[` as literal bytes and `*` only at the end of a path segment, so it is timed only on the cases its syntax covers; JUCE, libo and rtosc have no `//`.

Every matcher is timed through the same loop, with Google Benchmark's `DoNotOptimize` on the inputs and the result each iteration.

## Running

```
python3 report.py
```

This configures and builds the `release` preset, runs every benchmark ten times and prints the setup and the tables as Markdown. `--skip-build` reuses the binary, `--repetitions` changes the count, and `--preset release-no-juce` leaves JUCE out, which saves the largest fetch.

The binary itself is `build/release/oscpm_compare` and takes Google Benchmark's usual flags.

Timings are only taken on a quiet machine. CI builds the comparison and runs each case once to catch a comparator that no longer builds or no longer agrees; it does not time anything.

## Dependencies and licences

`DEPENDENCIES.md` lists every fetched project, its pinned revision and its licence. The comparators are fetched at configure time and built from their matching source files; none is copied into this repository. This repository's own code is under the zlib licence in `LICENSE`.
