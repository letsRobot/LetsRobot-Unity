all: robot

bin:
	mkdir -p bin

robot: bin/Robot
bin/Robot: bin
	g++-4.7 -std=c++0x -Wall -pthread -I Lib -o bin/Robot -O3 Pi/Robot/*.cpp Lib/*.cpp

clean:
	rm -f bin/Robot

anyway: clean robot
