MKDIR = mkdir -p
CXX = g++
CXXFLAGS = -std=c++14 $(INCLUDE)
INCLUDE = -Iinclude/
LDLIBS = -lpng -lX11 -lGL -lGLEW

BIN_DIR = bin/
TEST_DIR = tests/
TEST_BIN_DIR = bin/tests/

TESTS = $(wildcard $(TEST_DIR)*/*.cpp)
TEST_OUTPUTS = $(TESTS:.cpp=.test)
#TEST_OUTPUTS = $(TESTS:.cpp=)

$(BIN_DIR):
	 $(MKDIR) $(BIN_DIR)

$(TEST_BIN_DIR): $(BIN_DIR)
	$(MKDIR) $(TEST_BIN_DIR)

#$(TEST_DIR)/%/% : test/%.cpp $(PROJS_BIN_DIR)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDLIBS) -o $(BIN_DIR)$@ $<

%.test : $(TEST_BIN_DIR)
	$(CXX) $(CXXFLAGS) $(LDLIBS) -o $(TEST_BIN_DIR)$(notdir $(basename $@)) $(basename $@).cpp

all_tests : $(TEST_OUTPUTS) $(TEST_BIN_DIR)
#	$(CXX) $(CXXFLAGS) $(LDLIBS) -o $(TEST_BIN_DIR)$(notdir $<) $<.cpp
#	$(info $$TEST_OUTPUTS is [${TEST_OUTPUTS}])

all: all_tests run_tests

run_tests:
	run-parts $(TEST_BIN_DIR)

clean:
	rm -drf $(BIN_DIR) $(TEST_BIN_DIR)
