# DOOM port for PolkaVM/CoreVM

This is a DOOM port for [PolkaVM](https://github.com/koute/polkavm).

It's based on [doomgeneric](https://github.com/ozkl/doomgeneric), but it's even more portable.
The source code is completely standalone and doesn't even require a libc. It also has out-of-box
support for audio due to built-in OPL emulation, which doomgeneric doesn't have.

## Building

1. Install polkaports.
2. Run `source ./activate.sh corevm` from polkadot.
3. Init file `output/config.mk` (can be empty).
4. Run `make output/doom1_wad.c` to init linked wad blob.
5. Run `make -j` to build.

## License

GPLv2+

Many third-party libraries are used here with varying licenses; see the `libs` directory for details.
