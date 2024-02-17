# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++11 -Wall -Wextra

# Source files
SRCS := smash.cpp signals.cpp commands.cpp

# Header files
HDRS := signals.h commands.h

# Object files
OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := smash

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to build object files from source files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Phony target to clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Default target (smash) is phony
.PHONY: all
all: $(TARGET)
