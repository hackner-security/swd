CXX := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -g -O2 -fstack-protector -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now
CPPLINT_FLAGS := --linelength=120
BIN := bin
SRC := src
INCLUDE := include
TEST := tests
LIB := lib
BOOST_LIBRARIES := -lboost_program_options
EXOSIP_LIBRARIES := -losip2 -leXosip2 -losipparser2
KISS_LIBRARIES := -lm -lkissfft -I lib/kissfft
KISS_SOURCE := $(LIB)/kissfft/tools/kiss_fftr.c
RTP_LIBRARIES := -lortp -lbctoolbox
SQL_LIBRARIES := -lsqlite3
OTHER_LIBRARIES = -lpthread
LIBRARIES = $(BOOST_LIBRARIES) $(EXOSIP_LIBRARIES) $(KISS_LIBRARIES) $(RTP_LIBRARIES) $(SQL_LIBRARIES) $(OTHER_LIBRARIES)
EXECUTABLE := swd
CXX_TESTGEN_FLAGS := --error-printer
CXX_TESTGEN := cxxtestgen

all: $(BIN)/$(EXECUTABLE)

run:
	./$(BIN)/$(EXECUTABLE) $(ARG)

brun: clean all
	clear
	@echo "Executing ..."
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp $(KISS_SOURCE)
	@echo "Building ..."
	-mkdir -p bin
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES) -Wl,--as-needed

clean:
	@echo "Clearing ..."
	-rm $(BIN)/*

check:
	@echo "Checking Coding Style ..."
	-cpplint $(CPPLINT_FLAGS) $(SRC)/*.cpp
	-cpplint $(CPPLINT_FLAGS) $(INCLUDE)/*.hpp

test:
	@echo "Running tests ..."
	$(CXX_TESTGEN) $(CXX_TESTGEN_FLAGS) -o $(TEST)/audio_analyzer_test.cpp $(TEST)/audio_analyzer_test.h
	$(CXX) -o $(TEST)/test_runner -I $(INCLUDE) -L $(KISS_LIBRARIES) $(TEST)/audio_analyzer_test.cpp $(SRC)/audio_analyzer.cpp $(SRC)/wav.cpp $(SRC)/log.cpp $(LIB)/kissfft/tools/kiss_fftr.c
	./$(TEST)/test_runner

