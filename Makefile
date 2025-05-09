TARGET_BITNESS = 64
TARGET_ABI32 = ilp32e
TARGET_ABI64 = lp64e
TARGET_ABI = $(TARGET_ABI64)
MUSL_TARGET_ARCH = riscv64
TARGET_FLAGS = --target=riscv$(TARGET_BITNESS)-unknown-none-elf \
			   -march=rv$(TARGET_BITNESS)emac \
			   -mabi=${TARGET_ABI} \
			   -nostdlib \
			   -nodefaultlibs

MUSL_ROOT = libs/musl-1.2.4
DOOM_ROOT = libs/doomgeneric/doomgeneric
SDL_ROOT = libs/SDL
SDL_MIXER_ROOT = libs/SDL-Mixer-X
ADLMIDI_ROOT = libs/libADLMIDI
LIBCXX_ROOT = libs/libcxx
DLMALLOC_ROOT = libs/dlmalloc

outfile_guest = output/doom$(TARGET_BITNESS)-guest.elf
outfile_guest_polkavm = $(outfile_guest:.elf=.polkavm)

sources = src/impl_dummy_libc.c \
		  src/impl_dummy_sdl.c \
		  src/stb_impl.c \
		  $(DLMALLOC_ROOT)/malloc.c \
		  $(LIBCXX_ROOT)/src/new.cpp \
		  $(LIBCXX_ROOT)/src/new_helpers.cpp \
		  $(LIBCXX_ROOT)/src/new_handler.cpp \
		  $(LIBCXX_ROOT)/src/verbose_abort.cpp \
		  $(LIBCXX_ROOT)/src/string.cpp \
		  $(SDL_ROOT)/src/SDL.c \
		  $(SDL_ROOT)/src/SDL_error.c \
		  $(SDL_ROOT)/src/SDL_log.c \
		  $(SDL_ROOT)/src/SDL_dataqueue.c \
		  $(SDL_ROOT)/src/SDL_utils.c \
		  $(SDL_ROOT)/src/audio/SDL_audio.c \
		  $(SDL_ROOT)/src/audio/SDL_audiocvt.c \
		  $(SDL_ROOT)/src/audio/SDL_audiotypecvt.c \
		  $(SDL_ROOT)/src/audio/SDL_mixer.c \
		  $(SDL_ROOT)/src/audio/SDL_wave.c \
		  $(SDL_ROOT)/src/file/SDL_rwops.c \
		  $(SDL_ROOT)/src/thread/SDL_thread.c \
		  $(SDL_ROOT)/src/thread/generic/SDL_syscond.c \
		  $(SDL_ROOT)/src/thread/generic/SDL_sysmutex.c \
		  $(SDL_ROOT)/src/thread/generic/SDL_syssem.c \
		  $(SDL_ROOT)/src/thread/generic/SDL_systhread.c \
		  $(SDL_ROOT)/src/thread/generic/SDL_systls.c \
		  $(ADLMIDI_ROOT)/src/adlmidi.cpp \
		  $(ADLMIDI_ROOT)/src/adlmidi_midiplay.cpp \
		  $(ADLMIDI_ROOT)/src/adlmidi_opl3.cpp \
		  $(ADLMIDI_ROOT)/src/adlmidi_private.cpp \
		  $(ADLMIDI_ROOT)/src/adlmidi_sequencer.cpp \
		  $(ADLMIDI_ROOT)/src/adlmidi_load.cpp \
		  $(ADLMIDI_ROOT)/src/inst_db.cpp \
		  $(ADLMIDI_ROOT)/src/chips/dosbox_opl3.cpp \
		  $(ADLMIDI_ROOT)/src/chips/dosbox/dbopl.cpp \
		  $(ADLMIDI_ROOT)/src/wopl/wopl_file.c \
		  $(SDL_MIXER_ROOT)/src/effect_position.c \
		  $(SDL_MIXER_ROOT)/src/effects_internal.c \
		  $(SDL_MIXER_ROOT)/src/mixer.c \
		  $(SDL_MIXER_ROOT)/src/music.c \
		  $(SDL_MIXER_ROOT)/src/codecs/load_aiff.c \
		  $(SDL_MIXER_ROOT)/src/codecs/load_voc.c \
		  $(SDL_MIXER_ROOT)/src/codecs/mp3utils.c \
		  $(SDL_MIXER_ROOT)/src/codecs/music_midi_adl.c \
		  $(SDL_MIXER_ROOT)/src/utils.c \
		  libs/miniz/miniz.c \
		  libs/miniz/miniz_tdef.c \
		  $(MUSL_ROOT)/src/locale/__lctrans.c \
		  $(MUSL_ROOT)/src/locale/c_locale.c \
		  $(MUSL_ROOT)/src/errno/__errno_location.c \
		  $(MUSL_ROOT)/src/errno/strerror.c \
		  $(MUSL_ROOT)/src/exit/assert.c \
		  $(MUSL_ROOT)/src/string/strspn.c \
		  $(MUSL_ROOT)/src/string/strcspn.c \
		  $(MUSL_ROOT)/src/string/strlen.c \
		  $(MUSL_ROOT)/src/string/strcasecmp.c \
		  $(MUSL_ROOT)/src/string/strchr.c \
		  $(MUSL_ROOT)/src/string/strchrnul.c \
		  $(MUSL_ROOT)/src/string/strcmp.c \
		  $(MUSL_ROOT)/src/string/strdup.c \
		  $(MUSL_ROOT)/src/string/strncasecmp.c \
		  $(MUSL_ROOT)/src/string/strncmp.c \
		  $(MUSL_ROOT)/src/string/strncpy.c \
		  $(MUSL_ROOT)/src/string/strrchr.c \
		  $(MUSL_ROOT)/src/string/strstr.c \
		  $(MUSL_ROOT)/src/string/strnlen.c \
		  $(MUSL_ROOT)/src/string/strlcpy.c \
		  $(MUSL_ROOT)/src/string/stpncpy.c \
		  $(MUSL_ROOT)/src/string/strtok_r.c \
		  $(MUSL_ROOT)/src/string/memset.c \
		  $(MUSL_ROOT)/src/string/memcpy.c \
		  $(MUSL_ROOT)/src/string/memchr.c \
		  $(MUSL_ROOT)/src/string/memrchr.c \
		  $(MUSL_ROOT)/src/string/memcmp.c \
		  $(MUSL_ROOT)/src/string/memmove.c \
		  $(MUSL_ROOT)/src/string/wcschr.c \
		  $(MUSL_ROOT)/src/string/wcslen.c \
		  $(MUSL_ROOT)/src/string/wcsnlen.c \
		  $(MUSL_ROOT)/src/string/wmemchr.c \
		  $(MUSL_ROOT)/src/string/wmemcmp.c \
		  $(MUSL_ROOT)/src/stdio/__overflow.c \
		  $(MUSL_ROOT)/src/stdio/__fdopen.c \
		  $(MUSL_ROOT)/src/stdio/__toread.c \
		  $(MUSL_ROOT)/src/stdio/__towrite.c \
		  $(MUSL_ROOT)/src/stdio/__lockfile.c \
		  $(MUSL_ROOT)/src/stdio/__stdio_close.c \
		  $(MUSL_ROOT)/src/stdio/__stdio_read.c \
		  $(MUSL_ROOT)/src/stdio/__stdio_seek.c \
		  $(MUSL_ROOT)/src/stdio/__stdio_write.c \
		  $(MUSL_ROOT)/src/stdio/__stdio_exit.c \
		  $(MUSL_ROOT)/src/stdio/__stdout_write.c \
		  $(MUSL_ROOT)/src/stdio/__fmodeflags.c \
		  $(MUSL_ROOT)/src/stdio/__uflow.c \
		  $(MUSL_ROOT)/src/stdio/ftrylockfile.c \
		  $(MUSL_ROOT)/src/stdio/ofl.c \
		  $(MUSL_ROOT)/src/stdio/ofl_add.c \
		  $(MUSL_ROOT)/src/stdio/stderr.c \
		  $(MUSL_ROOT)/src/stdio/stdin.c \
		  $(MUSL_ROOT)/src/stdio/stdout.c \
		  $(MUSL_ROOT)/src/stdio/snprintf.c \
		  $(MUSL_ROOT)/src/stdio/vsnprintf.c \
		  $(MUSL_ROOT)/src/stdio/vfprintf.c \
		  $(MUSL_ROOT)/src/stdio/printf.c \
		  $(MUSL_ROOT)/src/stdio/sscanf.c \
		  $(MUSL_ROOT)/src/stdio/ftell.c \
		  $(MUSL_ROOT)/src/stdio/fwrite.c \
		  $(MUSL_ROOT)/src/stdio/fseek.c \
		  $(MUSL_ROOT)/src/stdio/fread.c \
		  $(MUSL_ROOT)/src/stdio/fprintf.c \
		  $(MUSL_ROOT)/src/stdio/fopen.c \
		  $(MUSL_ROOT)/src/stdio/fflush.c \
		  $(MUSL_ROOT)/src/stdio/ferror.c \
		  $(MUSL_ROOT)/src/stdio/fclose.c \
		  $(MUSL_ROOT)/src/stdio/vsscanf.c \
		  $(MUSL_ROOT)/src/stdio/vfscanf.c \
		  $(MUSL_ROOT)/src/stdio/putchar.c \
		  $(MUSL_ROOT)/src/stdio/puts.c \
		  $(MUSL_ROOT)/src/stdio/fputs.c \
		  $(MUSL_ROOT)/src/stdio/remove.c \
		  $(MUSL_ROOT)/src/stdio/rename.c \
		  $(MUSL_ROOT)/src/stdio/feof.c \
		  $(MUSL_ROOT)/src/stdio/fgets.c \
		  $(MUSL_ROOT)/src/stdio/fputc.c \
		  $(MUSL_ROOT)/src/stdio/fputwc.c \
		  $(MUSL_ROOT)/src/stdio/fwide.c \
		  $(MUSL_ROOT)/src/stdio/swprintf.c \
		  $(MUSL_ROOT)/src/stdio/vfwprintf.c \
		  $(MUSL_ROOT)/src/stdio/vswprintf.c \
		  $(MUSL_ROOT)/src/stdlib/abs.c \
		  $(MUSL_ROOT)/src/stdlib/atoi.c \
		  $(MUSL_ROOT)/src/stdlib/atof.c \
		  $(MUSL_ROOT)/src/stdlib/strtod.c \
		  $(MUSL_ROOT)/src/stdlib/strtol.c \
		  $(MUSL_ROOT)/src/stdlib/wcstod.c \
		  $(MUSL_ROOT)/src/stdlib/wcstol.c \
		  $(MUSL_ROOT)/src/stat/mkdir.c \
		  $(MUSL_ROOT)/src/ctype/islower.c \
		  $(MUSL_ROOT)/src/ctype/isupper.c \
		  $(MUSL_ROOT)/src/ctype/isdigit.c \
		  $(MUSL_ROOT)/src/ctype/toupper.c \
		  $(MUSL_ROOT)/src/ctype/tolower.c \
		  $(MUSL_ROOT)/src/ctype/iswspace.c \
		  $(MUSL_ROOT)/src/math/__fpclassifyl.c \
		  $(MUSL_ROOT)/src/math/__signbitl.c \
		  $(MUSL_ROOT)/src/math/frexpl.c \
		  $(MUSL_ROOT)/src/math/fmodl.c \
		  $(MUSL_ROOT)/src/math/scalbn.c \
		  $(MUSL_ROOT)/src/math/scalbnl.c \
		  $(MUSL_ROOT)/src/math/copysignl.c \
		  $(MUSL_ROOT)/src/math/sin.c \
		  $(MUSL_ROOT)/src/math/ceil.c \
		  $(MUSL_ROOT)/src/math/floor.c \
		  $(MUSL_ROOT)/src/math/floorf.c \
		  $(MUSL_ROOT)/src/math/exp.c \
		  $(MUSL_ROOT)/src/math/exp2.c \
		  $(MUSL_ROOT)/src/math/log.c \
		  $(MUSL_ROOT)/src/math/sqrt.c \
		  $(MUSL_ROOT)/src/math/round.c \
		  $(MUSL_ROOT)/src/math/sqrt_data.c \
		  $(MUSL_ROOT)/src/math/exp_data.c \
		  $(MUSL_ROOT)/src/math/log_data.c \
		  $(MUSL_ROOT)/src/math/__cos.c \
		  $(MUSL_ROOT)/src/math/__sin.c \
		  $(MUSL_ROOT)/src/math/__math_divzero.c \
		  $(MUSL_ROOT)/src/math/__math_invalid.c \
		  $(MUSL_ROOT)/src/math/__math_uflow.c \
		  $(MUSL_ROOT)/src/math/__math_oflow.c \
		  $(MUSL_ROOT)/src/math/__math_xflow.c \
		  $(MUSL_ROOT)/src/math/__rem_pio2.c \
		  $(MUSL_ROOT)/src/math/__rem_pio2_large.c \
		  $(MUSL_ROOT)/src/multibyte/mbsinit.c \
		  $(MUSL_ROOT)/src/multibyte/mbrtowc.c \
		  $(MUSL_ROOT)/src/multibyte/wctomb.c \
		  $(MUSL_ROOT)/src/multibyte/wcrtomb.c \
		  $(MUSL_ROOT)/src/multibyte/internal.c \
		  $(MUSL_ROOT)/src/multibyte/btowc.c \
		  $(MUSL_ROOT)/src/multibyte/mbtowc.c \
		  $(MUSL_ROOT)/src/unistd/lseek.c \
		  $(MUSL_ROOT)/src/unistd/close.c \
		  $(MUSL_ROOT)/src/thread/__lock.c \
		  $(MUSL_ROOT)/src/internal/shgetc.c \
		  $(MUSL_ROOT)/src/internal/floatscan.c \
		  $(MUSL_ROOT)/src/internal/intscan.c \
		  $(MUSL_ROOT)/src/internal/libc.c \
		  $(MUSL_ROOT)/src/internal/syscall_ret.c \
		  $(DOOM_ROOT)/i_sdlsound.c \
		  $(DOOM_ROOT)/i_sdlmusic.c \
		  $(DOOM_ROOT)/dummy.c \
		  $(DOOM_ROOT)/am_map.c \
		  $(DOOM_ROOT)/doomdef.c \
		  $(DOOM_ROOT)/doomstat.c \
		  $(DOOM_ROOT)/dstrings.c \
		  $(DOOM_ROOT)/d_event.c \
		  $(DOOM_ROOT)/d_items.c \
		  $(DOOM_ROOT)/d_iwad.c \
		  $(DOOM_ROOT)/d_loop.c \
		  $(DOOM_ROOT)/d_main.c \
		  $(DOOM_ROOT)/d_mode.c \
		  $(DOOM_ROOT)/d_net.c \
		  $(DOOM_ROOT)/f_finale.c \
		  $(DOOM_ROOT)/f_wipe.c \
		  $(DOOM_ROOT)/g_game.c \
		  $(DOOM_ROOT)/hu_lib.c \
		  $(DOOM_ROOT)/hu_stuff.c \
		  $(DOOM_ROOT)/info.c \
		  $(DOOM_ROOT)/i_cdmus.c \
		  $(DOOM_ROOT)/i_endoom.c \
		  $(DOOM_ROOT)/i_joystick.c \
		  $(DOOM_ROOT)/i_scale.c \
		  $(DOOM_ROOT)/i_sound.c \
		  $(DOOM_ROOT)/i_system.c \
		  $(DOOM_ROOT)/i_timer.c \
		  $(DOOM_ROOT)/memio.c \
		  $(DOOM_ROOT)/m_argv.c \
		  $(DOOM_ROOT)/m_bbox.c \
		  $(DOOM_ROOT)/m_cheat.c \
		  $(DOOM_ROOT)/m_config.c \
		  $(DOOM_ROOT)/m_controls.c \
		  $(DOOM_ROOT)/m_fixed.c \
		  $(DOOM_ROOT)/m_menu.c \
		  $(DOOM_ROOT)/m_misc.c \
		  $(DOOM_ROOT)/m_random.c \
		  $(DOOM_ROOT)/p_ceilng.c \
		  $(DOOM_ROOT)/p_doors.c \
		  $(DOOM_ROOT)/p_enemy.c \
		  $(DOOM_ROOT)/p_floor.c \
		  $(DOOM_ROOT)/p_inter.c \
		  $(DOOM_ROOT)/p_lights.c \
		  $(DOOM_ROOT)/p_map.c \
		  $(DOOM_ROOT)/p_maputl.c \
		  $(DOOM_ROOT)/p_mobj.c \
		  $(DOOM_ROOT)/p_plats.c \
		  $(DOOM_ROOT)/p_pspr.c \
		  $(DOOM_ROOT)/p_saveg.c \
		  $(DOOM_ROOT)/p_setup.c \
		  $(DOOM_ROOT)/p_sight.c \
		  $(DOOM_ROOT)/p_spec.c \
		  $(DOOM_ROOT)/p_switch.c \
		  $(DOOM_ROOT)/p_telept.c \
		  $(DOOM_ROOT)/p_tick.c \
		  $(DOOM_ROOT)/p_user.c \
		  $(DOOM_ROOT)/r_bsp.c \
		  $(DOOM_ROOT)/r_data.c \
		  $(DOOM_ROOT)/r_draw.c \
		  $(DOOM_ROOT)/r_main.c \
		  $(DOOM_ROOT)/r_plane.c \
		  $(DOOM_ROOT)/r_segs.c \
		  $(DOOM_ROOT)/r_sky.c \
		  $(DOOM_ROOT)/r_things.c \
		  $(DOOM_ROOT)/sha1.c \
		  $(DOOM_ROOT)/sounds.c \
		  $(DOOM_ROOT)/statdump.c \
		  $(DOOM_ROOT)/st_lib.c \
		  $(DOOM_ROOT)/st_stuff.c \
		  $(DOOM_ROOT)/s_sound.c \
		  $(DOOM_ROOT)/tables.c \
		  $(DOOM_ROOT)/v_video.c \
		  $(DOOM_ROOT)/wi_stuff.c \
		  $(DOOM_ROOT)/w_checksum.c \
		  $(DOOM_ROOT)/w_file.c \
		  $(DOOM_ROOT)/w_main.c \
		  $(DOOM_ROOT)/w_wad.c \
		  $(DOOM_ROOT)/z_zone.c \
		  $(DOOM_ROOT)/w_file_static.c \
		  $(DOOM_ROOT)/i_input.c \
		  $(DOOM_ROOT)/i_video.c \
		  $(DOOM_ROOT)/doomgeneric.c \
		  $(DOOM_ROOT)/mus2mid.c \

sources_guest = src/guest.c $(sources)

tmp_guest = $(sources_guest:.c=.o)
objects_guest = $(tmp_guest:.cpp=.o)

CC = clang
CXX = clang++
LD = clang
# preprocessor flags
CPPFLAGS = -Ioutput \
		   -Isrc/include \
		   -I$(SDL_ROOT)/include \
		   -I$(SDL_MIXER_ROOT)/include \
		   -I$(SDL_MIXER_ROOT)/src \
		   -I$(SDL_MIXER_ROOT)/src/codecs \
		   -I$(ADLMIDI_ROOT)/include \
		   -I$(LIBCXX_ROOT)/include \
		   -I$(MUSL_ROOT)/src/include \
		   -I$(MUSL_ROOT)/include \
		   -I$(MUSL_ROOT)/src/internal \
		   -I$(MUSL_ROOT)/src/multibyte \
		   -I$(MUSL_ROOT)/arch/generic \
		   -I$(MUSL_ROOT)/arch/$(MUSL_TARGET_ARCH) \
		   -Isrc \
		   -DHAVE_STDIO_H \
		   -DHAVE_O_CLOEXEC \
		   -DDYNAPI_NEEDS_DLOPEN=1 \
		   -DSDL_THREADS_DISABLED=1 \
		   -DSDL_SENSOR_DISABLED=1 \
		   -DSDL_HAPTIC_DISABLED=1 \
		   -DSDL_JOYSTICK_DISABLED=1 \
		   -DSDL_VIDEO_DISABLED=1 \
		   -DSDL_TIMERS_DISABLED=1 \
		   -D_GNU_SOURCE \
		   -DFEATURE_SOUND \
		   -DMUSIC_MID_ADLMIDI \
		   -DADLMIDI_DISABLE_NUKED_EMULATOR \
		   -DADLMIDI_DISABLE_OPAL_EMULATOR \
		   -DADLMIDI_DISABLE_JAVA_EMULATOR \
		   -DHAVE_MMAP=0 \
		   -DHAVE_MREMAP=0 \
		   -Dmalloc_getpagesize=4096 \
		   -DMORECORE_CANNOT_TRIM=1 \
		   -Dconstinit="" \
		   -Ilibs/miniz \
		   -DMINIZ_NO_INFLATE_APIS \
		   -Ilibs/stb \
		   -DSTBI_WRITE_NO_STDIO
LDFLAGS = $(TARGET_FLAGS) \
		  -Wl,--error-limit=0 \
		  -Wl,--emit-relocs \
		  -Wl,--no-relax
CFLAGS = $(TARGET_FLAGS) \
		 -mrelax \
		 -fpic \
		 -fPIE \
		 -ffast-math \
		 -gdwarf-5 \
		 -g3 \
		 -O3 \
		 -fdebug-prefix-map=$(PWD)=polkadoom \
		 -Werror=return-type \
		 -Wno-implicit-int \
		 -Wno-int-conversion \
		 -Wno-shift-op-parentheses \
		 -Wno-visibility \
		 -Wno-absolute-value \
		 -Wno-pointer-sign \
		 -Wno-string-plus-int
CXXFLAGS = $(CFLAGS) \
		   -fno-exceptions \
		   -fno-rtti

.PHONY: all clean run

all: $(outfile_guest_polkavm)

$(outfile_guest_polkavm): $(outfile_guest)
	polkatool link -s $< -o $@

$(outfile_guest): $(objects_guest) libclang_rt.builtins-riscv$(TARGET_BITNESS).a
	$(CC) $(LDFLAGS) $+ -o $@

libs/doomgeneric/doomgeneric/i_video.o: src/corevm_guest.h src/polkavm_guest.h
src/guest.o: src/corevm_guest.h src/polkavm_guest.h

output/doom1_wad.c: roms/doom1.wad
	xxd -i $< >$@
	sed -i \
		-e 's/unsigned char/const unsigned char/' \
		-e 's/\] = /] __attribute__((aligned(4))) = /' \
		$@

clean:
	find . -name '*.o' -delete
	rm -f output/*.elf output/*.polkavm

