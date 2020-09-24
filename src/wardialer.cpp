// Copyright 2020 Barger M., Knoll M., Kofler L.
#include "wardialer.hpp"

Argparser* args = Argparser::GetArgparser();
int thread_counter = 0;
int call_counter = 0;
std::atomic<int> id_ctr = 0;
DBClient db = DBClient("wardialing.db");
std::vector<std::thread> calls;
std::atomic<bool> stop_swd = false;

struct call_data {
  std::string id;
  std::vector<int8_t> alaw_samples;
};

std::vector<call_data> call_data_vector;

void signal_handler(int signal) {
  if (stop_swd == false) {
    Logger::GetLogger()->Log("Caught signal '" + std::to_string(signal) +
      "', finishing up calls and exiting.", LOG_LVL_STATUS);
    stop_swd = true;
  }
}

void segfault_handler(int signal) {
  Logger::GetLogger()->Log("Received SIGFAULT(" + std::to_string(signal) +
    "), trying to stop other threads...", LOG_LVL_ERROR);
  if (stop_swd == false) {
    stop_swd = true;
  }
}

void WardialThread(std::vector<std::string> numbers, int thread_id) {
  SIPClient *client = new SIPClient(args->GetUsername(), args->GetPassword(), args->GetServer(),
    4242 + thread_id, thread_id);
  for ( auto number : numbers ) {
    std::string id = number + "_" + std::to_string(id_ctr++);
    db.InsertData(id, number, "Ready", "");
    if ( client->Register() == true ) {
      db.UpdateEntry(id, "Calling", "");
      int call_duration = 0;
      if ( client->Invite(number, 25000.f, args->GetDebugStatus(), &call_duration) == true ) {
        call_data data;
        data.id = id;
        data.alaw_samples = client->GetCallData();
        call_data_vector.push_back(data);
        db.UpdateDuration(id, call_duration);
        db.UpdateEntry(id, "Call Finished", "");
      } else {
        db.UpdateEntry(id, "Call Failed", "");
      }
    } else {
      return;
    }
    if (stop_swd == true) {
      break;
    }
  }

  delete client;
  return;
}

void AnalyzeCallData() {
  for ( auto data : call_data_vector ) {
    db.UpdateEntry(data.id, "Analyzing", "");
    Wav wav;
    AudioAnalyzer audio_analyzer;
    if (wav.Read(data.alaw_samples) != false) {
      audio_analyzer.Analyze(&wav);
      std::string number = data.id.substr(0, data.id.find("_"));
      Logger::GetLogger()->Log("Detected device: " + audio_analyzer.GetReadableLineType(), LOG_LVL_STATUS, 0, number);
      db.UpdateEntry(data.id, "Finished", audio_analyzer.GetReadableLineType());
    } else {
      Logger::GetLogger()->Log("Analyzing failed", LOG_LVL_STATUS, 0);
      db.UpdateEntry(data.id, "Analyzing failed", "");
    }
  }
}

int Wardialer() {
  std::vector<std::string> numbers = args->GetNumbers();
  std::signal(SIGINT, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGSEGV, segfault_handler);
  int max_threads = args->GetThreads();

  // handle case if more threads are specified than numbers
  if (static_cast<int>(numbers.size()) < max_threads) {
    max_threads = numbers.size();
  }

  // caclulate how much numbers should be wardialed by each thread
  int numbers_per_thread = static_cast<int>(numbers.size()) / max_threads;
  int numbers_last_thread = numbers_per_thread;
  if (static_cast<int>(numbers.size()) != (numbers_per_thread * max_threads)) {
    numbers_last_thread =  numbers.size() - (numbers_per_thread * (max_threads - 1));
  }

  // split the numbers vector that each thread has the same amount of numbers
  // to wardial
  int number_counter;
  std::vector<std::vector<std::string>> numbers_for_threads;
  std::vector<std::string>::const_iterator first;
  std::vector<std::string>::const_iterator last;
  int gap;
  for (int i = 0; i < max_threads; i++) {
    number_counter = i * numbers_per_thread;
    first = numbers.begin() + number_counter;
    gap = number_counter + numbers_per_thread;
    if ( i == max_threads-1 ) {
      gap = number_counter + numbers_last_thread;
    }
    last = numbers.begin() + gap;
    std::vector<std::string> number_range(first, last);
    numbers_for_threads.push_back(number_range);
  }

  // Create threads with unique id and their number range
  int i = 1;
  for ( std::vector<std::string> number_range : numbers_for_threads ) {
    calls.push_back(std::thread(WardialThread, number_range, i));
    i++;
  }
  for (auto & call : calls) {
    if (call.joinable()) {
      call.join();
    }
  }
  // After all calls are finished analyze the call data
  if (stop_swd == false) {
    AnalyzeCallData();
  }
  return 0;
}
