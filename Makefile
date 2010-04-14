
CFLAGS= -O2 -ffast-math
CFLAGS+= -lm -lglut `sdl-config --cflags --libs`

  # Special Jarlsberg Options
#CFLAGS+= -DDEBUG_MODE
CFLAGS+= -DCOLORFUL_MODE
#CFLAGS+= -DINVERT_CUBE

all: jarlsberg

test: test.cpp
	g++ $(CFLAGS) -o Test test.cpp

jarlsberg: main.cpp
	g++ $(CFLAGS) -o Jarlsberg main.cpp

%.o: %.cpp
	g++ $(CFLAGS) -c $< -o $@

