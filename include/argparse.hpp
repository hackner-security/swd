// Copyright 2020 Barger M., Knoll M., Kofler L.
#ifndef INCLUDE_ARGPARSE_HPP_
#define INCLUDE_ARGPARSE_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <boost/program_options.hpp>
#include "log.hpp"

namespace po = boost::program_options;

class Argparser {
 public:
    // returns a singelton object for Argparser class
    static Argparser* GetArgparser();
    // Parse Arguments
    void Parse(int argc, char* argv[]);
    // Returns the username at the argument -u
    std::string GetUsername();
    // Returns the password at the argument -p
    std::string GetPassword();
    // Returns the server uri at the argument -s
    std::string GetServer();
    // Returns a vector with all numbers specified with the argument -n
    std::vector<std::string> GetNumbers();
    // Returns the the value of the argument -t
    int GetThreads();
    // Returns the path to the audiofile at the argument -f
    std::string GetPathToAudio();
    // Returns true if all arguments are specified to successfully analyse a file
    bool DoAnalyse();
    // Returns true if all arguments are specified to successfully wardial
    bool DoWardial();
    // Returns true if rtp data is to be saved to the disk
    bool GetDebugStatus();

 private:
    Argparser();
    // Parses multiple numbers which are seperated by a semicolon
    void ParseNumbers();
    // Parses a number range which is specified with NUMBER-NUMBER
    void ParseNumberRange(std::string number_range, size_t pos);
    // Prints usage informations
    void PrintUsage(bool help);
    // Variables
    // Vector where all numbers are stored
    std::vector<std::string> numbers;
    static Argparser* instance;
    bool analyze_flag = false;
    bool dial_flag = false;
    bool wardial_flag = false;
    bool debug = false;
    int threads;
    std::string username;
    std::string password;
    std::string server;
    std::string number;
    std::string path_to_audio;
    std::string path_to_numbers;
    po::variables_map vm;
    po::variables_map dial_vm;
    po::variables_map analyze_vm;
};

#endif  //  INCLUDE_ARGPARSE_HPP_
