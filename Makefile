
CFLAGS= -O2 -ffast-math -lm -lglut `sdl-config --cflags --libs`

all: jarlsberg

test: test.cpp
	g++ $(CFLAGS) -o Test test.cpp

jarlsberg: main.cpp
	g++ $(CFLAGS) -o Jarlsberg main.cpp

%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

