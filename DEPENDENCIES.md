# Dependencies

Everything is fetched from source at configure time with CMake's FetchContent,
pinned to a full commit hash or to a release archive with a checksum. Nothing
is vendored. liblo is built by its own CMake project; the other comparators
are built from their matching source files only.

| Name | Revision | Licence | Used for |
| --- | --- | --- | --- |
| [oscpm](https://github.com/jamiebullock/oscpm) | v0.10.1 (`70682d11e4f7e3b8f417608a4c8871f0ac380597`) | Zlib | The library under comparison |
| [Google Benchmark](https://github.com/google/benchmark) | v1.9.5 (`192ef10025eb2c4cdd392bc502f0c852196baa48`) | Apache-2.0 | Timing |
| [liblo](https://github.com/radarsat1/liblo) | 0.36 (`025f277275e6b81032a72dfb0b131adab80363e6`) | LGPL-2.1-or-later | `lo_pattern_match` and `lo_server`, as the static library its `cmake/` project builds |
| [libo](https://github.com/CNMAT/libo) | master, 2023-07-16 (`874edd2ab76a1931a2d8efb81196c07aa346f650`) | University of California Regents licence | `osc_match` from `osc_match.c` |
| [OSC for Arduino](https://github.com/CNMAT/OSC) | 4.0.0 (`d0a965d42b1d9bb0b7dad3cee7ec87c24a3602e2`) | University of California Regents licence | `osc_match` from `OSCMatch.c`, compiled under the name `cnmat_osc_match` |
| [rtosc](https://github.com/fundamental/rtosc) | v0.3.1 (`f3882fedab283e028514d40415ab64ea48d4b3a1`) | MIT | `rtosc_match` and `rtosc::Ports` from `src/` and `src/cpp/` |
| [oscpkt](http://gruntthepeon.free.fr/oscpkt/) | 1.2 (`oscpkt-1.2.tar.gz`, SHA-256 `7adf8b827eec90c9806adbd4a880b0573fe355e813e618e0041ac8bb2a29e6fa`) | Zlib | `fullPatternMatch` from `oscpkt.hh` |
| [JUCE](https://github.com/juce-framework/JUCE) | 9.0.4 (`3fb5cbc0340758596dac39f7c8de47d562b4896e`) | AGPL-3.0 or commercial | `OSCAddressPattern::matches` from the `juce_osc` module; `OSCPM_BENCH_JUCE` |

The pinned revisions are declared in `CMakeLists.txt`.
