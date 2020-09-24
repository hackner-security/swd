// Copyright 2020 Barger M., Knoll M., Kofler L.
#include "audio_analyzer.hpp"

AudioAnalyzer::AudioAnalyzer() {
    this->max_frq = 0;
    this->max_peak = 0;
}

void  AudioAnalyzer::GetSpectraFromFile(Wav *wav) {
  this->wav = wav;

  kiss_fftr_cfg cfg = NULL;
  kiss_fft_scalar *tbuf;
  kiss_fft_cpx *fbuf;

  float *mag2buf;
  uint nfreqs;
  int avfctr = 0;
  int nrows = 0;
  int navg = 1;

  cfg = kiss_fftr_alloc(NFFT, 0, 0, 0);
  tbuf = static_cast<kiss_fft_scalar*>(malloc(sizeof(kiss_fft_scalar)*(NFFT + 2)));
  fbuf = static_cast<kiss_fft_cpx*>(malloc(sizeof(kiss_fft_cpx)*(NFFT + 2)));
  mag2buf = static_cast<float*>(malloc(sizeof(float)*(NFFT + 2)));

  uint idx = 0;
  uint samples_len = wav->GetSamples().size();


  while (idx < samples_len) {
  	/*
        Prepare buffer for fft. Since fft can only calculate frequeny
        bins with a size of the power of two nfft has to be the next power
        of two of the sample rate. Since nfft is bigger than sample rate
        we have to fill the rest of the buffer with zeros.
    */
    for (uint i = 0; i < NFFT; i++) {
      if (idx + i >= samples_len) {
        tbuf[i] = 0;
      } else {
        tbuf[i] = wav->GetSamples().at(idx + i);
      }
    }

    // Get frequency spectrum from a frequency bin
    kiss_fftr(cfg, tbuf, fbuf);
    // Calculate nyquist frequency = max frequency NFFT / 2
    nfreqs = NFFT / 2 - 1;

    // Fill buffer with frequencys
    for (uint i = 0; i < nfreqs; ++i) {
      mag2buf[i] += fbuf[i].r * fbuf[i].r + fbuf[i].i * fbuf[i].i;
    }

    if (++avfctr == navg) {
      float eps = 1;
      avfctr = 0;
      nrows++;
      std::vector<Measurement> set;

      // There highest nfreqs can only be = nfreqs
      for (uint i = 0; i < nfreqs; ++i) {
        float pwr = 10 * log10(mag2buf[i] / navg + eps);
        Measurement measurement;

        // amplitude of frequency
        measurement.power = pwr;
        // frequency
        measurement.frequency = static_cast<float>(i) *
          (static_cast<float>(wav->GetSampleRate()) / 2) / static_cast<float>(nfreqs);
        set.push_back(measurement);
      }

      // push set of freqeuncy and amplitude. freuqency is the key (first element in vector)
      spectra.push_back(set);
      memset(mag2buf, 0, sizeof(mag2buf[0]) * nfreqs);
    }

      // Go to next freqeuency frame
    idx += NFFT;
  }

  free(cfg);
  free(tbuf);
  free(fbuf);
  free(mag2buf);
}

bool CompareByAmplitude(const Measurement &a, const Measurement &b) {
  return (a.power > b.power);
}

void AudioAnalyzer::CalculateSignificantFrequencies() {
  std::vector<std::vector<Measurement>> pkz;

  for (std::vector<Measurement> measurements : spectra) {
    std::sort(measurements.begin(), measurements.end(), CompareByAmplitude);
    std::vector<Measurement> tmp;

    for (uint i = 0; i < 10; i++) {
        tmp.push_back(measurements.at(i));
    }

    pkz.push_back(tmp);
  }

  for (int i = 0; i < 800; i++) {
    fcnt.insert(std::pair<int, float>(i * 5, 0.f));
  }

  for (std::vector<Measurement> measurments : pkz) {
    for (Measurement measurment : measurments) {
      int fdx = static_cast<int>(std::round(measurment.frequency / 5.0) * 5.0);

      std::map<int, float>::iterator fcnt_i = fcnt.find(fdx);
      fcnt_i->second += 0.1;
    }
  }
}

void AudioAnalyzer::CalculatePeakFrequency() {
  std::vector<std::vector<Measurement>> sorted_freqs;

  for (std::vector<Measurement> measurements : spectra) {
    std::sort(measurements.begin(), measurements.end(), CompareByAmplitude);
    std::vector<Measurement> tmp;

    for (uint i = 0; i < 10; i++) {
        tmp.push_back(measurements.at(i));
    }

    sorted_freqs.push_back(tmp);
  }

  for (std::vector<Measurement> measurements : sorted_freqs) {
    for (Measurement measurement : measurements) {
      int f = std::round(measurement.frequency);
      int p = std::round(measurement.power);

      if (f == 0) continue;
      if (p < 1) continue;

      if (measurement.power > max_peak) {
        max_frq = measurement.frequency;
        max_peak = measurement.power;
      }
    }
  }

  if (max_frq != 0 && max_peak != 0) {
    Logger::GetLogger()->Log("Peak frequency in file " + std::to_string(max_frq)
    + " with peak " + std::to_string(max_peak), LOG_LVL_INFO);
  }
}

bool AudioAnalyzer::IsModem() {
  if ( (fcnt.find(2100)->second > 1.0 || fcnt.find(2230)->second > 1.0 )
      && fcnt.find(2250)->second > 0.5) {
    return true;
  } else if (fcnt.find(2100)->second > 1.0 && (max_frq > 2245.0 && max_frq < 2255.0)) {
    return true;
  } else if (fcnt.find(2100)->second> 1.0 && (max_frq > 2995.0 && max_frq < 3005.0)) {
    return true;
  }

  return false;
}

bool AudioAnalyzer::IsFax() {
  int fax_freqs[] = {1625, 1665, 1825, 600, 1855,
    1100, 2250, 2230, 2220, 1800};
  double fax_sum = 0.0;

  for (int fax_freq : fax_freqs) {
    fax_sum += fcnt.find(fax_freq)->second;
  }

  if (fax_sum > 2.0) {
    return true;
  }

  return false;
}

void AudioAnalyzer::PrintSignificantFrequencies(float bottom_limit) {
  for (std::pair<int, float> pair : fcnt) {
    if (pair.second > bottom_limit) {
      std::cout << pair.first << " : " << pair.second << std::endl;
    }
  }
}

int AudioAnalyzer::GetMaxFrequency() {
  return max_frq;
}

LineType AudioAnalyzer::GetLineType() {
  if (IsModem()) {
    return LineType::MODEM;
  } else if (IsFax()) {
    return LineType::FAX;
  }
  return LineType::OTHER;
}

std::string AudioAnalyzer::GetReadableLineType() {
  LineType line_type = GetLineType();
  std::string readable_line_type;

  switch (line_type) {
  case LineType::FAX:
    readable_line_type = "Fax";
    break;
  case LineType::MODEM:
    readable_line_type = "Modem";
    break;
  default:
    readable_line_type = "Other";
    break;
  }

  return readable_line_type;
}

void AudioAnalyzer::Analyze(Wav *wav) {
  if (wav->GetSampleRate() == 8000) {
    GetSpectraFromFile(wav);
    CalculateSignificantFrequencies();
    CalculatePeakFrequency();
  } else {
    Logger::GetLogger()->Log("Can't analyze audio because sample rate is not 8000 samples per second"  , LOG_LVL_ERROR);
  }
}


