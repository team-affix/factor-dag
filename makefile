SOURCE = main.cpp dag_logic.cpp
INCLUDES = -I"./include/" -I"digital-logic/include/"

all:
	g++ -std=c++20 -g $(SOURCE) $(INCLUDES) -o main

clean:
	rm -rf main
	