// Copyright 2020 Barger M., Knoll M., Kofler L.
#ifndef INCLUDE_LOG_HPP_
#define INCLUDE_LOG_HPP_

#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <string>

typedef enum {
  LOG_LVL_ERROR,
  LOG_LVL_FATAL,
  LOG_LVL_INFO,
  LOG_LVL_WARN,
  LOG_LVL_TEST,
  LOG_LVL_STATUS,
} LogLevel;

// Class for singelton logger
class Logger {
 public:
  // message: The Message to Log
  // level: Specify LogLevel (default LogINFO)
  void Log(const std::string& message, LogLevel level);
  void Log(const std::string& message, LogLevel level, int threadid);
  void Log(const std::string& message, LogLevel level, int threadid, std::string number);
  // enable debug mode
  void EnableDebug(bool debug_mode);
  // Function to create a Logger class
  // returns a singelton object for Logger class
  static Logger* GetLogger();

 private:
  // Constructor
  Logger();
  // Destructor
  ~Logger();
  // Function to get current date and time
  const std::string CurrentDateTime();
  // Log file name
  static const char* file_name;
  // Log file stream object
  static std::ofstream log_file;
  // Singelton Logger class object pointer
  static Logger* instance;
  // Converts LogLevelEnum to String for Logging
  std::string GetLogLevelString(LogLevel level);
  // Debug mode boolean
  bool debug_mode;
};

#endif  //  INCLUDE_LOG_HPP_
