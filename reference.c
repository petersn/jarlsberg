#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>	    // Header File For The OpenGL32 Library
#include <GL/glu.h>	    // Header File For The GLu32 Library
#include <stdio.h>      // Header file for standard file i/o.
#include <stdlib.h>     // Header file for malloc/free.
#include <math.h>       // Header file for sin and cos

#include "structures/all.h"
#include "opengl/all.h"
#include "surfaces.h"

#define FOR_EACH_SURFACE(iii) for (iii=0; iii<surf_list.entry_index; iii++) {
#define END_FOR }
#define CURRENT_SURFACE(iii) (((Surface **)(surf_list.data))[iii])

#define SCREENWIDTH 1366
#define SCREENHEIGHT 768

/* ascii codes for various special keys */
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

/* The number of our GLUT window */
int window; 

/* lighting on/off (1 = on, 0 = off) */
int light;

/* L pressed (1 = yes, 0 = no) */
int lp;

/* F pressed (1 = yes, 0 = no) */
int fp;

int mouse_x, mouse_y;
float camera_angle, camera_angle_other;

const int var_framerate = 1000;
const int screenwidth   = 1366;
const int screenheight  = 768;

int width, height, refreshRate;

GLfloat x_pos, y_pos, z_pos;

managed_list surf_list;

GLfloat xrot;   // x rotation 
GLfloat yrot;   // y rotation 
GLfloat xspeed; // x rotation speed
GLfloat yspeed; // y rotation speed

//GLfloat z=-5.0f; // depth into the screen.

/* white ambient light at half intensity (rgba) */
GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

GLuint filter;          /* Which Filter To Use (nearest/linear/mipmapped) */

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
GLvoid InitGL(GLsizei Width, GLsizei Height)	// We call this right after our OpenGL window is created.
{
    //LoadGLTextures();                           // load the textures.
    glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
    glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
    glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
    glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
    glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();				// Reset The Projection Matrix
    
    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
    
    glMatrixMode(GL_MODELVIEW);

    // set up light number 1.
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  // add lighting. (ambient)
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);  // add lighting. (diffuse).
    glLightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
    glEnable(GL_LIGHT1);                             // turn light 1 on.
}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)
{
    if (Height==0)				// Prevent A Divide By Zero If The Window Is Too Small
	  Height=1;

    glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
    glMatrixMode(GL_MODELVIEW);
}

/* The main drawing function. */
GLvoid DrawGLScene(GLvoid)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
    glLoadIdentity();				// Reset The View

    glTranslatef(-x_pos,-y_pos,-z_pos);                  // move z units out from the screen.

    glRotatef(xrot,1.0f,0.0f,0.0f);		// Rotate On The X Axis
    glRotatef(yrot,0.0f,1.0f,0.0f);		// Rotate On The Y Axis

    //glBindTexture(GL_TEXTURE_2D, texture[filter]);   // choose the texture to use.

    int ii;

    FOR_EACH_SURFACE(ii)
        if (ii % 3 == 0) {
            glColor3f(0.0f,0.0f,1.0f);
        }
        if (ii % 3 == 1) {
            glColor3f(0.0f,1.0f,0.0f);
        }
        if (ii % 3 == 2) {
            glColor3f(1.0f,0.0f,0.0f);
        }
        surface_render( CURRENT_SURFACE(ii) );
    END_FOR

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble modelMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

    GLdouble projMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

    //fvector3 near, far;

    GLdouble nearx, neary, nearz;
    GLdouble farx, fary, farz;

    gluUnProject(SCREENWIDTH / 2, SCREENHEIGHT / 2, 0.0, modelMatrix, projMatrix, viewport, &nearx, &neary, &nearz);
    gluUnProject(SCREENWIDTH / 2, SCREENHEIGHT / 2, 0.9, modelMatrix, projMatrix, viewport, &farx, &fary, &farz);

    fvector3 near = {nearx, neary, nearz};
    fvector3 far  = {farx, fary, farz};
    fvector3 direction = fsub( far, near );

    fvector3 nearest_result;
    int nearest_valid;
      // Everyone knows that 1 billion = infinity.
    fvectype longest_distance = 1000000000.0;
    fvectype temp;

    ray_cast_result rc_result;

    FOR_EACH_SURFACE(ii)
        rc_result = ray_cast( CURRENT_SURFACE(ii), near, direction );
        if (rc_result.hit && (temp = fnorm( fsub( rc_result.point, near ) )) < longest_distance ) {
            nearest_valid = 1;
            nearest_result = rc_result.point;
            longest_distance = temp;
        }
        /*
        if (rc_result.hit) {
            glTranslatef(rc_result.point.x, rc_result.point.y, rc_result.point.z);
            glColor3f(0.0f,0.0f,1.0f);
            glutSolidSphere(0.05,100,100);
            glColor3f(0.0f,1.0f,0.0f);
            glTranslatef(-rc_result.point.x, -rc_result.point.y, -rc_result.point.z);
        }
        //*/
    END_FOR

    if (nearest_valid) {
        glTranslatef(nearest_result.x, nearest_result.y, nearest_result.z);
        glColor3f(1.0f,0.0f,0.0f);
        glutSolidSphere(0.1,100,100);
        glColor3f(0.0f,1.0f,0.0f);
        glTranslatef(-nearest_result.x, -nearest_result.y, -nearest_result.z);
    }

    //surface_render( s );

    xrot+=xspeed;		                // X Axis Rotation	
    yrot+=yspeed;		                // Y Axis Rotation

    // since this is double buffered, swap the buffers to display what just got drawn.
    glutSwapBuffers();
}

/* The function called whenever a normal key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{

    switch (key) {
    case ESCAPE: // kill everything.
	/* shut down our window */
	//glutDestroyWindow(window); 

	/* exit the program...normal termination. */
	exit(1);                   	
	break; // redundant.

    case 76: 
    case 108: // switch the lighting.
	printf("L/l pressed; light is: %d\n", light);
	light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
	printf("Light is now: %d\n", light);
	if (!light) {
	    glDisable(GL_LIGHTING);
	} else {
	    glEnable(GL_LIGHTING);
	}
	break;

    case 70:
    case 102: // switch the filter.
	printf("F/f pressed; filter is: %d\n", filter);
	filter+=1;
	if (filter>2) {
	    filter=0;
	}
	printf("Filter is now: %d\n", filter);
	break;

    default:
	break;
    }
}

#define movespeed 0.05

enum {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
      //
    KEY_MAXIMUM
};

int key_map[KEY_MAXIMUM] = {0};

/* The function called whenever a normal key is pressed. */
void specialKeyPressed(int key, int x, int y) 
{

    switch (key) {    
    case GLUT_KEY_PAGE_UP: // move the cube into the distance.
	//z-=0.02f;
	break;
    
    case GLUT_KEY_PAGE_DOWN: // move the cube closer.
	//z+=0.02f;
	break;

    case GLUT_KEY_UP: // decrease x rotation speed;
	//xspeed-=0.1f;
	//xspeed-=10.0f;
	key_map[KEY_UP] = 1;
	//z_pos -= cos(-yrot*3.14/180.0) * movespeed;
    //x_pos -= sin(-yrot*3.14/180.0) * movespeed;
	break;

    case GLUT_KEY_DOWN: // increase x rotation speed;
	//xspeed+=0.1f;
	//xspeed+=10.0f;
	key_map[KEY_DOWN] = 1;
	//z_pos += cos(-yrot*3.14/180.0) * movespeed;
    //x_pos += sin(-yrot*3.14/180.0) * movespeed;
	break;

    case GLUT_KEY_LEFT: // decrease y rotation speed;
	//yspeed-=0.1f;
	//yspeed-=100.0f;
	key_map[KEY_LEFT] = 1;
	//z_pos -= cos(-yrot*3.14/180.0+3.14/2) * movespeed;
    //x_pos -= sin(-yrot*3.14/180.0+3.14/2) * movespeed;
	break;

    case GLUT_KEY_RIGHT: // increase y rotation speed;
	//yspeed+=0.1f;
	//yspeed+=100.f;
	key_map[KEY_RIGHT] = 1;
	//z_pos += cos(-yrot*3.14/180.0+3.14/2) * movespeed;
    //x_pos += sin(-yrot*3.14/180.0+3.14/2) * movespeed;
	break;

    default:
	break;
    }	
}

/* The function called whenever a normal key is pressed. */
void specialKeyReleased(int key, int x, int y) 
{

    switch (key) {    
    case GLUT_KEY_PAGE_UP: // move the cube into the distance.
	//z-=0.02f;
	break;
    
    case GLUT_KEY_PAGE_DOWN: // move the cube closer.
	//z+=0.02f;
	break;

    case GLUT_KEY_UP: // decrease x rotation speed;
	//xspeed-=0.1f;
	//xspeed-=10.0f;
	key_map[KEY_UP] = 0;
	//z_pos -= cos(-yrot*3.14/180.0) * movespeed;
    //x_pos -= sin(-yrot*3.14/180.0) * movespeed;
	break;

    case GLUT_KEY_DOWN: // increase x rotation speed;
	//xspeed+=0.1f;
	//xspeed+=10.0f;
	key_map[KEY_DOWN] = 0;
	//z_pos += cos(-yrot*3.14/180.0) * movespeed;
    //x_pos += sin(-yrot*3.14/180.0) * movespeed;
	break;

    case GLUT_KEY_LEFT: // decrease y rotation speed;
	//yspeed-=0.1f;
	//yspeed-=100.0f;
	key_map[KEY_LEFT] = 0;
	//z_pos -= cos(-yrot*3.14/180.0+3.14/2) * movespeed;
    //x_pos -= sin(-yrot*3.14/180.0+3.14/2) * movespeed;
	break;

    case GLUT_KEY_RIGHT: // increase y rotation speed;
	//yspeed+=0.1f;
	//yspeed+=100.f;
	key_map[KEY_RIGHT] = 0;
	//z_pos += cos(-yrot*3.14/180.0+3.14/2) * movespeed;
    //x_pos += sin(-yrot*3.14/180.0+3.14/2) * movespeed;
	break;

    default:
	break;
    }	
}

void processMouseActiveMotion(int x, int y) {

    mouse_x = x;
    mouse_y = y;

}

int font;

int frame=0, time, timebase=0;
char s[30];

void setOrthographicProjection() {

	// switch to projection mode
	glMatrixMode(GL_PROJECTION);
	// save previous matrix which contains the 
	//settings for the perspective projection
	glPushMatrix();
	// reset matrix
	glLoadIdentity();
	// set a 2D orthographic projection
	gluOrtho2D(0, width, 0, height);
	// invert the y axis, down is positive
	glScalef(1, -1, 1);
	// mover the origin from the bottom left corner
	// to the upper left corner
	glTranslatef(0, -height, 0);
	glMatrixMode(GL_MODELVIEW);
}

void resetPerspectiveProjection() {
	// set the current matrix to GL_PROJECTION
	glMatrixMode(GL_PROJECTION);
	// restore previous settings
	glPopMatrix();
	// get back to GL_MODELVIEW matrix
	glMatrixMode(GL_MODELVIEW);
}


void renderBitmapString(float x, float y, void *font,char *string)
{
  
  char *c;
  // set position to start drawing fonts
  glRasterPos2f(x, y);
  // loop all the characters in the string
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}


void mainloop( int framenumber ) {
    DrawGLScene();

    glutWarpPointer(screenwidth / 2, screenheight / 2);

    if ( key_map[KEY_UP] ) {
        z_pos -= cos(-yrot*3.14/180.0) * movespeed;
        x_pos -= sin(-yrot*3.14/180.0) * movespeed;
    } else if ( key_map[KEY_DOWN] ) {
        z_pos += cos(-yrot*3.14/180.0) * movespeed;
        x_pos += sin(-yrot*3.14/180.0) * movespeed;
    } else if ( key_map[KEY_LEFT] ) {
        z_pos -= cos(-yrot*3.14/180.0+3.14/2) * movespeed;
        x_pos -= sin(-yrot*3.14/180.0+3.14/2) * movespeed;
    } else if ( key_map[KEY_RIGHT] ) {
        z_pos += cos(-yrot*3.14/180.0+3.14/2) * movespeed;
        x_pos += sin(-yrot*3.14/180.0+3.14/2) * movespeed;
    }

	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf(s,"FPS:%4.2f",
			frame*1000.0/(time-timebase));
		timebase = time;		
		frame = 0;
	}

	glColor3f(0.0f,1.0f,1.0f);
	glPushMatrix();
	glLoadIdentity();
	setOrthographicProjection();
	renderBitmapString(30,35,(void *)font,"abcdef");
	glPopMatrix();
	resetPerspectiveProjection();

    //glutTimerFunc( 1000/var_framerate, mainloop, framenumber+1 );

    glutTimerFunc( 1, mainloop, framenumber+1 );
}

int main(int argc, char **argv) 
{

    font = (int) GLUT_BITMAP_8_BY_13;

    refreshRate = 60;

    /* Initialize GLUT state - glut will take any command line arguments that pertain to it or 
       X Windows - look at its documentation at http://reality.sgi.com/mjk/spec3/spec3.html */  
    glutInit(&argc, argv);

    /* Select type of Display mode:   
     Double buffer 
     RGBA color
     Alpha components supported 
     Depth buffer */  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

    /* get a 640 x 480 window */
    //glutInitWindowSize(640, 480);

    /* the window starts at the upper left corner of the screen */
    //glutInitWindowPosition(0, 0);

    width  = glutGet(GLUT_SCREEN_WIDTH);
    height = glutGet(GLUT_SCREEN_HEIGHT);

    char temp[64]; // Aaugh. I hate using buffers that are "big enough"
    sprintf( temp, "%dx%d@%d", width, height, refreshRate );

    glutGameModeString( temp );

    glutEnterGameMode();
    glutSetCursor(GLUT_CURSOR_NONE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    /* Open a window */  
    //window = glutCreateWindow("Jarlsberg");  

    //glEnable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_COLOR_MATERIAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    /* Register the function to do all our OpenGL drawing. */
    glutDisplayFunc(&DrawGLScene);  

    /* Go fullscreen.  This is as soon as possible. */
    glutFullScreen();

    /* Even if there are no events, redraw our gl scene. */
    glutIdleFunc(&DrawGLScene);

    /* Register the function called when our window is resized. */
    glutReshapeFunc(&ReSizeGLScene);

    /* Register the function called when the keyboard is pressed. */
    glutKeyboardFunc(&keyPressed);

    /* Register the function called when special keys (arrows, page down, etc) are pressed. */
    glutSpecialFunc(&specialKeyPressed);

    /* Register the function called when special keys (arrows, page down, etc) are pressed. */
    glutSpecialUpFunc(&specialKeyReleased);

    /* Initialize our window. */
    InitGL(SCREENWIDTH, SCREENHEIGHT);

    list_init( &surf_list, sizeof(Surface *) );

    Surface *s;
    Surface *t;

    s = (Surface *)malloc( sizeof( Surface ) );
    surface_init( s, (ivector3){-10,-2,10}, 20, 20, (ivector3){1,0,0}, (ivector3){0,0,-1} );
    printsurface( s );
    list_append( &surf_list, &s );

    /*
    t = (Surface *)malloc( sizeof( Surface ) );
    surface_init( t, (ivector3){-1,0,-2}, 2, 1, (ivector3){1,0,0}, (ivector3){0,1,0} );
    printsurface( t );
    list_append( &surf_list, &t );
    */

    if (0) {
        //subtract( s, t );
        extrude( s, t, -1 );

        list_free( &surf_list );
        list_init( &surf_list, sizeof(Surface *) );

        copy_cluster( &surf_list );
    }

    glutTimerFunc( 1000/var_framerate, mainloop, 1 );

    /* Start Event Processing Engine */  
    glutMainLoop();  

    return 1;
}

