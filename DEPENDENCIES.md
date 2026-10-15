# Dependencies

Everything is fetched from source at configure time with CMake's FetchContent,
pinned to a full commit hash. Nothing is vendored. The comparators are built
from their matching source files only; their own build systems are not run.

| Name | Revision | Licence | Used for |
| --- | --- | --- | --- |
| [oscpm](https://github.com/jamiebullock/oscpm) | v0.9.8 (`ff146c10ab1db38a9745a4ac98219ae51f4ff3ec`) | Zlib | The library under comparison |
| [Google Benchmark](https://github.com/google/benchmark) | v1.9.5 (`192ef10025eb2c4cdd392bc502f0c852196baa48`) | Apache-2.0 | Timing |
| [liblo](https://github.com/radarsat1/liblo) | 0.36 (`025f277275e6b81032a72dfb0b131adab80363e6`) | LGPL-2.1-or-later | `lo_pattern_match` from `src/pattern_match.c` |
| [libo](https://github.com/CNMAT/libo) | master, 2023-07-16 (`874edd2ab76a1931a2d8efb81196c07aa346f650`) | University of California Regents licence | `osc_match` from `osc_match.c` |
| [rtosc](https://github.com/fundamental/rtosc) | v0.3.1 (`f3882fedab283e028514d40415ab64ea48d4b3a1`) | MIT | `rtosc_match` from `src/dispatch.c`, `src/rtosc.c` and `src/arg-val-math.c` |
| [JUCE](https://github.com/juce-framework/JUCE) | 9.0.4 (`3fb5cbc0340758596dac39f7c8de47d562b4896e`) | AGPL-3.0 or commercial | `OSCAddressPattern::matches` from the `juce_osc` module; `OSCPM_BENCH_JUCE` |

The pinned revisions are declared in `CMakeLists.txt`.
