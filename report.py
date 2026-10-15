#!/usr/bin/env python3
# Part of oscpm-bench
#
# SPDX-FileCopyrightText: 2026 Jamie Bullock
# SPDX-License-Identifier: Zlib

"""Builds the comparison, runs it and prints the result as Markdown."""

import argparse
import datetime
import json
import pathlib
import platform
import re
import statistics
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent

RAW_COLUMNS = ["oscpm::match", "liblo", "libo", "rtosc"]
PREPARED_COLUMNS = ["Pattern::matches", "JUCE"]


def run(command, **kwargs):
    return subprocess.run(command, check=True, text=True, **kwargs)


def output(command):
    try:
        return run(command, capture_output=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def build(preset):
    run(["cmake", "--preset", preset], cwd=ROOT)
    run(["cmake", "--build", "--preset", preset], cwd=ROOT)


def runBenchmarks(binary, resultsPath, repetitions):
    completed = run(
        [
            str(binary),
            f"--benchmark_out={resultsPath}",
            "--benchmark_out_format=json",
            f"--benchmark_repetitions={repetitions}",
            "--benchmark_report_aggregates_only=true",
        ],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE,
    )
    notes = []
    compiler = ""
    for line in completed.stderr.splitlines():
        if line.startswith("note: "):
            notes.append(line[len("note: "):])
        elif line.startswith("compiler: "):
            compiler = line[len("compiler: "):]
    return notes, compiler


def medians(resultsPath):
    with open(resultsPath, encoding="utf-8") as handle:
        results = json.load(handle)
    byName = {}
    for entry in results["benchmarks"]:
        if entry.get("aggregate_name") != "median":
            continue
        name = entry["run_name"]
        nanoseconds = entry["real_time"]
        if entry.get("time_unit", "ns") != "ns":
            raise SystemExit(f"{name} is not reported in nanoseconds")
        byName[name] = nanoseconds
    return byName


def pinnedRevisions():
    text = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    pattern = re.compile(r"FetchContent_Declare\(\s*(\w+)\s.*?GIT_TAG\s+([0-9a-f]{40})\s*#\s*([^\n]+)", re.S)
    return [(name, sha[:7], tag.strip()) for name, sha, tag in pattern.findall(text)]


def processorName():
    if platform.system() == "Darwin":
        return output(["sysctl", "-n", "machdep.cpu.brand_string"])
    try:
        with open("/proc/cpuinfo", encoding="utf-8") as handle:
            for line in handle:
                if line.startswith("model name"):
                    return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return platform.processor() or platform.machine()


def operatingSystem():
    if platform.system() == "Darwin":
        return f"macOS {platform.mac_ver()[0]}"
    return f"{platform.system()} {platform.release()}"


def cachedFlags(buildDir):
    cache = (buildDir / "CMakeCache.txt").read_text(encoding="utf-8")
    match = re.search(r"^CMAKE_CXX_FLAGS_RELEASE:STRING=(.*)$", cache, re.M)
    return match.group(1).strip() if match else ""


def oscpmSourceDir(buildDir):
    cache = (buildDir / "CMakeCache.txt").read_text(encoding="utf-8")
    match = re.search(r"^FETCHCONTENT_SOURCE_DIR_OSCPM:PATH=(.+)$", cache, re.M)
    if match:
        return pathlib.Path(match.group(1).strip())
    return buildDir / "_deps" / "oscpm-src"


def oscpmVersion(buildDir):
    described = output(["git", "-C", str(oscpmSourceDir(buildDir)), "describe", "--tags", "--always"])
    return described or "unknown"


def formatTime(nanoseconds):
    if nanoseconds >= 100:
        return f"{nanoseconds:,.0f}"
    if nanoseconds >= 10:
        return f"{nanoseconds:.1f}"
    return f"{nanoseconds:.2f}"


def cell(times, form, pairId, column, baseline):
    time = times.get(f"{form}/{pairId}/{column}")
    if time is None:
        return "-"
    if baseline is None or column == baseline[0]:
        return formatTime(time)
    return f"{formatTime(time)} ({time / baseline[1]:.1f}x)"


def table(times, form, pairIds, columns):
    lines = ["| Case | " + " | ".join(columns) + " |", "|---|" + "---:|" * len(columns)]
    for pairId in pairIds:
        baseTime = times.get(f"{form}/{pairId}/{columns[0]}")
        baseline = (columns[0], baseTime) if baseTime else None
        cells = [cell(times, form, pairId, column, baseline) for column in columns]
        lines.append(f"| {pairId} | " + " | ".join(cells) + " |")
    return lines


def pairIdsInOrder(times):
    seen = []
    for name in times:
        pairId = name.split("/")[1]
        if pairId not in seen:
            seen.append(pairId)
    return seen


def report(times, notes, compiler, buildDir, repetitions):
    revisions = pinnedRevisions()
    lines = [
        f"- Processor: {processorName()}",
        f"- Operating system: {operatingSystem()}",
        f"- Compiler: {compiler}, `{cachedFlags(buildDir)}`",
        f"- oscpm: {oscpmVersion(buildDir)}, measured {datetime.date.today().isoformat()}",
        "- Comparators: " + ", ".join(f"{name} {tag} ({sha})" for name, sha, tag in revisions if name not in ("oscpm", "benchmark")),
        f"- Method: median wall-clock time in nanoseconds of {repetitions} repetitions; a ratio is the comparator's time over oscpm's",
        "",
        "**Two strings in, one bool out**",
        "",
    ]
    pairIds = pairIdsInOrder(times)
    lines += table(times, "raw", pairIds, RAW_COLUMNS)
    lines += ["", "**Parsed before the timed loop**", ""]
    lines += table(times, "prepared", pairIds, PREPARED_COLUMNS)
    if notes:
        lines += ["", "Cases a matcher does not take, or answers differently, are left blank:", ""]
        lines += [f"- {note}" for note in notes]
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preset", default="release", help="the configure and build preset (default: release)")
    parser.add_argument("--repetitions", type=int, default=10, help="repetitions per benchmark (default: 10)")
    parser.add_argument("--skip-build", action="store_true", help="run the binary already in the build directory")
    arguments = parser.parse_args()

    buildDir = ROOT / "build" / arguments.preset
    if not arguments.skip_build:
        build(arguments.preset)
    binary = buildDir / "oscpm_compare"
    resultsPath = buildDir / "results.json"
    notes, compiler = runBenchmarks(binary, resultsPath, arguments.repetitions)
    print(report(medians(resultsPath), notes, compiler, buildDir, arguments.repetitions))
    return 0


if __name__ == "__main__":
    sys.exit(main())
