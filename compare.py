#!/usr/bin/env python3
# Part of oscpm-bench
#
# SPDX-FileCopyrightText: 2026 Jamie Bullock
# SPDX-License-Identifier: Zlib

"""Times two oscpm_compare binaries in interleaved rounds and prints the ratio per benchmark as Markdown."""

import argparse
import pathlib
import statistics
import tempfile

from report import medians, runBenchmarks

OSCPM_ROWS = "raw/.*/oscpm::match|prepared/.*/Pattern::matches|space/.*/AddressSpace"


def timeOnce(binary, filterPattern, repetitions):
    with tempfile.TemporaryDirectory() as directory:
        resultsPath = pathlib.Path(directory) / "results.json"
        runBenchmarks(binary, resultsPath, repetitions, filterPattern)
        return medians(resultsPath)


def formatTime(nanoseconds):
    if nanoseconds >= 1000:
        return f"{nanoseconds:,.0f}"
    return f"{nanoseconds:.3g}"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("base", type=pathlib.Path, help="the oscpm_compare binary to compare against")
    parser.add_argument("head", type=pathlib.Path, help="the oscpm_compare binary under test")
    parser.add_argument("--filter", default=OSCPM_ROWS, help="regular expression selecting benchmarks (default: the oscpm rows)")
    parser.add_argument("--rounds", type=int, default=5, help="rounds, each running both binaries (default: 5)")
    parser.add_argument("--repetitions", type=int, default=5, help="repetitions per benchmark per round (default: 5)")
    arguments = parser.parse_args()

    results = {"base": [], "head": []}
    for index in range(arguments.rounds):
        order = ("base", "head") if index % 2 == 0 else ("head", "base")
        for side in order:
            results[side].append(timeOnce(getattr(arguments, side), arguments.filter, arguments.repetitions))
    names = sorted(set(results["base"][0]) & set(results["head"][0]))
    print(f"| Benchmark | base | head | head / base, median of {arguments.rounds} rounds |")
    print("|---|---:|---:|---:|")
    for name in names:
        base = statistics.median(r[name] for r in results["base"])
        head = statistics.median(r[name] for r in results["head"])
        ratio = statistics.median(h[name] / b[name] for b, h in zip(results["base"], results["head"]))
        print(f"| {name} | {formatTime(base)} | {formatTime(head)} | {ratio:.2f} |")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
