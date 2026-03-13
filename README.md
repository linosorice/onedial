<p align="center">
  <img src="resources/icon/onedial_icon.png" alt="OneDial" width="128">
</p>

<h1 align="center">OneDial</h1>

<p align="center">
  <strong>One knob. Five amps. Zero friction.</strong><br>
  A neural amp modeler plugin for macOS with a single rotary control.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/macOS-12%2B-black?style=flat-square&logo=apple" alt="macOS 12+">
  <img src="https://img.shields.io/badge/format-Audio%20Unit-blue?style=flat-square" alt="Audio Unit">
  <img src="https://img.shields.io/badge/C%2B%2B-17-orange?style=flat-square&logo=cplusplus" alt="C++17">
  <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="MIT License">
</p>

---

## The Idea

Most amp sim plugins drown you in options. OneDial takes the opposite approach: **a single knob that sweeps from crystal clean to searing lead**. Turn left for sparkle, turn right for fire. That's it.

Under the hood, five high-quality [NAM](https://github.com/sdatkinson/NeuralAmpModelerCore) neural network models run in parallel pairs, crossfading seamlessly as you turn the dial. The result is a continuous spectrum of tone — not five presets with hard boundaries.

## Tone Map

| Dial | Zone | Character |
|:---:|:---:|:---|
| `0 – 2` | **CLEAN** | Fender Deluxe Reverb — glassy, bell-like cleans |
| `2 – 4` | **WARM** | Vox AC30 Top Boost — chimey breakup |
| `4 – 6` | **CRUNCH** | Marshall 1959SLP Plexi — classic British grind |
| `6 – 8` | **DRIVE** | Marshall JCM800 — tight, aggressive midrange |
| `8 – 10` | **LEAD** | Mesa Boogie Rectifier — thick, saturated sustain |

Every position between zones blends the two nearest amp models in real-time. Position 3.7? That's 65% AC30, 35% Plexi. Smooth, musical, no clicks.

## Signal Chain

```
Guitar In → Noise Gate → [NAM Zone A ⟷ NAM Zone B] → IR Cabinet → Volume → Out
                              ↑ crossfade blend ↑
```

## Features

- **Single-knob workflow** — dial in your tone in seconds
- **Neural amp modeling** — powered by NeuralAmpModelerCore
- **Seamless zone blending** — parallel processing with continuous crossfade
- **Matched cabinet IRs** — each zone has its own impulse response
- **Built-in noise gate** — keeps things quiet between riffs
- **Vintage analog UI** — walnut & brass aesthetic with VU meter, because tone starts with the eyes
- **Zero configuration** — all models are bundled, no file management needed

## Build

> Requires macOS 12+, CMake 3.22+, and a C++17 compiler.

```bash
# Configure & build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The AU plugin is automatically installed to `~/Library/Audio/Plug-Ins/Components/`.

**Logic Pro users** — to load unsigned Audio Units:

```bash
defaults write com.apple.Logic10 DoNotValidateAudioUnits -bool YES
```

## Customizing Profiles

Swap in your own NAM captures and cabinet IRs:

```
resources/
├── profiles/           # .nam neural amp models
│   ├── clean.nam
│   ├── warm.nam
│   ├── crunch.nam
│   ├── drive.nam
│   └── lead.nam
└── cabinets/           # .wav impulse responses
    ├── clean.wav
    ├── warm.wav
    ├── crunch.wav
    ├── drive.wav
    └── lead.wav
```

Drop your files, rebuild, and you've got a completely different amp collection — still controlled by one knob.

## Tech Stack

| | |
|---|---|
| **Framework** | [JUCE 8](https://juce.com/) |
| **Neural Engine** | [NeuralAmpModelerCore](https://github.com/sdatkinson/NeuralAmpModelerCore) |
| **Math** | [Eigen](https://eigen.tuxfamily.org/) |
| **Build** | CMake + [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) |

## License

MIT
