compile: main.cpp src/*.cpp include/*.h
	c++ -O3 -Wall -std=c++11 main.cpp src/*.cpp -I include/ -lboost_program_options -lpthread -o main

debug: main.cpp src/*.cpp include/*.h
	c++ -g -std=c++11 main.cpp src/*.cpp -I include/ -lboost_program_options -lpthread -o main

profile: main.cpp src/*.cpp include/*.h
	c++ -O3 -pg -std=c++11 main.cpp src/*.cpp -I include/ -lboost_program_options -lpthread -o main