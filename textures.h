// C++ textures header

#ifndef _TEXTURES_HEADER_
#define _TEXTURES_HEADER_

#include "main.h"

struct Texture {
	GLuint official_number;

	Texture( const char* path ) {
		official_number = LoadTexture( path );
	}

};

#endif

