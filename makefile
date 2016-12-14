CXX = g++
DEBUG_FLAG = -g3 -Wall
RELEASE_FLAG = -O2

debug: metro.cpp
	$(CXX) metro.cpp -o metro $(DEBUG_FLAG)

release: metro.cpp
	$(CXX) metro.cpp -o metro $(RELEASE_FLAG)

clean:
	rm metro
