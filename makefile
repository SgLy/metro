CXX = g++
DEBUG_FLAG = -g3 -Wall
RELEASE_FLAG = -O2

release: main.cpp
	$(CXX) main.cpp -o main $(RELEASE_FLAG)

debug: main.cpp
	$(CXX) main.cpp -o main $(DEBUG_FLAG)

clean:
	rm main
