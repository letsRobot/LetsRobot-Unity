all: robot

bin:
	mkdir -p bin

robot: bin/Robot
bin/Robot: bin Pi/Robot/*.cpp Pi/Robot/*.h Lib/*.cpp Lib/*.h
	g++ -std=c++0x -Wall -pthread -I Lib -o bin/Robot -O3 Pi/Robot/*.cpp Lib/*.cpp

clean:
	rm -f bin/Robot

anyway: clean robot
