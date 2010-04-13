 // Main Jarlsberg code
using namespace std;
#include <iostream>
#include <list>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cmath>

#include "surfaces.h"
#include "surfaces.cpp"

#define BPP 4
#define DEPTH 32

  // Game constants
const double CONST_movespeed = 0.3;

  // Global variables that are essentially static configuration
int config_WIDTH;
int config_HEIGHT;
SDL_Surface *screen;
int glob_tex;

  // Dynamic globals
list<surface> geometry;
list<surface> geometry_temp;
list<surface>::iterator iter_surf;

  // Make an array to store all currently held keys.
  // 1024 is simply a heursitic: codes for keys seem to be ASCII + small extensions beyond.
char keys_held[1024] = {0};
int mouse_x, mouse_y;

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
GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

  // Global variables
struct camera_t {
    GLfloat x, y, z;
    GLfloat facing, tilt;

      // New angles are mixed with old angles to smooth out low FPS mice
    GLfloat facing_target, tilt_target;
      // Set to 0.0 for the camera to be as sluggish as possible, and 1.0 for it to be as snappy as possible
#define CAMERA_SMOOTHING_FACTOR 0.4

};

  // Default camera
camera_t camera = {0};

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

      // Reposition the camera based on its position
    glRotatef( camera.tilt,   1.0f, 0.0f, 0.0f);
    glRotatef( camera.facing, 0.0f, 1.0f, 0.0f);
    glTranslatef( -camera.x, -camera.y, -camera.z );

    int ii = 0;
    for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {
        if (ii%3 == 0)
            glColor3f(1.0f, 0.0f, 0.0f);
        if (ii%3 == 1)
            glColor3f(0.0f, 1.0f, 0.0f);
        if (ii%3 == 2)
            glColor3f(0.0f, 0.0f, 1.0f);
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
        glutSolidSphere(0.1,10,10);
        glTranslatef(-hit_point.x, -hit_point.y, -hit_point.z);
    }

    if (selection_state == SELECTION_SELECTING || selection_state == SELECTION_DONE) {

        hit = true;

        matrix<double> globalize = matrix_recast<int, double>( &(selection_plane.globalize_basis) );
        vector<double> temp = globalize * vector_recast<int, double>(selection_start);

        if (selection_state == SELECTION_SELECTING) {
            surface *new_selection_plane;
              // Magically load up values into the variables
            raycast_from_camera( &hit, NULL, &selection_end, &new_selection_plane );
            if (hit && (!coplanar(selection_plane, *new_selection_plane))) hit = false;
        }

        if (hit) {
            temp = temp + vector_recast<int, double>(selection_plane.localize_basis.row2)*0.1;
            glColor3f(1.0f,1.0f,1.0f);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp + globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp + globalize * vector<double>( 0, (selection_end.y-selection_start.y), 0 );
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp - globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
                glVertex3f(temp.x, temp.y, temp.z);
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp + globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp - globalize * vector<double>( 0, (selection_end.y-selection_start.y), 0 );
                glVertex3f(temp.x, temp.y, temp.z);
                temp = temp - globalize * vector<double>( (selection_end.x-selection_start.x), 0, 0 );
                glVertex3f(temp.x, temp.y, temp.z);
            glEnd();
        }
    }

    SDL_GL_SwapBuffers();

//    SDL_BlitSurface(text, &src, screen, &dest);

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

int LoadTexture(const char *path) {
    GLuint texture;
    SDL_Surface* bmpFile;

    cout << "Loading texture: " << path << endl;

      // Load the bitmap
    bmpFile = SDL_LoadBMP(path);

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
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    gluBuild2DMipmaps(GL_TEXTURE_2D,3,bmpFile->w, \
        bmpFile->h,GL_BGR_EXT, \
        GL_UNSIGNED_BYTE,bmpFile->pixels);

      //Free surface after using it
    SDL_FreeSurface(bmpFile);

    return texture;
}

void InitGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
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
    glLightfv(GL_LIGHT1, GL_AMBIENT,  LightAmbient);  // Add lighting -- Ambient
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  LightDiffuse);  // Add lighting -- Diffuse
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition); // Set light position
    glEnable(GL_LIGHT1);                              // Turn light 1 on
}


int main(int argc, char **argv) {

    const SDL_VideoInfo *info;
    bool game_continue;
    int ii;
    double cos_angle, sin_angle;
    SDL_Event event;
      // Flags to pass to SDL_SetVideoMode
    int videoFlags;

    geometry.push_back( surface(vector<int>(20,-5,-20), 40, 40, vector<int>(-1,0,0), vector<int>(0,0,1)) );

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
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
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
    glob_tex = LoadTexture("Data/Old.bmp");

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
                        if (selection_state == SELECTION_DONE) {
                            surface_local( selection_plane, selection_plane,
                                           selection_start.x, selection_start.y,
                                           selection_end.x-selection_start.x,
                                           selection_end.y-selection_start.y );

                              // Clear out the temporary list
                            geometry_temp.clear();

                            list<surface> new_list;

                            //cout << "Iterating through " << geometry.size() << " elements" << endl;

                              // Iterate through each surface, and add its associated extruded representation
                            for (iter_surf=geometry.begin(); iter_surf != geometry.end(); ++iter_surf) {
                                //cerr << "Loop" << endl;
                                if (keys_held[SDLK_PAGEUP]) {
                                    new_list = extrude( *iter_surf, selection_plane, 1 );
                                    //cout << "selstart: " << selection_start << " normal: " << selection_plane.localize_basis.row2 << endl;
                                    //selection_start = selection_start + selection_plane.localize_basis.row2;
                                    //selection_end   = selection_end + selection_plane.localize_basis.row2;
                                    //selection_plane.local_position.z += 1;
                                } else {
                                    new_list = extrude( *iter_surf, selection_plane, -1 );
                                    //selection_start = selection_start + vector<int>(0,-1,0);
                                    //selection_start = selection_start - selection_plane.localize_basis.row2;
                                    //selection_end   = selection_end - selection_plane.localize_basis.row2;
                                    //selection_plane.local_position.z -= 1;
                                }
                                //selection_plane.position = selection_plane.globalize_basis * selection_plane.local_position;
                                //cout << "Adding " << new_list.size() << " new elements" << endl;
                                geometry_temp.insert( geometry_temp.end(), new_list.begin(), new_list.end() );
                            }

                            geometry_temp = prune( geometry_temp );
                            geometry      = optimize( geometry_temp );

                            if (keys_held[SDLK_PAGEUP]) {
                                selection_start = selection_start + vector<int>(0,0,1);
                                selection_end = selection_end + vector<int>(0,0,1);
                                selection_plane.local_position.z += 1;
                            } else {
                                selection_start = selection_start - vector<int>(0,0,1);
                                selection_end = selection_end - vector<int>(0,0,1);
                                selection_plane.local_position.z -= 1;
                            }
                            selection_plane.position = selection_plane.globalize_basis * selection_plane.local_position;

                            //geometry = geometry_temp;

                            cout << "Total surfaces: " << geometry.size() << endl;

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
        cos_angle = (CONST_movespeed*0.5) * cos( camera.facing * (M_PI / 180.0) );
        sin_angle = (CONST_movespeed*0.5) * sin( camera.facing * (M_PI / 180.0) );

          // Update game state
        if (keys_held[SDLK_UP]) {
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
            camera.x += location.x;
            camera.y += location.y;
            camera.z += location.z;
        }
        if (keys_held[SDLK_DOWN]) {
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
            camera.x -= location.x;
            camera.y -= location.y;
            camera.z -= location.z;
        }
        if (keys_held[SDLK_LEFT]) {
            camera.x -= cos_angle;
            camera.z -= sin_angle;
        }
        if (keys_held[SDLK_RIGHT]) {
            camera.x += cos_angle;
            camera.z += sin_angle;
        }

    }

      // Game exited normally
    SDL_Quit();
    return 0;

}

