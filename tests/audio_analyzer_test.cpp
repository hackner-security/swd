/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_AudioAnalyzerTest_init = false;
#include "/home/pro4/wardialing/tests/audio_analyzer_test.h"

static AudioAnalyzerTest suite_AudioAnalyzerTest;

static CxxTest::List Tests_AudioAnalyzerTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_AudioAnalyzerTest( "tests/audio_analyzer_test.h", 5, "AudioAnalyzerTest", suite_AudioAnalyzerTest, Tests_AudioAnalyzerTest );

static class TestDescription_suite_AudioAnalyzerTest_test_linetype_modem1 : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AudioAnalyzerTest_test_linetype_modem1() : CxxTest::RealTestDescription( Tests_AudioAnalyzerTest, suiteDescription_AudioAnalyzerTest, 8, "test_linetype_modem1" ) {}
 void runTest() { suite_AudioAnalyzerTest.test_linetype_modem1(); }
} testDescription_suite_AudioAnalyzerTest_test_linetype_modem1;

static class TestDescription_suite_AudioAnalyzerTest_test_linetype_modem2 : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AudioAnalyzerTest_test_linetype_modem2() : CxxTest::RealTestDescription( Tests_AudioAnalyzerTest, suiteDescription_AudioAnalyzerTest, 20, "test_linetype_modem2" ) {}
 void runTest() { suite_AudioAnalyzerTest.test_linetype_modem2(); }
} testDescription_suite_AudioAnalyzerTest_test_linetype_modem2;

static class TestDescription_suite_AudioAnalyzerTest_test_linetype_fax : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AudioAnalyzerTest_test_linetype_fax() : CxxTest::RealTestDescription( Tests_AudioAnalyzerTest, suiteDescription_AudioAnalyzerTest, 32, "test_linetype_fax" ) {}
 void runTest() { suite_AudioAnalyzerTest.test_linetype_fax(); }
} testDescription_suite_AudioAnalyzerTest_test_linetype_fax;

static class TestDescription_suite_AudioAnalyzerTest_test_linetype_other1 : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AudioAnalyzerTest_test_linetype_other1() : CxxTest::RealTestDescription( Tests_AudioAnalyzerTest, suiteDescription_AudioAnalyzerTest, 44, "test_linetype_other1" ) {}
 void runTest() { suite_AudioAnalyzerTest.test_linetype_other1(); }
} testDescription_suite_AudioAnalyzerTest_test_linetype_other1;

static class TestDescription_suite_AudioAnalyzerTest_test_linetype_other2 : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AudioAnalyzerTest_test_linetype_other2() : CxxTest::RealTestDescription( Tests_AudioAnalyzerTest, suiteDescription_AudioAnalyzerTest, 56, "test_linetype_other2" ) {}
 void runTest() { suite_AudioAnalyzerTest.test_linetype_other2(); }
} testDescription_suite_AudioAnalyzerTest_test_linetype_other2;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
