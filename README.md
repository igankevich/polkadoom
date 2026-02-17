# DOOM port for CoreVM

This port uses CoreVM guest API for video/audio/console output.
Based on <https://github.com/koute/polkadoom>.


## Building

You will need latest build tools from [PolkaPorts](https://github.com/paritytech/polkaports).

```bash
# Build with audio support (requires PolkaVM recompiler to run at full speed).
./configure
make -j

# Build without audio (can be run with PolkaVM interpreter).
./configure --no-audio
make -j
```


## Running


```bash
jamt vm new -r roms output/doom.corevm 10000000000
corevm-builder SERVICE_ID
corevm-monitor SERVICE_ID
```


## License

GPLv2+

Many third-party libraries are used here with varying licenses; see the `libs` directory for details.
