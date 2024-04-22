all:
	g++ -std=c++20 -g main.cpp source/karnaugh.cpp -I"./include/" -o main

clean:
	rm -rf main
	