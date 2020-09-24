// Copyright 2020 Barger M., Knoll M., Kofler L.
#ifndef INCLUDE_SWD_HPP_
#define INCLUDE_SWD_HPP_

#include <unistd.h>
#include <iostream>
#include <iterator>
#include <boost/program_options.hpp>

#include "sip_client.hpp"
#include "argparse.hpp"
#include "log.hpp"
#include "wav.hpp"
#include "db_client.hpp"
#include "audio_analyzer.hpp"
#include "wardialer.hpp"

boost::program_options::variables_map vm;

int argparse(int argc, char *argv[]);

#endif  //  INCLUDE_SWD_HPP_
