// Copyright 2020 Barger M., Knoll M., Kofler L.

#ifndef INCLUDE_WAV_HPP_
#define INCLUDE_WAV_HPP_

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>

#include "log.hpp"

// WAV Header
#define WAV_HEADER_SIZE 44
struct WAV_HEADER {
  // RIFF Header
  char riff_magic_num[4];
  uint32_t file_size;
  char wav_magic_num[4];

  // FMT Header
  char fmt_magic_num[4];
  uint32_t fmt_hdr_len;
  uint16_t format_tag;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t bytes_per_second;
  uint16_t block_align;
  uint16_t bits_per_sample;

  // Chunk Header
  char chunk_magic_num[4];
  uint32_t data_block_len;
};

class Wav {
 public:
  // Constructor
  //
  // file_path: path to the wav file
  Wav();

  // Extracts information from a wav file
  //
  // return: was extraction successful
  bool Read(std::string file_path);

  bool Read(std::vector<int8_t> alaw_samples);

  // return: samples in the file
  std::vector<int16_t> GetSamples();

  // return: sample rate
  uint GetSampleRate();

  // Prints all header information to the console
  void PrintHeaderInfo();

  bool IsWavFile();

  double GetDuration();


 private:
  int16_t DecodeAlawSample(int8_t number);
  void DecodeAlaw(std::vector<int8_t> alaw_samples);

  std::vector<int16_t> samples;
  std::string file_name;
  WAV_HEADER wav_hdr;
  double duration;
};

#endif  // INCLUDE_WAV_HPP_
