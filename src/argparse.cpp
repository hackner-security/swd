// Copyright 2020 Barger M., Knoll M., Kofler L.

#include "argparse.hpp"
po::options_description desc("OPTIONS");
Argparser* Argparser::instance = nullptr;

Argparser::Argparser() {
}

Argparser* Argparser::GetArgparser() {
  if (instance == nullptr) {
    instance = new Argparser();
  }
  return instance;
}

void Argparser::Parse(int argc, char *argv[]) {
  if (argc < 2) {
    Argparser::PrintUsage(0);
    throw "Wrong Usage!";
  }
  try {
    desc.add_options()
        ("help,h", "produce help message")
        ("verbose,v", "enable verbose output")
        ("debug,d", "enable debug mode, saves rtp streams to the disk\n")
        ("username,u", po::value<std::string>(&username), "set SIP Provider Username")
        ("password,p", po::value<std::string>(&password), "set SIP Provider Password")
        ("server,s", po::value<std::string>(&server), "set URI of SIP Server")
        ("number,n", po::value<std::string>(&number), "set number or number range to wardial")
        ("file,f", po::value<std::string>(&path_to_numbers), "specfiy a file with numbers to wardial")
        ("threads,t", po::value<int>(&threads)->default_value(1),
                                          "set how many wardialing calls should be done parallel\n")
        ("analyse,a", po::value<std::string>(&path_to_audio), "analyze a file");

    // store values in variable map vm
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("verbose")) {
      Logger::GetLogger()->EnableDebug(true);
    }
    if (vm.count("debug")) {
    std::string warn_illegal = "You are saving call data to the disk! "
        "Depending on your local laws this might be illegal!";
      Logger::GetLogger()->Log(warn_illegal, LOG_LVL_WARN);
      this->debug = true;
    }
    if (vm.count("help")) {
      Argparser::PrintUsage(1);
    } else if (!path_to_audio.empty()) {
      analyze_flag = true;
    } else if (!username.empty() & !password.empty() & !server.empty() & (!number.empty()|!path_to_numbers.empty())) {
      ParseNumbers();
      wardial_flag = true;
    } else {
      Argparser::PrintUsage(0);
      throw "Wrong Usage!";
    }
  }

  catch (std::exception &e) {
    if (vm.count("help")) {
          Argparser::PrintUsage(1);
    } else {
        Argparser::PrintUsage(0);
    }
    Logger::GetLogger()->Log("Error in argparse: " + std::string(e.what()), LOG_LVL_ERROR);
    throw "Wrong usage!";
  }
  catch (const char* msg) {
    throw msg;
  }
}

void Argparser::ParseNumbers() {
  // extract numbers from file
  if (!path_to_numbers.empty()) {
    std::ifstream numbers_file(path_to_numbers);
    std::string file_content((std::istreambuf_iterator<char>(numbers_file)),
                              (std::istreambuf_iterator<char>()));
    numbers_file.close();
    if (numbers_file.fail()) {
      Logger::GetLogger()->Log("Something terrible happened to the numbers file! :(", LOG_LVL_FATAL);
    }

    if (!number.empty()) {
      number.append(",");
    }
    number.append(file_content);
//    std::cout << number;
  }
  // remove all newline characters
  std::size_t i = 0;
  while ( i < number.length() ) {
    i = number.find('\n', i);
    if (i == std::string::npos) {
      break;
    }
    number.erase(i);
  }

  // store all numbers from the -n option in numbers vector
  std::size_t pos = number.find(',');
  std::size_t pos_o = 0;
  std::size_t pos_h;
  if (!number.empty()) {
    if (pos != std::string::npos) {
      while (pos != std::string::npos) {
       // check if there is a hyphen, because then it is a range
        pos_h = number.substr(pos_o, pos-pos_o).find('-');
        if (pos_h != std::string::npos) {
          ParseNumberRange(number.substr(pos_o, pos-pos_o), pos_h);
        } else {
         numbers.push_back(number.substr(pos_o, pos-pos_o));
        }
        pos_o = pos+1;
        pos = number.find(',', pos_o);
      }
      pos_h = number.substr(pos_o).find('-');
      if (pos_h != std::string::npos) {
        ParseNumberRange(number.substr(pos_o), pos_h);
      } else {
        numbers.push_back(number.substr(pos_o));
      }
    } else {
      pos_h = number.find('-');
      if (pos_h != std::string::npos) {
          ParseNumberRange(number, pos_h);
        } else {
         numbers.push_back(number);
        }
      }
  }
}

void Argparser::ParseNumberRange(std::string number_range, size_t pos) {
  // store number ranges defined with '-' in the numbers vector
  int64_t number1 = std::stoll(number_range.substr(0, pos));
  int64_t number2 = std::stoll(number_range.substr(pos+1));
  int64_t max = number1 > number2 ? number1:number2;
  int64_t min = number1 < number2 ? number1:number2;
  for (int64_t i = min; i <= max; i++) {
    std::string num = std::to_string(i);
    while (num.length() < number_range.substr(0, pos).length()) {
      num.insert(0, "0");
    }
    numbers.push_back(num);
  }
}

void Argparser::PrintUsage(bool help) {
    std::cout << "swd is a simple wardialer\n" << std::endl;
    std::cout << "USAGE:" << std::endl;
    std::cout << "Wardialing:\n"
      << "swd -u USERNAME -p PASSWORD -s URI -n NUMBER" << "\n\n";
    std::cout << "Analyse an audio file to determine if the sounds were produced by a modem:\n"
     << "swd -a PATHTOFILE" << "\n\n";
    if (help) {
          std::cout << desc << "\n";
    } else {
        std::cout << "Try 'swd --help' for more information." << std::endl;
    }
}

bool Argparser::DoAnalyse() {
  return this->analyze_flag;
}

bool Argparser::DoWardial() {
  return this->wardial_flag;
}

std::string Argparser::GetPathToAudio() {
  return this->path_to_audio;
}

std::string Argparser::GetUsername() {
  return this->username;
}

std::string Argparser::GetPassword() {
  return this->password;
}

int Argparser::GetThreads() {
  return this->threads;
}

std::string Argparser::GetServer() {
  return this->server;
}

std::vector<std::string> Argparser::GetNumbers() {
  return this->numbers;
}

bool Argparser::GetDebugStatus() {
  return this->debug;
}
