// Main header for Jarlsberg in C++

#ifndef _MAIN_JARLSBERG_HEADER_
#define _MAIN_JARLSBERG_HEADER_

extern GLfloat default_emission[];

int LoadTexture(const char *path, bool user_accesible = true);

void extrude_game_geometry( int zunits, bool do_optimize = true );

extern const double CONST_physics_goeyness;
extern const double CONST_physics_goo_exponent;

class texture_entry_t {
public:
	GLuint texture_index;
	GLfloat emission[4];
	double scale_factor;

	texture_entry_t() {
		texture_index = 0;
		emission = { 0.8f, 0.8f, 0.8f, 1.0f };
		scale_factor = 1.0;
	}

	void set_emission(GLfloat* e) {
		for (int ii=0; ii<4; ii++) {
			emission[ii] = e[ii];
		}
	}

};

extern texture_entry_t* global_texture_table;
extern int global_texture_index;

#endif

