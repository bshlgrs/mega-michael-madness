CC = clang++
FLAGS = --std=c++11 -Wall
FILES = Tools.cpp Distribution.cpp QuantitativeModel.cpp

# Make for debugging
all:
	$(CC) $(FLAGS) -g -O0 $(FILES)

# Make for high performance
perf:
	$(CC) $(FLAGS) -O3 $(FILES)
