// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_AUDIO_CONVERTER_H_
#define MEDIA_BASE_AUDIO_CONVERTER_H_

#include <list>

#include "base/callback.h"
#include "base/time.h"
#include "media/audio/audio_parameters.h"
#include "media/base/media_export.h"

namespace media {

class AudioBus;
class AudioPullFifo;
class ChannelMixer;
class MultiChannelResampler;

// AudioConverter is a complete mixing, resampling, buffering, and channel
// mixing solution for converting data from one set of AudioParameters to
// another.  For efficiency pieces are only invoked when necessary; e.g. the
// resampler is only used if the input and output sample rates differ.  Mixing
// and channel down mixing are done prior to resampling to maximize efficiency.
class MEDIA_EXPORT AudioConverter {
 public:
  class MEDIA_EXPORT InputCallback {
   public:
    // Method for providing more data into the converter.  Expects |audio_bus|
    // to be completely filled with data upon return; zero padded if not enough
    // frames are available to satisfy the request.  The return value is the
    // volume level of the provided audio data.  If a volume level of zero is
    // returned no further processing will be done on the provided data, else
    // the volume level will be used to scale the provided audio data.
    virtual double ProvideInput(AudioBus* audio_bus,
                                base::TimeDelta buffer_delay) = 0;

   protected:
    virtual ~InputCallback() {}
  };

  // Construct an AudioConverter for converting between the given input and
  // output parameters.  Specifying |disable_fifo| means all InputCallbacks are
  // capable of handling arbitrary buffer size requests; i.e. one call might ask
  // for 10 frames of data (indicated by the size of AudioBus provided) and the
  // next might ask for 20.  In synthetic testing, disabling the FIFO yields a
  // ~20% speed up for common cases.
  AudioConverter(const AudioParameters& input_params,
                 const AudioParameters& output_params,
                 bool disable_fifo);
  ~AudioConverter();

  // Converts audio from all inputs into the |dest|.  |dest| must be sized for
  // data matching the output AudioParameters provided during construction.
  void Convert(AudioBus* dest);

  // Add or remove an input from the converter.
  void AddInput(InputCallback* input);
  void RemoveInput(InputCallback* input);

  // Flush all buffered data.  Automatically called when all inputs are removed.
  void Reset();

 private:
  // Called by MultiChannelResampler when more data is necessary.
  void ProvideInput(int resampler_frame_delay, AudioBus* audio_bus);

  // Called by AudioPullFifo when more data is necessary.
  void SourceCallback(int fifo_frame_delay, AudioBus* audio_bus);

  // Set of inputs for Convert().
  typedef std::list<InputCallback*> InputCallbackSet;
  InputCallbackSet transform_inputs_;

  // Used to buffer data between the client and the output device in cases where
  // the client buffer size is not the same as the output device buffer size.
  scoped_ptr<AudioPullFifo> audio_fifo_;

  // Handles resampling.
  scoped_ptr<MultiChannelResampler> resampler_;

  // Handles channel transforms.  |unmixed_audio_| is a temporary destination
  // for audio data before it goes into the channel mixer.
  scoped_ptr<ChannelMixer> channel_mixer_;
  scoped_ptr<AudioBus> unmixed_audio_;

  // Temporary AudioBus destination for mixing inputs.
  scoped_ptr<AudioBus> mixer_input_audio_bus_;

  // Since resampling is expensive, figure out if we should downmix channels
  // before resampling.
  bool downmix_early_;

  // Used to calculate buffer delay information for InputCallbacks.
  base::TimeDelta input_frame_duration_;
  base::TimeDelta output_frame_duration_;
  int resampler_frame_delay_;

  const int input_channel_count_;

  DISALLOW_COPY_AND_ASSIGN(AudioConverter);
};

}  // namespace media

#endif  // MEDIA_BASE_AUDIO_CONVERTER_H_
