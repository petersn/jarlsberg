 // Main Jarlsberg code
using namespace std;
#include <iostream>
#include <fstream>
#include <list>

#ifndef WINDOWS_BUILD

#include <SDL/SDL.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#else

#include <SDL.h>
#include <GL\glut.h>
#include <GL\gl.h>
#include <GL\glu.h>

#endif

#include <cmath>

  // Windows compatibility definitions
#ifndef M_PI
#define M_PI 3.1415926535897931
#endif

#ifndef INFINITY
#include <limits>
#define INFINITY (numeric_limits<double>::infinity())
#endif

#include "main.h"

#include "surfaces.h"
#include "surfaces.cpp"
#include "textures.h"

#define BPP 4
#define DEPTH 32

  // Game constants
const double CONST_movespeed			= 0.3;
const double CONST_player_width		 = 1.1;
const double CONST_player_height		= 4.4;
const double CONST_player_head		  = 0.6;
const double CONST_gravity			  = 0.02;
  //  The terminal velocity is chosen such that if you're falling,
  //  you see no obvious repeating patterns on adjacent walls.
  //  Any number whose LCM with 1 is great will do. (Rational LCM, not integer LCM.)
const double CONST_terminal_velocity	= 17.63;
const double CONST_jump_force		   = 1.3;
  //  Lower values for goeyness result in sludgier collision detection.
  //  Higher values make the world more rubbery. (Values >1 are a baaaaaad idea.)
const double CONST_physics_goeyness	 = 0.1;
  //  The formula for response to collision is (depth of collision ** goo_exponent * goeyness)
  //  Therefore, a low goo exponent prevents small objects from acting as ridiculous "player canon".
const double CONST_physics_goo_exponent = 0.5;
const double CONST_physics_mu_air	   = 0.99;
const double CONST_physics_mu_floor	 = 0.5;

const double CONST_move_increment	   = 0.5;

  // Global variables that are essentially static configuration
int config_WIDTH;
int config_HEIGHT;
SDL_Surface *screen;

texture_entry_t* global_texture_table;
int global_texture_index = 0;
int current_texture_on_brush = 0;

int skybox_texture;

  // Dynamic globals
list<surface> geometry;
list<surface> geometry_temp;
list<surface>::iterator iter_surf;

  // Make an array to store all currently held keys.
  // 1024 is simply a heursitic: codes for keys seem to be ASCII + small extensions beyond.
char keys_held[1024] = {0};
int mouse_x, mouse_y;

enum physics_mode_t {
	PHYSICS_FLY,
	PHYSICS_CONSTRAINED,
	PHYSICS_WALK,
};

enum game_mode_t {
	GAME_EDITING,
	GAME_PLAYING,
};

game_mode_t game_mode = GAME_EDITING;
physics_mode_t physics_mode = PHYSICS_WALK;

enum selection_state_t {
	SELECTION_NONE,
	SELECTION_SELECTING,
	SELECTION_DONE,
};

selection_state_t selection_state = SELECTION_NONE;
surface selection_plane;
vector<int> selection_start, selection_end;

  // Config
/* white ambient light at half intensity (rgba) */
#ifndef INVERT_CUBE
GLfloat LightAmbient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
#else
GLfloat LightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
#endif

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[]  = { 0.7f, 0.7f, 0.7f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };

GLfloat default_emission[] = {0.0, 0.0, 0.0, 1.0};

  // Global variables
struct camera_t {
	GLfloat x, y, z;
	vector<double> momentum;
	GLfloat facing, tilt;
	bool floored;

	  // New angles are mixed with old angles to smooth out low FPS mice
	GLfloat facing_target, tilt_target;

	  // Set to 0.0 for the camera to be as sluggish as possible, and 1.0 for it to be as snappy as possible
#define CAMERA_SMOOTHING_FACTOR 0.5

};

  // Default camera
camera_t camera = {0};

camera_t start_location = {0};

  // Include a small bit of code that does ray casting
#include "ray_cast.cpp"

void DrawScreen() {

	if (SDL_MUSTLOCK(screen)) {
		if(SDL_LockSurface(screen) < 0) return;
	}

	//SDL_FillRect( SDL_GetVideoSurface(), NULL, SDL_MapRGB(SDL_GetVideoSurface()->format,100,32,64) );

	  // Clear the screen and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	  // Reset the view
	glLoadIdentity();

	  // Set light position
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	  // Reposition the camera based on its position
	glRotatef( camera.tilt,   1.0f, 0.0f, 0.0f);
	glRotatef( camera.facing, 0.0f, 1.0f, 0.0f);
	glTranslatef( -camera.x, -camera.y, -camera.z );

#ifndef COLORFUL_MODE
	glColor3f(1.0f, 1.0f, 1.0f);
#endif

	int ii = 0;
	for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {
#ifdef COLORFUL_MODE
		if (ii%3 == 0)
			glColor3f(1.0f, 0.1f, 0.1f);
		if (ii%3 == 1)
			glColor3f(0.0f, 1.0f, 0.0f);
		if (ii%3 == 2)
			glColor3f(0.0f, 0.0f, 1.0f);
#endif
		render_surface( *iter_surf );
		ii++;
	}

	bool hit;
	vector<double> hit_point;

	  // Magically load up values into the variables
	raycast_from_camera( &hit, &hit_point );

	//hit_point = vector_recast<int, double>( vector_recast<double, int> ( hit_point - vector<double>(0.5,0.5,0.5) ) );
	//cout << hit << endl << hit_point << endl;

	if (hit) {
		glTranslatef(hit_point.x, hit_point.y, hit_point.z);
		glColor3f(0.5f,0.5f,0.5f);
		glutSolidSphere(0.1,20,20);

		//glPushMatrix();
		//glLoadIdentity();
		//LightPosition[0] = hit_point.x;
		//LightPosition[1] = hit_point.y;
		//LightPosition[2] = hit_point.z;
		//glPopMatrix();

		glTranslatef(-hit_point.x, -hit_point.y, -hit_point.z);
	}

	if (selection_state == SELECTION_SELECTING || selection_state == SELECTION_DONE) {

		hit = true;

		matrix<double> globalize = matrix_recast<int, double>( selection_plane.globalize_basis );
		vector<double> temp = globalize * vector_recast<int, double>(selection_start);

		if (selection_state == SELECTION_SELECTING) {
			surface *new_selection_plane;
			  // Magically load up values into the variables
			raycast_from_camera( &hit, NULL, &selection_end, &new_selection_plane );
			if (hit && (!coplanar(selection_plane, *new_selection_plane))) hit = false;
		}

		if (hit) {
			temp = temp + vector_recast<int, double>(selection_plane.localize_basis.row2)*0.1;
			if (current_texture_on_brush == global_texture_index) {
				glBindTexture(GL_TEXTURE_2D, skybox_texture);
			} else {
				glBindTexture(GL_TEXTURE_2D, global_texture_table[current_texture_on_brush].texture_index);
			}
			GLfloat glowing[] = {1.0, 1.0, 1.0, 1.0};
			glMaterialfv(GL_FRONT, GL_EMISSION, glowing);
			glColor3f(2.0f,2.0f,2.0f);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp + globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
				glTexCoord2f(1.0, 0.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp + globalize * vector<double>( 0, (selection_end.y-selection_start.y), 0 );
				glTexCoord2f(1.0, 1.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp - globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
				glTexCoord2f(0.0, 1.0);
				glVertex3f(temp.x, temp.y, temp.z);

				// Draw the same rectangle in reverse order
				glTexCoord2f(0.0, 0.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp + globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
				glTexCoord2f(0.0, 1.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp - globalize * vector<double>( 0, (selection_end.y-selection_start.y), 0 );
				glTexCoord2f(1.0, 1.0);
				glVertex3f(temp.x, temp.y, temp.z);
				temp = temp - globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
				glTexCoord2f(1.0, 0.0);
				glVertex3f(temp.x, temp.y, temp.z);
			glEnd();
		}
	}

	SDL_GL_SwapBuffers();

//	SDL_BlitSurface(text, &src, screen, &dest);

	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_Flip(screen);

}

SDL_Surface *LoadBitmap( const char *path ) {
	SDL_Surface *image;
	SDL_Surface *temp;

	temp = SDL_LoadBMP(path);
	if (temp == NULL) {
		cerr << "Unable to load bitmap: " << SDL_GetError() << endl;
		return NULL;
	}

	image = SDL_DisplayFormat(temp);
	if (image == NULL) {
		cerr << "Unable to convert bitmap to current display format: " << SDL_GetError() << endl;
		return NULL;
	}
	SDL_FreeSurface(temp);

	return image;
}

int LoadTexture(const char *path, bool user_accesible) {
	GLuint texture;
	SDL_Surface* bmpFile;

	cout << "Loading texture: " << path << endl;

	  // Load the bitmap
	bmpFile = SDL_LoadBMP(path);

	if (bmpFile == NULL) {
		cerr << "Failed to load texture: " << path << endl;
		return -1;
	}

	  // Standard OpenGL texture creation code
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,minFilter);
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,magFilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	gluBuild2DMipmaps(GL_TEXTURE_2D,3,bmpFile->w, \
		bmpFile->h,GL_BGR_EXT, \
		GL_UNSIGNED_BYTE,bmpFile->pixels);

	  // Free the surface after using it
	SDL_FreeSurface(bmpFile);

	// Don't put internal surfaces into the global texture table
	if (!user_accesible) {
		return texture;
	}

	global_texture_index++;

	texture_entry_t* new_list = new texture_entry_t[global_texture_index];

	for (int ii=0; ii<global_texture_index-1; ii++) {
		new_list[ii] = global_texture_table[ii];
	}

	delete[] global_texture_table;
	global_texture_table = new_list;

	global_texture_table[global_texture_index-1].texture_index = texture;

	return global_texture_index-1;
}

void InitGL() {

//#ifndef INVERT_CUBE
//	glClearColor(0.0, 0.0, 0.0, 1.0);
//#else
	glClearColor(0.2, 0.2, 0.8, 1.0);
//#endif

	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	  // Enables Clearing Of The Depth Buffer
	glClearDepth(1.0);
	  // The Type Of Depth Test To Do
	glDepthFunc(GL_LESS);
	  // Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	  // Enable texture mapping
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);

	  // Reset The Projection Matrix
	glLoadIdentity();

	  // Calculate The Aspect Ratio Of The Window
	gluPerspective(60.0f, (GLfloat) config_WIDTH / (GLfloat) config_HEIGHT, 0.1f, 4000.0f);

	glMatrixMode(GL_MODELVIEW);

	  // Set up light 1
	glEnable(GL_LIGHTING);

	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

	GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
	glEnable ( GL_COLOR_MATERIAL ) ;

	glLightfv(GL_LIGHT0, GL_AMBIENT,  LightAmbient);  // Add lighting -- Ambient
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  LightDiffuse);  // Add lighting -- Diffuse
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition); // Set light position
	glEnable(GL_LIGHT0);							  // Turn light 1 on

//	glMaterialfv(GL_FRONT, GL_AMBIENT,   ambient);
//	glMaterialfv(GL_FRONT, GL_DIFFUSE,   diffuse);
//	glMaterialfv(GL_FRONT, GL_SPECULAR,  specular);
//	glMaterialfv(GL_FRONT, GL_EMISSION,  emission);
//	glMaterialf (GL_FRONT, GL_SHININESS, shininess);
}

void extrude_game_geometry( int zunits, bool do_optimize ) {
	if (selection_state == SELECTION_DONE) {
		surface_local( selection_plane, selection_plane,
					   selection_start.x, selection_start.y,
					   selection_end.x-selection_start.x,
					   selection_end.y-selection_start.y );

		  // Clear out the temporary list
		geometry_temp.clear();

		list<surface> new_list;

		  // Iterate through each surface, and add its associated extruded representation
		for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {
			new_list = extrude( *iter_surf, selection_plane, zunits );
			geometry_temp.insert( geometry_temp.end(), new_list.begin(), new_list.end() );
		}

		geometry_temp = prune( geometry_temp );

		if (do_optimize) {
			geometry = optimize( geometry_temp );
		} else {
			geometry = geometry_temp;
		}

		selection_start = selection_start + vector<int>(0,0,zunits);
		selection_end = selection_end + vector<int>(0,0,zunits);
		selection_plane.local_position.z += zunits;
		selection_plane.position = selection_plane.globalize_basis * selection_plane.local_position;
		//cout << "Total surfaces: " << geometry.size() << endl;

	}
}

void move_player( vector<double> dxyz ) {

	if (physics_mode == PHYSICS_CONSTRAINED || physics_mode == PHYSICS_WALK) {
		while (~dxyz > CONST_move_increment) {
			vector<double>unit = (dxyz * CONST_move_increment) / (~dxyz);

			camera.x += unit.x;
			camera.y += unit.y;
			camera.z += unit.z;
			vector<double>new_xyz = limit( geometry, vector<double>(camera.x, camera.y, camera.z), camera.momentum, CONST_player_width, CONST_player_height, CONST_player_head, &camera.floored );
			//if (floored && camera.momentum < 0)
				//camera.momentum = 0.0;

			dxyz = dxyz - unit;

			camera.x = new_xyz.x;
			camera.y = new_xyz.y;
			camera.z = new_xyz.z;
		}
	}

	camera.x += dxyz.x;
	camera.y += dxyz.y;
	camera.z += dxyz.z;

	if (physics_mode == PHYSICS_CONSTRAINED || physics_mode == PHYSICS_WALK) {
		vector<double>new_xyz = limit( geometry, vector<double>(camera.x, camera.y, camera.z), camera.momentum, CONST_player_width, CONST_player_height, CONST_player_head, &camera.floored );
		//if (floored && camera.momentum < 0)
			//camera.momentum = 0.0;
		camera.x = new_xyz.x;
		camera.y = new_xyz.y;
		camera.z = new_xyz.z;
	}
}

int main(int argc, char **argv) {

	const SDL_VideoInfo *info;
	bool game_continue;
	int ii;
	double cos_angle, sin_angle;
	SDL_Event event;
	  // Flags to pass to SDL_SetVideoMode
	int videoFlags;

#ifndef INVERT_CUBE
	geometry.push_back( surface(vector<int>(20,-5,-20), 40, 40, vector<int>(-1,0,0), vector<int>(0,0,1)) );
	geometry.push_back( surface(vector<int>(20,-5,-20), 20, 40, vector<int>(0,1,0), vector<int>(-1,0,0)) );
	geometry.push_back( surface(vector<int>(20,-5,-20), 40, 20, vector<int>(0,0,1), vector<int>(0,1,0)) );
	geometry.push_back( surface(vector<int>(-20,-5,20), 20, 40, vector<int>(0,1,0), vector<int>(1,0,0)) );
	geometry.push_back( surface(vector<int>(-20,-5,20), 40, 20, vector<int>(0,0,-1), vector<int>(0,1,0)) );
	geometry.push_back( surface(vector<int>(20,15,-20), 40, 40, vector<int>(0,0,1), vector<int>(-1,0,0)) );
#else
	geometry.push_back( surface(vector<int>(20,-5-30,-20), 40, 40, vector<int>(0,0,1), vector<int>(-1,0,0)) );
	geometry.push_back( surface(vector<int>(20,-5-30,-20), 40, 20, vector<int>(-1,0,0), vector<int>(0,1,0)) );
	geometry.push_back( surface(vector<int>(20,-5-30,-20), 20, 40, vector<int>(0,1,0), vector<int>(0,0,1)) );
	geometry.push_back( surface(vector<int>(-20,-5-30,20), 40, 20, vector<int>(1,0,0), vector<int>(0,1,0)) );
	geometry.push_back( surface(vector<int>(-20,-5-30,20), 20, 40, vector<int>(0,1,0), vector<int>(0,0,-1)) );
	geometry.push_back( surface(vector<int>(20,15-30,-20), 40, 40, vector<int>(-1,0,0), vector<int>(0,0,1)) );
#endif

	  // Init glut just for spheres
	  // I hope to remove this later
	glutInit(&argc, argv);  

	  // Init video
	if ( (ii = SDL_Init(SDL_INIT_VIDEO)) < 0 ) {
		cerr << "SDL_Init failed: " << SDL_GetError() << endl;
		return 1;
	}

	  // Get screen width and height information
	info = SDL_GetVideoInfo();
	if (info == NULL) {
		cerr << "Unable to get video info: " << SDL_GetError() << endl;
		return 1;
	}
	config_WIDTH  = info->current_w;
	config_HEIGHT = info->current_h;

	/* the flags to pass to SDL_SetVideoMode */
	videoFlags  = SDL_OPENGL;		  /* Enable OpenGL in SDL */
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
	videoFlags |= SDL_HWPALETTE;	   /* Store the palette in hardware */
	videoFlags |= SDL_RESIZABLE;	   /* Enable window resizing */
	videoFlags |= SDL_FULLSCREEN;

	  // Old flags: SDL_FULLSCREEN | SDL_HWSURFACE | SDL_OPENGL

	/* This checks to see if surfaces can be stored in memory */
	if ( info->hw_available )
		videoFlags |= SDL_HWSURFACE;
	else
		videoFlags |= SDL_SWSURFACE;

	/* This checks if hardware blits can be done */
	if ( info->blit_hw )
		videoFlags |= SDL_HWACCEL;

	/* Sets up OpenGL double buffering */
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	cout << "Maximum resolution identified as " << config_WIDTH << "x" << config_HEIGHT << endl;

	if (!(screen = SDL_SetVideoMode(config_WIDTH, config_HEIGHT, DEPTH, videoFlags))) {
		SDL_Quit();
		cerr << "SDL_SetVideoMode failed: " << SDL_GetError() << endl;
	}

	  // Hide the cursor
	SDL_ShowCursor(SDL_DISABLE);

	  // Initialize OpenGL
	InitGL();

	  // Load the default surface texture
	global_texture_table = new texture_entry_t[global_texture_index];
	GLfloat lava_emission[] = {0.8, 0.8, 0.8, 1.0};
#ifndef HARD_CODE
	skybox_texture = LoadTexture("Data/Skybox.bmp", false);
	LoadTexture("Data/Old.bmp");
	global_texture_table[global_texture_index-1].set_emission( default_emission );
	LoadTexture("Data/Lava.bmp");
	global_texture_table[global_texture_index-1].set_emission( lava_emission );
	global_texture_table[global_texture_index-1].scale_factor = 0.1;
	LoadTexture("Data/Grass.bmp");
	global_texture_table[global_texture_index-1].set_emission( default_emission );
	global_texture_table[global_texture_index-1].scale_factor = 0.1;
#else
	char* home_dir = getenv("HOME");
	char* str_block = (char *) malloc(1024); // Big, for reaaaally long usernames
	snprintf(str_block, 1024, "%s/.jarlsberg/Data/Old.bmp", home_dir);
	LoadTexture(str_block);
	snprintf(str_block, 1024, "%s/.jarlsberg/Data/Lava.bmp", home_dir);
	LoadTexture(str_block, lava_emission);
	free(str_block);
#endif

	game_continue = true;

	while (game_continue) {
		DrawScreen();

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					game_continue = false;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						game_continue = false;
					}

					  // Make sure I was right about my heuristic (see above where keys_held is defined)
					if (event.key.keysym.sym >= 0 && event.key.keysym.sym < 1024)
						keys_held[event.key.keysym.sym] = 1;
					else
						cerr << "keysym out of range: " << event.key.keysym.sym << endl;

					if (event.key.keysym.sym == SDLK_SPACE && game_mode == GAME_EDITING) {
						ofstream myfile;
						myfile.open("map.jb_geo");
						dump_to_file( myfile, geometry );
						myfile.close();
					}
					else if (event.key.keysym.sym == SDLK_BACKSPACE) {
						ifstream myfile;
						myfile.open("map.jb_geo", ios::in|ios::binary);
						geometry = load_from_file( myfile );
						myfile.close();
					}
					else if (event.key.keysym.sym == 'e' && game_mode == GAME_EDITING) {
						physics_mode = PHYSICS_FLY;
					}
					else if (event.key.keysym.sym == 'w' && game_mode == GAME_EDITING) {
						physics_mode = PHYSICS_WALK;
					}
					else if (event.key.keysym.sym == 'c' && game_mode == GAME_EDITING) {
						physics_mode = PHYSICS_CONSTRAINED;
					}
					else if (event.key.keysym.sym == 't' && game_mode == GAME_EDITING) {
						bool hit;
						surface *surf_ptr;
						raycast_from_camera( &hit, NULL, NULL, &surf_ptr );
						if (hit) {
							// >, not >=, because surface_texture_index == global_texture_index is skybox
							if (++surf_ptr->surface_texture_index > global_texture_index) {
								surf_ptr->surface_texture_index = 0;
							}
						}
					}
					else if (event.key.keysym.sym == 'r' && game_mode == GAME_EDITING) {
						bool hit;
						surface *surf_ptr;
						raycast_from_camera( &hit, NULL, NULL, &surf_ptr );
						if (hit) {
							if (--surf_ptr->surface_texture_index < 0) {
								surf_ptr->surface_texture_index = global_texture_index;
							}
						}
					}
					else if (event.key.keysym.sym == 'g' && game_mode == GAME_EDITING) {
						// >, not >=, because surface_texture_index == global_texture_index is skybox
						if (++current_texture_on_brush > global_texture_index)
							current_texture_on_brush = 0;
					}
					else if (event.key.keysym.sym == 'f' && game_mode == GAME_EDITING) {
						if (--current_texture_on_brush < 0)
							current_texture_on_brush = global_texture_index;
					}
					else if (event.key.keysym.sym == 'p' && game_mode == GAME_EDITING) {
						if (selection_state == SELECTION_DONE) {
							surface_local( selection_plane, selection_plane,
									selection_start.x, selection_start.y,
									selection_end.x-selection_start.x,
									selection_end.y-selection_start.y );

							// Start by extruding by zero -- this has the effect of separating out the selected region
							extrude_game_geometry(0, false);

							// Next, paint every surface that overlaps the selection
							// Because of the previous operation, those surfaces that overlap exactly fill the selected area
							for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {
								if (overlaps( selection_plane, *iter_surf )) {
									(*iter_surf).surface_texture_index = current_texture_on_brush;
								}
							}
							// Finally, optimize because we skipped this operation previously
							geometry = optimize( geometry );
						}
					}
					else if (event.key.keysym.sym == 's' && game_mode == GAME_EDITING) {
						start_location = camera;
					}
					else if (event.key.keysym.sym == 'q') {
						if (game_mode == GAME_EDITING) {
							physics_mode = PHYSICS_WALK;
							camera = start_location;
							current_texture_on_brush = 0;
							game_mode = GAME_PLAYING;
						}
						else if (game_mode == GAME_PLAYING) {
							game_mode = GAME_EDITING;
						}
					}

					break;
			   case SDL_KEYUP:
					if (event.key.keysym.sym >= 0 && event.key.keysym.sym < 1024)
						keys_held[event.key.keysym.sym] = 0;
					else
						cerr << "keysym out of range: " << event.key.keysym.sym << endl;
					break;
			   case SDL_MOUSEMOTION:
					mouse_x = event.motion.x;
					mouse_y = event.motion.y;
					break;
			   case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						bool hit;
						surface *surf_ptr;

						  // Magically load up values into the variables
						raycast_from_camera( &hit, NULL, &selection_start, &surf_ptr );

						  // If the test hit a surface, then copy it over
						if (hit) {
							selection_plane = *surf_ptr;
							selection_state = SELECTION_SELECTING;
						} else {
							selection_state = SELECTION_NONE;
						}
					}
					if (event.button.button == SDL_BUTTON_RIGHT) {
						if (physics_mode == PHYSICS_WALK) {
							camera.momentum.y += CONST_jump_force;
						}
					}
					if (event.button.button == SDL_BUTTON_WHEELUP) {
						extrude_game_geometry(-1);
					}
					if (event.button.button == SDL_BUTTON_WHEELDOWN) {
						extrude_game_geometry(1);
					}
					break;
			   case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT) {

						bool hit;
						surface *new_selection_plane;
						vector<double> hit_point;
						vector<int> local_point;

						  // Magically load up values into the variables
						raycast_from_camera( &hit, &hit_point, &local_point, &new_selection_plane );

						if (hit && coplanar(selection_plane, *new_selection_plane)) {
							selection_end = local_point;
							selection_state = SELECTION_DONE;
						} else {
							selection_state = SELECTION_NONE;
						}
					}
					break;
			}
		}

		  // ========== Test for extrusion ========== //

		  // Test for specific keys
		if (keys_held[SDLK_PAGEUP] ^ keys_held[SDLK_PAGEDOWN]) {
			if (keys_held[SDLK_PAGEUP]) {
				extrude_game_geometry(1);
			}
			if (keys_held[SDLK_PAGEDOWN]) {
				extrude_game_geometry(-1);
			}
		}

		  // ========== Done ========== //

		SDL_WarpMouse(config_WIDTH/2, config_HEIGHT/2);

		camera.facing_target -= (config_WIDTH/2  - mouse_x)*0.1;
		camera.tilt_target   -= (config_HEIGHT/2 - mouse_y)*0.1;

		  // Mix together old camera angle and new camera angle
		camera.facing = (camera.facing * (1.0 - CAMERA_SMOOTHING_FACTOR)) + (camera.facing_target * CAMERA_SMOOTHING_FACTOR);
		camera.tilt   = (camera.tilt   * (1.0 - CAMERA_SMOOTHING_FACTOR)) + (camera.tilt_target   * CAMERA_SMOOTHING_FACTOR);

		  // Compute the required trig for moving the player
		cos_angle = CONST_movespeed * cos( camera.facing * (M_PI / 180.0) );
		sin_angle = CONST_movespeed * sin( camera.facing * (M_PI / 180.0) );

		if (physics_mode == PHYSICS_WALK) {
			camera.momentum.y -= CONST_gravity;
			if (camera.momentum.y < -CONST_terminal_velocity)
				camera.momentum.y = -CONST_terminal_velocity;

			// Slow down the player, then move them based on their momentum
			if (camera.floored) {
				camera.momentum = camera.momentum * CONST_physics_mu_floor;
			}
			else {
				camera.momentum = camera.momentum * CONST_physics_mu_air;
			}
			move_player( camera.momentum );
		}

		  // Update game state
		if (keys_held[SDLK_UP]) {
			if (physics_mode == PHYSICS_FLY || physics_mode == PHYSICS_CONSTRAINED) {
				GLint viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);

				GLdouble modelMatrix[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

				GLdouble projMatrix[16];
				glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

				GLdouble farx, fary, farz;

				gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.5, modelMatrix, projMatrix, viewport, &farx, &fary, &farz);

				vector<double> location = vector<double>(farx, fary, farz);

				  // Center around the player
				location = location - (vector<double>(camera.x, camera.y, camera.z));
				  // Normalize the vector
				location = location / (~location);
				  // Multiply by movement speed
				location = location * CONST_movespeed;
				  // Add vector into camera location
				move_player( location );
			} else {
				move_player( vector<double>(sin_angle, 0.0, -cos_angle) );
			}
		}
		if (keys_held[SDLK_DOWN]) {
			if (physics_mode == PHYSICS_FLY || physics_mode == PHYSICS_CONSTRAINED) {
				GLint viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);

				GLdouble modelMatrix[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

				GLdouble projMatrix[16];
				glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

				GLdouble farx, fary, farz;

				gluUnProject(config_WIDTH / 2, config_HEIGHT / 2, 0.5, modelMatrix, projMatrix, viewport, &farx, &fary, &farz);

				vector<double> location = vector<double>(farx, fary, farz);

				  // Center around the player
				location = location - (vector<double>(camera.x, camera.y, camera.z));
				  // Normalize the vector
				location = location / (~location);
				  // Multiply by movement speed
				location = location * CONST_movespeed;
				  // Add vector into camera location
				move_player( -location );
			} else {
				move_player( vector<double>(-sin_angle, 0.0, cos_angle) );
			}
		}
		if (keys_held[SDLK_LEFT]) {
			//camera.x -= cos_angle;
			//camera.z -= sin_angle;
			move_player( vector<double>(-cos_angle, 0.0, -sin_angle) );
		}
		if (keys_held[SDLK_RIGHT]) {
			//camera.x += cos_angle;
			//camera.z += sin_angle;
			move_player( vector<double>(cos_angle, 0.0, sin_angle) );
		}

	}

	  // Game exited normally
	SDL_Quit();
	return 0;

}

