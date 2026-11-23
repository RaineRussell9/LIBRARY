CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

SRC = main.cpp Date.cpp Storage.cpp Library.cpp
OBJ = $(SRC:.cpp=.o)

library: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

%.o: %.cpp $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) library library.exe
