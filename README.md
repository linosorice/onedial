# OneDial

Neural Amp Modeler plugin for Logic Pro with a single knob.

Turn left for clean, turn right for high gain. No menus, no file browsers, no technical jargon.

## Zones

| Position | Zone   | Reference Amp          |
|----------|--------|------------------------|
| 0 – 2    | CLEAN  | Fender Deluxe Reverb   |
| 2 – 4    | WARM   | Vox AC30 Top Boost     |
| 4 – 6    | CRUNCH | Marshall 1959SLP Plexi |
| 6 – 8    | DRIVE  | Marshall JCM800        |
| 8 – 10   | LEAD   | Mesa Boogie Rectifier  |

Smooth audio blending between adjacent zones.

## Build

Requirements: macOS 12+, CMake 3.22+, C++17 compiler.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The AU plugin is automatically copied to `~/Library/Audio/Plug-Ins/Components/`.

To use unsigned AUs in Logic Pro, run:
```bash
defaults write com.apple.Logic10 DoNotValidateAudioUnits -bool YES
```

## Adding Profiles

Place `.nam` files in `resources/profiles/` and `.wav` IR files in `resources/cabinets/`, then rebuild. Files are named by zone:

```
resources/profiles/clean.nam
resources/profiles/warm.nam
resources/profiles/crunch.nam
resources/profiles/drive.nam
resources/profiles/lead.nam
resources/cabinets/clean.wav
resources/cabinets/warm.wav
resources/cabinets/crunch.wav
resources/cabinets/drive.wav
resources/cabinets/lead.wav
```

## License

MIT
