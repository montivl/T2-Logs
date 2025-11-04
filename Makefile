CXX := g++
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -pedantic -Iinclude

all: test_recent test_frequency test_extra

test_recent: tests/test_recent.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test_frequency: tests/test_frequency.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test_extra: tests/test_extra.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f test_recent test_frequency
