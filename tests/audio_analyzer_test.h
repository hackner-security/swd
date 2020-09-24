#include <cxxtest/TestSuite.h>
#include <wav.hpp>
#include <audio_analyzer.hpp>

class AudioAnalyzerTest : public CxxTest::TestSuite {
 public:
  
  void test_linetype_modem1 () {
    Wav wav;
    wav.Read("tests/audios/modem1_short.wav");

    AudioAnalyzer audio_analyzer;
    audio_analyzer.Analyze(&wav);

    LineType line_type = audio_analyzer.GetLineType();

    TS_ASSERT_EQUALS(line_type, MODEM);
  }

  void test_linetype_modem2 () {
    Wav wav;
    wav.Read("tests/audios/modem2_short.wav");

    AudioAnalyzer audio_analyzer;
    audio_analyzer.Analyze(&wav);

    LineType line_type = audio_analyzer.GetLineType();

    TS_ASSERT_EQUALS(line_type, MODEM);
  }

  void test_linetype_fax () {
    Wav wav;
    wav.Read("tests/audios/fax.wav");

    AudioAnalyzer audio_analyzer;
    audio_analyzer.Analyze(&wav);

    LineType line_type = audio_analyzer.GetLineType();

    TS_ASSERT_EQUALS(line_type, FAX);
  }

  void test_linetype_other1 () {
    Wav wav;
    wav.Read("tests/audios/music1.wav");

    AudioAnalyzer audio_analyzer;
    audio_analyzer.Analyze(&wav);

    LineType line_type = audio_analyzer.GetLineType();

    TS_ASSERT_EQUALS(line_type, OTHER);
  }

  void test_linetype_other2 () {
    Wav wav;
    wav.Read("tests/audios/music2.wav");

    AudioAnalyzer audio_analyzer;
    audio_analyzer.Analyze(&wav);

    LineType line_type = audio_analyzer.GetLineType();

    TS_ASSERT_EQUALS(line_type, OTHER);
  }
};
