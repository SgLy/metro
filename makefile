CXX = g++
DEBUG_FLAG = -g3 -Wall
RELEASE_FLAG = -O2

debug: main.cpp
	$(CXX) main.cpp -o main $(DEBUG_FLAG)

release: main.cpp
	$(CXX) main.cpp -o main $(RELEASE_FLAG)

clean:
	rm main
