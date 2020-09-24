// Copyright 2020 Barger M., Knoll M., Kofler L.

#include "log.hpp"

Logger* Logger::instance = nullptr;
std::ofstream Logger::log_file;
const char* Logger::file_name = "log.txt";

Logger::Logger() {
    debug_mode = 0;
}

Logger::~Logger() {
    log_file.close();
}

Logger* Logger::GetLogger() {
    if (instance == nullptr) {
        instance = new Logger();
        log_file.open(file_name, std::ofstream::app);
    }
    return instance;
}

const std::string Logger::CurrentDateTime() {
    char buff[20];
    time_t now = time(nullptr);
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    return buff;
}

std::string Logger::GetLogLevelString(LogLevel level) {
    switch (level) {
        case LOG_LVL_ERROR: return "ERROR";
        case LOG_LVL_FATAL: return "FATAL";
        case LOG_LVL_INFO: return "INFO";
        case LOG_LVL_WARN: return "WARNING";
        case LOG_LVL_TEST: return "TEST";
        case LOG_LVL_STATUS: return "STATUS";

        default: return "";
    }
}

void Logger::EnableDebug(bool debug_mode) {
    this->debug_mode = debug_mode;
}

void Logger::Log(const std::string& message, LogLevel level) {
  Log(message, level, 0, "");
}

void Logger::Log(const std::string& message, LogLevel level, int threadid) {
  Log(message, level, threadid, "");
}

void Logger::Log(const std::string& message, LogLevel level,  int threadid, std::string number) {
  std::string thread_id_str = threadid == 0 ? "" : "[T" + std::to_string(threadid) + "] ";
  std::string number_str = number.length() == 0 ? "" : "[" + number + "] ";
  std::string date_str = "[" + CurrentDateTime() + "] ";
  std::string log_lvl_str =  "[" + GetLogLevelString(level) + "] ";
  std::string message_str = thread_id_str + log_lvl_str +  number_str + message + "\n";
  std::string log_message_str = date_str + message_str;
  log_file << log_message_str;
  log_file.flush();

  if (debug_mode) {
      std::cout << message_str;
  } else {
    if (GetLogLevelString(level).compare("STATUS") == 0) {
      std::cout << message_str;
    } else if (GetLogLevelString(level).compare("ERROR") == 0) {
      std::cerr << message_str;
    }
  }
}
