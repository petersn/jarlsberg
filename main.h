// Main header for Jarlsberg in C++

#ifndef _MAIN_JARLSBERG_HEADER_
#define _MAIN_JARLSBERG_HEADER_

#include "surfaces.h"

extern GLfloat default_emission[];

int LoadTexture(const char *path, bool user_accesible = true);

void extrude_game_geometry( int zunits, bool do_optimize = true );

extern const double CONST_physics_goeyness;
extern const double CONST_physics_goo_exponent;

  // Global variables
struct camera_t {
	GLfloat x, y, z;
	vector<double> momentum;
	GLfloat facing, tilt;
	int floored;

	  // New angles are mixed with old angles to smooth out low FPS mice
	GLfloat facing_target, tilt_target;

	  // Set to 0.0 for the camera to be as sluggish as possible, and 1.0 for it to be as snappy as possible
#define CAMERA_SMOOTHING_FACTOR 0.5

};

class texture_entry_t {
public:
	GLuint texture_index;
	GLfloat emission[4];
	double scale_factor;
	double damage_per_frame;
	double victory_points;
	bool extrudable;

	texture_entry_t() {
		texture_index = 0;
		emission = { 0.8f, 0.8f, 0.8f, 1.0f };
		scale_factor = 1.0;
		damage_per_frame = 0.0;
		victory_points = 0.0;
		extrudable = true;
	}

	void set_emission(GLfloat* e) {
		for (int ii=0; ii<4; ii++) {
			emission[ii] = e[ii];
		}
	}

};

extern texture_entry_t* global_texture_table;
extern int global_texture_index;

enum game_mode_t {
	GAME_EDITING,
	GAME_PLAYING,
};

extern game_mode_t game_mode;
extern double hit_points;
extern double victory_total;

void reload_level_geometry(void);
void restart_level(void);
void load_next_level(void);

#endif

