
Compilation on UNIX
-------------------

The makefile works, but the minimum required is basically something like:

	g++ -lglut -lSDL main.cpp

But you may as well add in options like -O2, and -ffast-math.

Ubuntu (or Debian) packages
---------------------------

From a fresh vanilla Ubuntu install (last tested on 10.04, x86_64) the only packages you should need are:

	build-essential libsdl1.2-dev libglut3-dev

Compilation on Windows
----------------------

A project file for Visual C++ 2010 Distress is included.

Before you compile, make sure you have the glut and SDL headers, dlls, and object files.
You can get these at: 

Generic Windows compilation
---------------------------

Link against:

	opengl32.lib glu32.lib glut32.lib sdl.lib sdlmain.lib

Make sure all the following headers are available:

	SDL.h gl.h glu.h glut.h 

