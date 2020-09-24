// Copyright 2020 Barger M., Knoll M., Kofler L.
#include "wav.hpp"

Wav::Wav() {
  this->file_name = "";
}

bool Wav::Read(std::string file_name) {
  this->file_name = file_name;

  std::ifstream wav_file(file_name, std::ios::binary | std::ios::ate);

  if (wav_file.is_open()) {
    int size = wav_file.tellg();

    wav_file.seekg(0, std::ios::beg);
    wav_file.read(reinterpret_cast<char *>(&wav_hdr), WAV_HEADER_SIZE);
    wav_file.seekg(WAV_HEADER_SIZE, std::ios::beg);

    int data_len = (size - WAV_HEADER_SIZE) / 2;

    samples = std::vector<int16_t>(data_len);
    wav_file.read(reinterpret_cast<char *> (&samples[0]), data_len*2);

    wav_file.close();

    if (wav_file.fail()) {
      Logger::GetLogger()->Log("Something terrible happened to the file :(", LOG_LVL_FATAL);
    } else {
        if (IsWavFile()) {
          duration = static_cast<double>(samples.size()) / static_cast<double>(wav_hdr.sample_rate);
      } else {
        Logger::GetLogger()->Log("File has wrong format!", LOG_LVL_ERROR);
        return false;
      }
    }
  } else {
    Logger::GetLogger()->Log("File doesn't exist!", LOG_LVL_ERROR);
    return false;
  }

  return true;
}

bool Wav::Read(std::vector<int8_t> alaw_samples) {
  if (alaw_samples.size() == 0) {
    return false;
  }

  DecodeAlaw(alaw_samples);
  wav_hdr.sample_rate = 8000;

  return true;
}

void Wav::PrintHeaderInfo() {
  std::cout << "Magic Num: " << wav_hdr.riff_magic_num << std::endl;
  std::cout << "Filename: " << file_name << std::endl;
  std::cout << "Format tag: " << wav_hdr.format_tag << std::endl;
  std::cout << "Sample rate: " << wav_hdr.sample_rate << std::endl;
  std::cout << "Bytes per second: " << wav_hdr.bytes_per_second << std::endl;
  std::cout << "Bits per sample: " << wav_hdr.bits_per_sample << std::endl;
  std::cout << "Data block length: " << wav_hdr.data_block_len << std::endl;
  std::cout << "Samples: " << samples.size() << std::endl;
  std::cout << "Duration: " << duration << " seconds" << std::endl;
}

std::vector<int16_t> Wav::GetSamples() {
  return samples;
}

uint Wav::GetSampleRate() {
  return wav_hdr.sample_rate;
}

double Wav::GetDuration() {
  return duration;
}

bool Wav::IsWavFile() {
  std::string riff_magic_num(wav_hdr.riff_magic_num, 4);
  std::string wav_magic_num(wav_hdr.wav_magic_num, 4);
  std::string fmt_magic_num(wav_hdr.fmt_magic_num, 4);

  if (riff_magic_num == "RIFF" && fmt_magic_num == "fmt "
    && wav_magic_num == "WAVE") {
      return true;
  }

  return false;
}

void Wav::DecodeAlaw(std::vector<int8_t> alaw_samples) {
  for (int8_t alaw_sample : alaw_samples) {
    samples.push_back(DecodeAlawSample(alaw_sample));
  }
}

int16_t Wav::DecodeAlawSample(int8_t number) {
  uint8_t sign = 0x00;
  uint8_t position = 0;
  int16_t decoded = 0;
  number^=0x55;

  if (number & 0x80) {
    number&=~(1<<7);
    sign = -1;
  }

  position = ((number & 0xF0) >> 4) + 4;

  if (position != 4) {
    decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
      | (1 << (position - 5)));
  } else {
    decoded = (number << 1) | 1;
  }

  return (sign == 0) ? (decoded) : (-decoded);
}
