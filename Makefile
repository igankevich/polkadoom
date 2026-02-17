DOOM_ROOT = libs/doomgeneric/doomgeneric
SDL_ROOT = libs/SDL
SDL_MIXER_ROOT = libs/SDL-Mixer-X
ADLMIDI_ROOT = libs/libADLMIDI
LIBCXX_ROOT = libs/libcxx
MIN_STACK_SIZE = 1048576

include output/config.mk

outfile_guest = output/doom$(DOOM_SUFFIX).elf
outfile_guest_corevm = $(outfile_guest:.elf=.corevm)

sources = src/impl_dummy_sdl.c \
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
		  $(DOOM_ROOT)/w_file_stdc.c \
		  $(DOOM_ROOT)/i_input.c \
		  $(DOOM_ROOT)/i_video.c \
		  $(DOOM_ROOT)/doomgeneric.c \
		  $(DOOM_ROOT)/mus2mid.c \

sources_guest = src/guest.c $(sources)

tmp_guest = $(sources_guest:.c=.o)
objects_guest = $(tmp_guest:.cpp=.o)

CC = polkavm-cc
CXX = polkavm-c++
LD = polkavm-cc
# preprocessor flags
CPPFLAGS = -Ioutput \
		   -Isrc/include \
		   -I$(SDL_ROOT)/include \
		   -I$(SDL_MIXER_ROOT)/include \
		   -I$(SDL_MIXER_ROOT)/src \
		   -I$(SDL_MIXER_ROOT)/src/codecs \
		   -I$(ADLMIDI_ROOT)/include \
		   -I$(LIBCXX_ROOT)/include \
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
		   -Dconstinit=
LDFLAGS = 
CFLAGS = -ffast-math \
		 -gdwarf-5 \
		 -g3 \
		 -O3 \
		 -fdebug-prefix-map=$(PWD)=polkadoom \
		 -Werror=return-type \
		 -Werror=date-time \
		 -Wno-implicit-int \
		 -Wno-int-conversion \
		 -Wno-shift-op-parentheses \
		 -Wno-visibility \
		 -Wno-absolute-value \
		 -Wno-pointer-sign \
		 -Wno-string-plus-int
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

.PHONY: all clean run

all: $(outfile_guest_corevm)

$(outfile_guest_corevm): $(outfile_guest)
	polkatool link -s $< -o $@.tmp --min-stack-size $(MIN_STACK_SIZE)
	jam-blob set-meta \
		--name PolkaDoom \
		--version 0.1 \
		--license GPLv2 \
		--author 'Parity Technologies <admin@parity.io>' \
		$@.tmp
	mv $@.tmp $@


$(outfile_guest): $(objects_guest)
	$(CC) $(LDFLAGS) $+ -o $@

libs/doomgeneric/doomgeneric/i_video.o: src/corevm_guest.h src/polkavm_guest.h
src/guest.o: src/corevm_guest.h src/polkavm_guest.h output/config.h

clean:
	find . -name '*.o' -delete
	rm -f output/*.elf output/*.polkavm

