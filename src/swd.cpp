// Copyright 2020 Barger M., Knoll M., Kofler L.
#include "swd.hpp"

int main(int argc, char *argv[]) {
  // Parse CLI Arguments
  Argparser* args = Argparser::GetArgparser();

  // Sample call of db client
  DBClient db = DBClient("wardialing.db");
  try {
      args->Parse(argc, argv);
      if (args->DoWardial()) {
        Wardialer();
        return 0;
      } else if (args->DoAnalyse()) {
        Wav wav;

        if (wav.Read(args->GetPathToAudio())) {
          AudioAnalyzer audio_analyzer;
          audio_analyzer.Analyze(&wav);

          Logger::GetLogger()->Log(audio_analyzer.GetReadableLineType() + " detected in file " + args->GetPathToAudio(),
              LOG_LVL_STATUS);
        }
      }
  } catch(const char* msg) {
    std::cerr << msg << std::endl;
    return 1;
  }
  return 0;
}
