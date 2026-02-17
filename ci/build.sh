#!/bin/sh

main() {
	set -ex
	workdir="$(mktemp -d)"
	trap cleanup EXIT
	root="$PWD"
	polkaports_install
	doom_build
}

doom_build() {
	cd "$root"
	git config --global --add safe.directory "$PWD"
	./configure
	make -j
	./configure --no-audio
	make -j
}

polkaports_install() {
	sudo -n apt-get -qq update
	sudo -n apt-get -qq install -y clang-19 lld-19 llvm-19 autotools-dev
	rustup component add rust-src
	git clone --recurse-submodules https://github.com/paritytech/polkaports "$workdir"/polkaports
	cd "$workdir"/polkaports
	env CC=clang-19 \
		LD=clang-19 \
		LLD=lld-19 \
		AR=llvm-ar-19 \
		AS=llvm-as-19 \
		NM=llvm-nm-19 \
		STRIP=llvm-strip-19 \
		OBJCOPY=llvm-objcopy-19 \
		OBJDUMP=llvm-objdump-19 \
		RANLIB=llvm-ranlib-19 \
		./setup.sh
	suffix=corevm
	. ./activate.sh corevm
}

cleanup() {
	rm -rf "$workdir"
}

main
