// Copyright 2020 Barger M., Knoll M., Kofler L.
#ifndef INCLUDE_WARDIALER_HPP_
#define INCLUDE_WARDIALER_HPP_

#include <thread> // NOLINT
#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <iomanip>
#include <chrono> // NOLINT

#include "wav.hpp"
#include "audio_analyzer.hpp"
#include "sip_client.hpp"
#include "argparse.hpp"
#include "db_client.hpp"

int Wardialer();
void WardialThread(std::vector<std::string> numbers, int thread_id);
void dec_thread_counter();
void inc_thread_counter();

#endif  //  INCLUDE_WARDIALER_HPP_
