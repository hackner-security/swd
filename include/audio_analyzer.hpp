// Copyright 2020 Barger M., Knoll M., Kofler L.

#ifndef INCLUDE_AUDIO_ANALYZER_HPP_
#define INCLUDE_AUDIO_ANALYZER_HPP_


#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <map>
#include <set>

#include "wav.hpp"
#include "log.hpp"

#include "./kiss_fft.h"
#include "tools/kiss_fftr.h"

#define NFFT 8192

enum LineType {
  FAX, MODEM, OTHER
};

// represents a point in the spectrum coordinate system
struct Measurement {
  float frequency, power;
};

class AudioAnalyzer {
 public:
  // Constructor
  //
  // wav: Pointer to a Wav object, which contains information about
  // the pcm file
  AudioAnalyzer();

  // Returns the max frequency
  int GetMaxFrequency();

  // Returns the linetype as enum
  LineType GetLineType();

  // Returns the line type as string
  std::string GetReadableLineType();

  // Prints the sifnicant frequencies with its power.
  //
  // bottom_limit: only frequenies with a significant over bottom_limit
  // will be printed
  void PrintSignificantFrequencies(float bottom_limit);

  void Analyze(Wav *wav);

 private:
  Wav *wav;                                         // audio data and further information about the audio file
  std::vector<std::vector<Measurement>> spectra;    // frequency spectra of each second
  std::map<int, float> fcnt;                        // significant frequencies in the audio
  int max_frq;                                      // max frequency in the audio
  int max_peak;                                     // peak of the max frequency

  // Performs fft on every second in the file and extracts the
  // frequency spectrum from it.
  void GetSpectraFromFile(Wav *wav);

  // Calculates significant frequencies from each spectrum.
  // Result is saved in fcnt.
  void CalculateSignificantFrequencies();

  // Checks, if the significant frequencies match to a modem
  bool IsModem();

  // Checks, if the significant frequencies match to a modem
  bool IsFax();

  // Calculates the peak frequency in the audio file
  void CalculatePeakFrequency();
};

#endif  // INCLUDE_AUDIO_ANALYZER_HPP_
