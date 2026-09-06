# Synth303 — Roland TB-303 Baseline Clone

A real-time software emulation of the Roland TB-303 Bass Line synthesizer, built as a JUCE audio plugin (VST3 / AU / Standalone).

By Giang, María, and Moritz (Modern Real-Time Audio course project).

References: [TB-303](https://en.wikipedia.org/wiki/Roland_TB-303) · [Signal flow](https://www.tinyloops.com/tb303/sound_signalflow.html)

## Overview

The Roland TB-303 defined the acid house sound with its squelchy, resonant filter sweeps. Synth303 recreates its core in software: a monophonic voice driving an anti-aliased oscillator through a resonant transistor-ladder filter, shaped by fast decay envelopes with accent.

It's built on JUCE and a shared MRTA (Modern Real-Time Audio) utility layer that handles parameter management and GUI generation, and builds to standard plugin formats for use in any modern DAW or as a standalone app.

## Features

* Anti-aliased oscillator — DPW (Differentiated Parabolic Waveform) generation with Saw and Square waveforms
* Transistor ladder filter (VCF) with envelope-modulated cutoff
* Dual envelopes — a fast amplitude (VCA) envelope and a decay-driven filter envelope
* Accent and glide — velocity accent depth and parameter interpolation for authentic slides
* Cross-format — VST3, AU (macOS), or Standalone from a single CMake configuration

## Architecture

```text
              ┌──────────────────────────┐
              │   TB303Processor (JUCE)  │  audio/MIDI buffer loop
              │   TB303Editor (GUI)      │  parameter UI
              └────────────┬─────────────┘
                           │  parameters
                           ▼
              ┌──────────────────────────┐
              │       TB303 Voice        │  voice management & routing
              └────────────┬─────────────┘
                           │
        ┌──────────┬───────┴───────┬──────────────┐
        ▼          ▼               ▼              ▼
   ┌─────────┐ ┌─────────┐   ┌──────────┐  ┌──────────────┐
   │   VCO   │ │   VCF   │   │   VCA    │  │  Envelope    │
   │Oscillator│→│ Ladder │ → │  (gain)  │← │  Generator   │
   └─────────┘ │ Filter  │   └──────────┘  └──────────────┘
               └─────────┘
```

The DSP core is pure, framework-agnostic C++. The Synth303 layer wraps it in JUCE's processor/editor model, and the MRTA utilities provide parameter storage, a thread-safe parameter FIFO, and a generic parameter editor.

### Tech stack

| Layer        | Technology                        |
| ------------ | --------------------------------- |
| DSP core     | C++ (JUCE-independent)            |
| Plugin/host  | JUCE (VST3 / AU / Standalone)     |
| Build system | CMake 3.25+                       |
| Platforms    | Linux, macOS (universal), Windows |

## Controls

| Control       | Hardware Term   | Range Mapping                     |
| ------------- | --------------- | --------------------------------- |
| Tuning        | Tuning          | `[-12.0, 12.0]` semitone offset   |
| Cut-off Freq  | Cut-off freq    | `[20 Hz, 20 kHz]` exponential     |
| Resonance     | Resonance       | `[0.0, 1.0]` → internal Q `[0.5, 4.0]` |
| Envelope Mod. | Env Mod         | `[-1.0, 1.0]` reversible depth    |
| Decay         | Decay           | `[1 ms, 1000 ms]` decay timeline  |
| Accent        | Accent          | `[0.0, 1.0]` velocity modifier    |
| Volume        | Volume          | `[-60 dB, +12 dB]` master gain    |
| Waveform      | Waveform Switch | `Saw` / `Square`                  |

## Technical notes

**Anti-aliased oscillator (DPW).** Naïve saw/square generation aliases badly at high pitches. The oscillator uses the Differentiated Parabolic Waveform method — synthesizing a smooth polynomial and differentiating it — to suppress aliased partials cheaply enough to run per-sample in real time.

**Envelope-driven filter.** The acid character comes from the interaction between the resonant ladder filter and a fast, decay-only filter envelope, scaled by the Env Mod and Accent controls. The decay curve and modulation depth — not the filter topology alone — are what make it recognizably a 303.

## Getting started

Prerequisites: a C++ toolchain (Clang / GCC / MSVC), CMake 3.25+, and JUCE's platform dependencies (see the JUCE docs for Linux system packages).

Configure — creates `build/` and fetches JUCE:

```bash
./configure.sh
```

Build — `build.sh <target> <format> <config> <jobs>`:

```bash
./build.sh synth303 Standalone Release
```

* format — `VST3`, `AU` (macOS only), or `Standalone` (default)
* config — `Release` or `Debug` (default)
* jobs — concurrent build jobs (default `4`)

The built plugin/app is placed under `build/`.

## Project structure

```text
aalto303/
├── CMakeLists.txt              # Build system configuration
├── configure.sh               # Generate build/ and fetch JUCE
├── build.sh                   # Build a target/format/config
├── cmake/add_plugin.cmake     # JUCE target builder macros
├── mrta_utils/                # Shared framework abstractions
│   ├── Processor/             # BaseProcessor
│   ├── Parameter/             # Parameter manager, FIFO, info
│   └── GUI/                   # Generic parameter editor, knobs
└── projects/
    ├── DSP/                   # Pure synthesis core engine
    │   ├── Oscillator.*       # Anti-aliased DPW waveform generator
    │   ├── LadderFilter.*     # Transistor-ladder emulation
    │   ├── EnvelopeGenerator.*
    │   ├── Ramp.h             # Parameter/glide interpolation
    │   └── TB303.*            # Voice & sound definitions
    └── Synth303/              # JUCE processor & GUI layer
        ├── TB303Processor.*   # Audio/MIDI buffer processing
        └── TB303Editor.*      # Plugin UI
```

`projects/example/` contains reference JUCE examples and `snipets/` holds standalone C++ concept demos; neither is part of the plugin build.

## License

Licensed under the GNU General Public License v3.0 — see [LICENSE](LICENSE).
