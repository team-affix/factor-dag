SOURCE = main.cpp factor.cpp
INCLUDE = -I"./include/" -I"digital-logic/include/"

all:
	g++ -std=c++20 -g $(SOURCE) $(INCLUDE) -o main

clean:
	rm -rf main
	