add_library(OpenAL STATIC)

target_folder(OpenAL ${FOLDER_EXTERNAL})

set(OPENAL_DIR ../OpenAL-git)

target_sources_sandboxed(OpenAL
  DIRECTORY openal

  STRIP
  ${OPENAL_DIR}/

  SOURCES
  config.h
  version.h
  default_hrtf.txt

  ${OPENAL_DIR}/include/al/al.h
  ${OPENAL_DIR}/include/al/alc.h
  ${OPENAL_DIR}/include/al/alext.h
  ${OPENAL_DIR}/include/al/efx-creative.h
  ${OPENAL_DIR}/include/al/efx-presets.h
  ${OPENAL_DIR}/include/al/efx.h

  ${OPENAL_DIR}/common/albit.h
  ${OPENAL_DIR}/common/albyte.h
  ${OPENAL_DIR}/common/alcomplex.cpp
  ${OPENAL_DIR}/common/alcomplex.h
  ${OPENAL_DIR}/common/aldeque.h
  ${OPENAL_DIR}/common/alfstream.cpp
  ${OPENAL_DIR}/common/alfstream.h
  ${OPENAL_DIR}/common/almalloc.cpp
  ${OPENAL_DIR}/common/almalloc.h
  ${OPENAL_DIR}/common/alnumbers.h
  ${OPENAL_DIR}/common/alnumeric.h
  ${OPENAL_DIR}/common/aloptional.h
  ${OPENAL_DIR}/common/alspan.h
  ${OPENAL_DIR}/common/alstring.cpp
  ${OPENAL_DIR}/common/alstring.h
  ${OPENAL_DIR}/common/altraits.h
  ${OPENAL_DIR}/common/atomic.h
  ${OPENAL_DIR}/common/comptr.h
  ${OPENAL_DIR}/common/dynload.cpp
  ${OPENAL_DIR}/common/dynload.h
  ${OPENAL_DIR}/common/intrusive_ptr.h
  ${OPENAL_DIR}/common/opthelpers.h
  ${OPENAL_DIR}/common/phase_shifter.h
  ${OPENAL_DIR}/common/polyphase_resampler.cpp
  ${OPENAL_DIR}/common/polyphase_resampler.h
  ${OPENAL_DIR}/common/pragmadefs.h
  ${OPENAL_DIR}/common/ringbuffer.cpp
  ${OPENAL_DIR}/common/ringbuffer.h
  ${OPENAL_DIR}/common/strutils.cpp
  ${OPENAL_DIR}/common/strutils.h
  ${OPENAL_DIR}/common/threads.cpp
  ${OPENAL_DIR}/common/threads.h
  ${OPENAL_DIR}/common/vecmat.h
  ${OPENAL_DIR}/common/vector.h
  ${OPENAL_DIR}/al/auxeffectslot.cpp
  ${OPENAL_DIR}/al/auxeffectslot.h
  ${OPENAL_DIR}/al/buffer.cpp
  ${OPENAL_DIR}/al/buffer.h
  ${OPENAL_DIR}/al/effect.cpp
  ${OPENAL_DIR}/al/effect.h
  ${OPENAL_DIR}/al/effects/autowah.cpp

  ${OPENAL_DIR}/al/effects/chorus.cpp

  ${OPENAL_DIR}/al/effects/compressor.cpp

  ${OPENAL_DIR}/al/effects/convolution.cpp

  ${OPENAL_DIR}/al/effects/dedicated.cpp

  ${OPENAL_DIR}/al/effects/distortion.cpp

  ${OPENAL_DIR}/al/effects/echo.cpp

  ${OPENAL_DIR}/al/effects/effects.cpp
  ${OPENAL_DIR}/al/effects/effects.h
  ${OPENAL_DIR}/al/effects/equalizer.cpp

  ${OPENAL_DIR}/al/effects/fshifter.cpp

  ${OPENAL_DIR}/al/effects/modulator.cpp

  ${OPENAL_DIR}/al/effects/null.cpp

  ${OPENAL_DIR}/al/effects/pshifter.cpp

  ${OPENAL_DIR}/al/effects/reverb.cpp

  ${OPENAL_DIR}/al/effects/vmorpher.cpp

  ${OPENAL_DIR}/al/error.cpp
  ${OPENAL_DIR}/al/event.cpp
  ${OPENAL_DIR}/al/event.h
  ${OPENAL_DIR}/al/extension.cpp
  ${OPENAL_DIR}/al/filter.cpp
  ${OPENAL_DIR}/al/filter.h
  ${OPENAL_DIR}/al/listener.cpp
  ${OPENAL_DIR}/al/listener.h
  ${OPENAL_DIR}/al/source.cpp
  ${OPENAL_DIR}/al/source.h
  ${OPENAL_DIR}/al/state.cpp
  ${OPENAL_DIR}/al/eax/api.cpp
  ${OPENAL_DIR}/al/eax/api.h
  ${OPENAL_DIR}/al/eax/call.cpp
  ${OPENAL_DIR}/al/eax/call.h
  ${OPENAL_DIR}/al/eax/effect.h
  ${OPENAL_DIR}/al/eax/exception.cpp
  ${OPENAL_DIR}/al/eax/exception.h
  ${OPENAL_DIR}/al/eax/fx_slot_index.cpp
  ${OPENAL_DIR}/al/eax/fx_slot_index.h
  ${OPENAL_DIR}/al/eax/fx_slots.cpp
  ${OPENAL_DIR}/al/eax/fx_slots.h
  ${OPENAL_DIR}/al/eax/globals.cpp
  ${OPENAL_DIR}/al/eax/globals.h
  ${OPENAL_DIR}/al/eax/utils.cpp
  ${OPENAL_DIR}/al/eax/utils.h
  ${OPENAL_DIR}/al/eax/x_ram.h
  ${OPENAL_DIR}/alc/alc.cpp
  ${OPENAL_DIR}/alc/alu.cpp
  ${OPENAL_DIR}/alc/alu.h
  ${OPENAL_DIR}/alc/alconfig.cpp
  ${OPENAL_DIR}/alc/alconfig.h
  ${OPENAL_DIR}/alc/context.cpp

  ${OPENAL_DIR}/alc/context.h
  ${OPENAL_DIR}/alc/device.cpp

  ${OPENAL_DIR}/alc/device.h
  ${OPENAL_DIR}/alc/effects/base.h
  ${OPENAL_DIR}/alc/effects/autowah.cpp

  ${OPENAL_DIR}/alc/effects/chorus.cpp

  ${OPENAL_DIR}/alc/effects/compressor.cpp

  ${OPENAL_DIR}/alc/effects/convolution.cpp

  ${OPENAL_DIR}/alc/effects/dedicated.cpp

  ${OPENAL_DIR}/alc/effects/distortion.cpp

  ${OPENAL_DIR}/alc/effects/echo.cpp

  ${OPENAL_DIR}/alc/effects/equalizer.cpp

  ${OPENAL_DIR}/alc/effects/fshifter.cpp

  ${OPENAL_DIR}/alc/effects/modulator.cpp

  ${OPENAL_DIR}/alc/effects/null.cpp
  ${OPENAL_DIR}/alc/effects/pshifter.cpp
  ${OPENAL_DIR}/alc/effects/reverb.cpp
  ${OPENAL_DIR}/alc/effects/vmorpher.cpp
  ${OPENAL_DIR}/alc/inprogext.h
  ${OPENAL_DIR}/alc/panning.cpp
  ${OPENAL_DIR}/alc/backends/base.cpp
  ${OPENAL_DIR}/alc/backends/base.h
  ${OPENAL_DIR}/alc/backends/loopback.cpp
  ${OPENAL_DIR}/alc/backends/loopback.h
  ${OPENAL_DIR}/alc/backends/null.cpp
  ${OPENAL_DIR}/alc/backends/null.h
  ${OPENAL_DIR}/alc/backends/winmm.cpp
  ${OPENAL_DIR}/alc/backends/winmm.h
  ${OPENAL_DIR}/alc/backends/dsound.cpp
  ${OPENAL_DIR}/alc/backends/dsound.h
  ${OPENAL_DIR}/alc/backends/wasapi.cpp
  ${OPENAL_DIR}/alc/backends/wasapi.h
  ${OPENAL_DIR}/alc/backends/wave.cpp
  ${OPENAL_DIR}/alc/backends/wave.h
  ${OPENAL_DIR}/core/ambdec.cpp
  ${OPENAL_DIR}/core/ambdec.h
  ${OPENAL_DIR}/core/ambidefs.cpp
  ${OPENAL_DIR}/core/ambidefs.h
  ${OPENAL_DIR}/core/async_event.h
  ${OPENAL_DIR}/core/bformatdec.cpp
  ${OPENAL_DIR}/core/bformatdec.h
  ${OPENAL_DIR}/core/bs2b.cpp
  ${OPENAL_DIR}/core/bs2b.h
  ${OPENAL_DIR}/core/bsinc_defs.h
  ${OPENAL_DIR}/core/bsinc_tables.cpp
  ${OPENAL_DIR}/core/bsinc_tables.h
  ${OPENAL_DIR}/core/bufferline.h
  ${OPENAL_DIR}/core/buffer_storage.cpp
  ${OPENAL_DIR}/core/buffer_storage.h
  ${OPENAL_DIR}/core/context.cpp
  ${OPENAL_DIR}/core/context.h
  ${OPENAL_DIR}/core/converter.cpp
  ${OPENAL_DIR}/core/converter.h
  ${OPENAL_DIR}/core/cpu_caps.cpp
  ${OPENAL_DIR}/core/cpu_caps.h
  ${OPENAL_DIR}/core/cubic_defs.h
  ${OPENAL_DIR}/core/cubic_tables.cpp
  ${OPENAL_DIR}/core/cubic_tables.h
  ${OPENAL_DIR}/core/devformat.cpp
  ${OPENAL_DIR}/core/devformat.h
  ${OPENAL_DIR}/core/device.cpp
  ${OPENAL_DIR}/core/device.h
  ${OPENAL_DIR}/core/effects/base.h
  ${OPENAL_DIR}/core/effectslot.cpp
  ${OPENAL_DIR}/core/effectslot.h
  ${OPENAL_DIR}/core/except.cpp
  ${OPENAL_DIR}/core/except.h
  ${OPENAL_DIR}/core/filters/biquad.h
  ${OPENAL_DIR}/core/filters/biquad.cpp
  ${OPENAL_DIR}/core/filters/nfc.cpp
  ${OPENAL_DIR}/core/filters/nfc.h
  ${OPENAL_DIR}/core/filters/splitter.cpp
  ${OPENAL_DIR}/core/filters/splitter.h
  ${OPENAL_DIR}/core/fmt_traits.cpp
  ${OPENAL_DIR}/core/fmt_traits.h
  ${OPENAL_DIR}/core/fpu_ctrl.cpp
  ${OPENAL_DIR}/core/fpu_ctrl.h
  ${OPENAL_DIR}/core/front_stablizer.h
  ${OPENAL_DIR}/core/helpers.cpp
  ${OPENAL_DIR}/core/helpers.h
  ${OPENAL_DIR}/core/hrtf.cpp
  ${OPENAL_DIR}/core/hrtf.h
  ${OPENAL_DIR}/core/logging.cpp
  ${OPENAL_DIR}/core/logging.h
  ${OPENAL_DIR}/core/mastering.cpp
  ${OPENAL_DIR}/core/mastering.h
  ${OPENAL_DIR}/core/mixer.cpp
  ${OPENAL_DIR}/core/mixer.h
  ${OPENAL_DIR}/core/resampler_limits.h
  ${OPENAL_DIR}/core/uhjfilter.cpp
  ${OPENAL_DIR}/core/uhjfilter.h
  ${OPENAL_DIR}/core/uiddefs.cpp
  ${OPENAL_DIR}/core/voice.cpp
  ${OPENAL_DIR}/core/voice.h
  ${OPENAL_DIR}/core/voice_change.h
  ${OPENAL_DIR}/core/mixer/defs.h
  ${OPENAL_DIR}/core/mixer/hrtfbase.h
  ${OPENAL_DIR}/core/mixer/hrtfdefs.h
  ${OPENAL_DIR}/core/mixer/mixer_c.cpp
  ${OPENAL_DIR}/core/mixer/mixer_sse.cpp
  ${OPENAL_DIR}/core/mixer/mixer_sse2.cpp
  ${OPENAL_DIR}/core/mixer/mixer_sse3.cpp
  ${OPENAL_DIR}/core/mixer/mixer_sse41.cpp
)

target_compile_definitions(OpenAL
  PUBLIC
  AL_LIBTYPE_STATIC
  AL_BUILD_LIBRARY
  AL_ALEXT_PROTOTYPES
  ALC_API=
  AL_API=
  NOMINMAX
  RESTRICT=__restrict
  PRIVATE
  _CRT_SECURE_NO_WARNINGS
)

target_compile_options(OpenAL
  PRIVATE
  # Enable standards-conformance mode on MSVC
  $<$<CXX_COMPILER_ID:MSVC>:/permissive->
  # Suppress compiler warnings
  $<$<CXX_COMPILER_ID:MSVC>:/wd4065>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4127>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4267>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4324>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4530>
  $<$<CXX_COMPILER_ID:MSVC>:/wd5030>
)

target_include_directories(OpenAL
  PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/openal
  ${CMAKE_CURRENT_BINARY_DIR}/openal/common
  ${CMAKE_CURRENT_BINARY_DIR}/openal/include
)
