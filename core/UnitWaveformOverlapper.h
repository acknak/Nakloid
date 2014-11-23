﻿#ifndef UnitWaveformOverlapper_h
#define UnitWaveformOverlapper_h

#include "../format/UnitWaveformContainer.h"

#include <vector>
#include <utility>
#include <boost/filesystem/path.hpp>

class UnitWaveformContainer;

class UnitWaveformOverlapper {
 public:
  static struct Parameters {
    Parameters() {
      stretch_self_fade = false;
      ms_self_fade = 1.0;
      interpolation = false;
      overlap_normalize = false;
      max_volume = 0.9;
      compressor = false;
      compressor_threshold = -18.0;
      compressor_ratio = 2.5;
      num_lobes = 1;
      window_modification = true;
    };
    bool stretch_self_fade;
    double ms_self_fade;
    bool interpolation;
    bool overlap_normalize;
    double max_volume;
    bool compressor;
    double compressor_threshold;
    double compressor_ratio;
    short num_lobes;
    bool window_modification;
    WavHeader wav_header;
  } params;

  UnitWaveformOverlapper(const std::vector<long>& pitchmarks);
  virtual ~UnitWaveformOverlapper();

  bool overlapping(const UnitWaveformContainer* const uwc, std::pair<long, long> ms_note_pron, const std::vector<short>& velocities);
  bool overlapping(const UnitWaveformContainer* const uwc, std::pair<long, long> ms_note_pron, long ms_note_margin, const std::vector<short>& velocities);
  void outputWav(const boost::filesystem::path& path_output) const;

  // accessor
  const std::vector<long>& getPitchmarks() const;

 private:
  UnitWaveformOverlapper(const UnitWaveformOverlapper& other);
  UnitWaveformOverlapper& operator=(const UnitWaveformOverlapper& other);

  class PitchMarkObject {
   public:
    explicit PitchMarkObject(std::vector<long>::const_iterator it):it(it),scale(1.0){};
    virtual ~PitchMarkObject(){};

    class UnitWaveformParams {
     public:
      UnitWaveformParams(std::vector<UnitWaveform>::const_iterator it, double scale, long output_pitch, long base_f0, unsigned char lobe);
      UnitWaveform uw;
      double scale;
    };
    std::vector<long>::const_iterator it; //pitchmarks iterator
    std::vector<UnitWaveformParams> uwps;
    double scale;
    double getRmsMean();
  };

  std::vector<long>::const_iterator pos2it(long pos) const;
  std::vector<UnitWaveform>::const_iterator binary_pos_search(std::vector<UnitWaveform>::const_iterator from, std::vector<UnitWaveform>::const_iterator to, const long pos_target) const;
  std::vector<long> pitchmarks;
  std::vector<double> output_wav;
};

#endif
