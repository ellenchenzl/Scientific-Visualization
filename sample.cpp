#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include "glew.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"
#include "glui.h"


//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.


// title of these windows:

const char *WINDOWTITLE = { "OpenGL / GLUT Sample -- Joe Graphics" };
const char *GLUITITLE   = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// size of the box:

const float BOXSIZE = { 2.f };


// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


enum LeftButton
{
	ROTATE,
	SCALE
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// line widths:

const float ISO_WIDTH = { 2. };
const float CONTOUR_WIDTH = { 2. };


// ranges and scalar function:

const float XMIN = { -1.00 };
const float XMAX = {  1.00 };
const float YMIN = { -1.00 };
const float YMAX = {  1.00 };
const float ZMIN = { -1.00 };
const float ZMAX = {  1.00 };
const float RMIN = {  0.00 };
const float RMAX = {  1.732f };
const float GMIN = {  0.00 };
const float GMAX = { 300.00 };


const float SMIN = {   0.0 };
const float SMAX = { 100.0 };

const int GRIDSKIP = { 4 };

#define S	0
#define X	1
#define Y	2
#define Z	3
#define R	4
#define G	5

const char *SFORMAT = { "S: %.3f - %.3f" };
const char *XFORMAT = { "X: %.3f - %.3f" };
const char *YFORMAT = { "Y: %.3f - %.3f" };
const char *ZFORMAT = { "Z: %.3f - %.3f" };
const char *RFORMAT = { "R: %.3f - %.3f" };
const char *GFORMAT = { "G: %.3f - %.3f" };

const int SLIDERWIDTH = { 200 };


// how much to divide the plane and volume:

#define N	10
#define NX	N
#define NY	N
#define NZ	N
#define VECTORSCALE 0
#define FSRS 1
#define MAX_ITERATIONS 300
#define SOME_TOLERANCE 0.15


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint		AxesList;			// list to hold the axes
int		AxesOn;				// != 0 means to draw the axes
int		DebugOn;			// != 0 means to print debugging info
int		DepthCueOn;			// != 0 means to use intensity depth cueing
GLUI *		Glui;				// instance of glui window
int		GluiWindow;			// the glut id for the glui window
int	LeftButton;
int		Grayscale;;			// display in grayscale
int		MainWindow;			// window id for main graphics window
int		PointsOn;			// display points
float		RotMatrix[4][4];	// set by glui rotation widget
float		Scale, Scale2;			// scaling factor
float		TransXYZ[3];		// set by glui translation widgets
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float		Xrot, Yrot;			// rotation angles in degrees



/*float		SLowHigh[2];
float		XLowHigh[2];
float		YLowHigh[2];
float		ZLowHigh[2];
float		RLowHigh[2];
float		GLowHigh[2];

GLUI_StaticText * SLabel;
GLUI_StaticText * XLabel;
GLUI_StaticText * YLabel;
GLUI_StaticText * ZLabel;
GLUI_StaticText * RLabel;
GLUI_StaticText * GLabel;*/

int BoundOn;
int VectorOn;
int StreamlinesOn;
int WhichWayDoProbe;
float TimeStep;

GLUI_HSlider * VectorScaleSlider;
float VectorScaleValue[1];
GLUI_StaticText * VectorScaleLable;
const float VECTORSCALEMIN = { 0.01f };
const float VECTORSCALEMAX = { 0.4f };
const char* VECTORSCALEFORMAT = { "Vector Scale: %5.3f" };

GLUI_HSlider * FSRSSlider;
float FSRSLowHigh[2];
GLUI_StaticText * FSRSLable;
const float FSRSMIN = { 0. };
const double FSRSMAX = { sqrt(12.f) };
const char* FSRSFORMAT = { "Flow Speed Range Slider: %5.3f - %5.3f" };

float ProbeXY[2];
float ProbeYZ[2];
float ProbeXZ[2];

GLboolean redraw_streamline = false;
int iterations = 0;

// function prototypes:

void	Animate( );
void	Buttons(int);
void	Display( );
void	DoAxesMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGlui( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Sliders( int );
float	Temperature( float, float, float );
void	Visibility( int );
void    SetTimeStep(int);
void    Vector(float x, float y, float z, float *vxp, float *vyp, float *vzp);
void    streamlines(float x, float y, float z);
void	Cross(float[3], float[3], float[3]);
//float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
void    Arrow(float tail[3], float head[3]);
void    Advect(float *x, float *y, float *z);
inline float SQR(float x);

void	Axes( float );
void	HsvRgb( float[3], float [3] );
enum WhichProb
{
	OFF,
	SINGLE,
	MULTI,
	RIBBON,
	LINETRACE,
	BLOBTRACE
};

struct node
{
	float x, y, z;           // location
	float vx, vy, vz;        // vector tail
	float rgb[3];		     // the assigned color (to be used later)
	float s;				 // speed
};

struct node  Nodes[NX][NY][NZ], PlaneXY[NX][NY], PlaneXZ[NX][NZ], PlaneYZ[NY][NZ];


void SetTimeStep(int id)
{

}

void
Vector(float x, float y, float z, float *vxp, float *vyp, float *vzp)
{
	*vxp = y * z * (y*y + z * z);
	*vyp = x * z * (x*x + z * z);
	*vzp = x * y * (x*x + y * y);
}
void streamlines(float x, float y, float z)
{
	float vx, vy, vz;
	//glLineWidth(1.);
	//glColor3f(0.0, 255., 255.);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < MAX_ITERATIONS; i++)
	{
		if (x < -1 || x > 1)
			break;
		if (y < -1 || y > 1)
			break;
		if (z < -1 || z > 1)
			break;
		glVertex3f(x, y, z);
		Vector(x, y, z, &vx, &vy, &vz);
		if (sqrt(SQR(vx) + SQR(vy) + SQR(vz)) < SOME_TOLERANCE)
			break;
		Advect(&x, &y, &z);
	}
	glEnd();
}


void Advect(float *x, float *y, float *z)
{
	float xa, ya, za;
	float xb, yb, zb;
	float vxa, vya, vza;
	float vxb, vyb, vzb;
	float xc, yc, zc;
	float vx, vy, vz;

	xa = *x;
	ya = *y;
	za = *z;
	Vector(xa, ya, za, &vxa, &vya, &vza);
	xb = xa + TimeStep * vxa;
	yb = ya + TimeStep * vya;
	zb = za + TimeStep * vza;
	Vector(xb, yb, zb, &vxb, &vyb, &vzb);
	vx = (vxa + vxb) / 2.;
	vy = (vya + vyb) / 2.;
	vz = (vza + vzb) / 2.;
	xc = xa + TimeStep * vx;
	yc = ya + TimeStep * vy;
	zc = za + TimeStep * vz;

	*x = xc;
	*y = yc;
	*z = zc;
}

void DrawObject(float dx, float dy, float dz)
{
	glColor3f(0., 255., 0.);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, 0.05 + dy, 0.0 + dz);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, 0.05 + dz);
	glVertex3f(-0.05 + dx, 0.0 + dy, -0.05 + dz);
	glVertex3f(0.0 + dx, -0.05 + dy, 0.0 + dz);
	glEnd();
}




// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );


	// setup all the graphics stuff:

	InitGraphics( );


	// create the display structures that will not change:

	InitLists( );


	// init all the global variables used by Display( ):
	// this will also post a redisplay
	// it is important to call this before InitGlui():
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset( );


	// setup the user interface stuff:

	InitGlui( );


	// setup all the user interface stuff:

	InitMenus( );


	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );


	// this is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:
	redraw_streamline = true;
	iterations += 8;
	if (iterations >= MAX_ITERATIONS)
	{
		iterations = 0;
	}

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


//
// glui buttons callback:
///

void
Buttons( int id )

{
	char str[256];

	switch( id )
	{
		case RESET:
			Reset();
		
			sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
			VectorScaleLable->set_text(str);

			sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
			FSRSLable->set_text(str);
			FSRSSlider->set_float_limits(FSRSLowHigh[0], FSRSLowHigh[1]);

			Glui->sync_live();
			glutSetWindow(MainWindow);
			glutPostRedisplay();
			break;
		case QUIT:
			/* gracefully close the glui window:				*/
			/* gracefully close out the graphics:				*/
			/* gracefully close the graphics window:			*/
			/* gracefully exit the program:					*/

			Glui->close();
			glutSetWindow( MainWindow );
			glFinish();
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;


		default:
			fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	

	}

	
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );


	// set the eye position, look-at position, and up-vector:

	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );

	glTranslatef((GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2]);

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );


	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	float scale2 = 1.f + Scale2;           // because glui translation starts at 0
	if( scale2 < MINSCALE )
		scale2 = MINSCALE;
	glScalef( scale2, scale2, scale2 );



	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}


	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3f( 1., 1., 1. );
		glCallList( AxesList );
	}


	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );


	// draw the current object:

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;

	// create the object:

	if (BoundOn == GLUITRUE)
	{
		glLineWidth(1.);
		InitLists();
	}
	//vector cloud
	float head[3], tail[3];

	if (VectorOn == GLUITRUE)
	{
		for (int i = 0; i < NX; i++)
		{
			for (int j = 0; j < NY; j++)
			{
				for (int k = 0; k < NZ; k++)
				{
					//calculate head coordinates
					head[0] = (Nodes[i][j][k].x + (Nodes[i][j][k].vx / 2) * VectorScaleValue[0]);
					head[1] = (Nodes[i][j][k].y + (Nodes[i][j][k].vy / 2) * VectorScaleValue[0]);
					head[2] = (Nodes[i][j][k].z + (Nodes[i][j][k].vz / 2) * VectorScaleValue[0]);
					//calculate tail coordinates
					tail[0] = (Nodes[i][j][k].x - (Nodes[i][j][k].vx / 2) * VectorScaleValue[0]);
					tail[1] = (Nodes[i][j][k].y - (Nodes[i][j][k].vy / 2) * VectorScaleValue[0]);
					tail[2] = (Nodes[i][j][k].z - (Nodes[i][j][k].vz / 2) * VectorScaleValue[0]);
					//check speed range
					if (Nodes[i][j][k].s < FSRSLowHigh[0] || Nodes[i][j][k].s > FSRSLowHigh[1])
						continue;
					//draw arrows
					glLineWidth(1.);
					glColor3f(Nodes[i][j][k].rgb[0], Nodes[i][j][k].rgb[1], Nodes[i][j][k].rgb[2]);
					Arrow(tail, head);
				}
			}
		}
	}
	//streamlines
	if (StreamlinesOn == GLUITRUE)
	{
		for (int i = 0; i < NX; i += 2)
		{
			for (int j = 0; j < NY; j += 2)
			{
				for (int k = 0; k < NZ; k += 2)
				{
					glLineWidth(1.);
					glColor3f(0.0, 255., 255.);
					streamlines(Nodes[i][j][k].x, Nodes[i][j][k].y, Nodes[i][j][k].z);
					streamlines(Nodes[9][j][k].x, Nodes[9][j][k].y, Nodes[9][j][k].z);
					streamlines(Nodes[i][9][k].x, Nodes[i][9][k].y, Nodes[i][9][k].z);
					streamlines(Nodes[i][j][9].x, Nodes[i][j][9].y, Nodes[i][j][9].z);
					streamlines(Nodes[9][9][k].x, Nodes[9][9][k].y, Nodes[9][9][k].z);
					streamlines(Nodes[i][9][9].x, Nodes[i][9][9].y, Nodes[i][9][9].z);
					streamlines(Nodes[9][j][9].x, Nodes[9][j][9].y, Nodes[9][j][9].z);
					streamlines(Nodes[9][9][9].x, Nodes[9][9][9].y, Nodes[9][9][9].z);
				}
			}
		}

	}
	//probe
	float ProbeXYZ[3];
	ProbeXYZ[0] = (GLfloat)ProbeXY[0] + (GLfloat)ProbeXZ[0];
	ProbeXYZ[1] = (GLfloat)ProbeXY[1] + (GLfloat)ProbeYZ[1];
	ProbeXYZ[2] = -(GLfloat)ProbeYZ[0] - (GLfloat)ProbeXZ[1];

	if (ProbeXYZ[0] > 1)
		ProbeXYZ[0] = 1;
	else if (ProbeXYZ[0] < -1)
		ProbeXYZ[0] = -1;
	if (ProbeXYZ[1] > 1)
		ProbeXYZ[1] = 1;
	else if (ProbeXYZ[1] < -1)
		ProbeXYZ[1] = -1;
	if (ProbeXYZ[2] > 1)
		ProbeXYZ[2] = 1;
	else if (ProbeXYZ[2] < -1)
		ProbeXYZ[2] = -1;

	switch (WhichWayDoProbe)
	{
	case OFF:
		break;
	case SINGLE:
		//draw single
		iterations = MAX_ITERATIONS;
		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		break;
	case MULTI:
		//draw multi
		iterations = MAX_ITERATIONS;
		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2] + 0.2);
		streamlines(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2] - 0.2);
		streamlines(ProbeXYZ[0], ProbeXYZ[1] + 0.2, ProbeXYZ[2]);
		streamlines(ProbeXYZ[0], ProbeXYZ[1] - 0.2, ProbeXYZ[2]);
		streamlines(ProbeXYZ[0] + 0.2, ProbeXYZ[1], ProbeXYZ[2]);
		streamlines(ProbeXYZ[0] - 0.2, ProbeXYZ[1], ProbeXYZ[2]);
		break;
	case RIBBON:
		//draw ribbon
		float x, y, z;
		float cx, cy, cz;
		float cxp, cyp, czp;
		float nx, ny, nz;
		float nxp, nyp, nzp;

		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);

		cx = x = ProbeXYZ[0] + 4 * 0.05;
		cy = y = ProbeXYZ[1];
		cz = z = ProbeXYZ[2];

		glBegin(GL_QUADS);
		for (int i = 0; i < 10; i++)
		{
			cxp = cx - 0.05;
			cyp = cy;
			czp = cz;

			for (int j = 0; j < 50; j++)
			{
				if (cx < -1 || cx > 1 || cxp < -1 || cxp > 1)
					break;
				if (cy < -1 || cy > 1 || cyp < -1 || cyp > 1)
					break;
				if (cz < -1 || cz > 1 || czp < -1 || czp > 1)
					break;

				nx = cx;
				ny = cy;
				nz = cz;
				Advect(&nx, &ny, &nz);

				nxp = cxp;
				nyp = cyp;
				nzp = czp;
				Advect(&nxp, &nyp, &nzp);

				glVertex3f(cx, cy, cz);
				glVertex3f(cxp, cyp, czp);
				glVertex3f(nxp, nyp, nzp);
				glVertex3f(nx, ny, nz);

				cx = nx;
				cy = ny;
				cz = nz;

				cxp = nxp;
				cyp = nyp;
				czp = nzp;
			}
			cx = x - i * 0.05;
			cy = y;
			cz = z;
		}
		glEnd();
		break;
	case LINETRACE:
		//draw line trace
		float lcx[7], lcy[7], lcz[7];

		DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
		glLineWidth(2.);

		if (redraw_streamline == true)
		{
			for (int n = 0; n < 7; n++)
			{
				lcx[n] = ProbeXYZ[0] + 3 * 0.05 - n * 0.05;
				lcy[n] = ProbeXYZ[1] + 3 * 0.05 - n * 0.05;
				lcz[n] = ProbeXYZ[2] + 3 * 0.05 - n * 0.05;
			}

			for (int i = 0; i < iterations; i++)
			{
				for (int j = 0; j < 7; j++)
				{
					if (lcx[j] < -1 || lcx[j] > 1)
						break;
					if (lcy[j] < -1 || lcy[j] > 1)
						break;
					if (lcz[j] < -1 || lcz[j] > 1)
						break;
					Advect(&lcx[j], &lcy[j], &lcz[j]);
				}
			}

			glBegin(GL_LINE_STRIP);
			glVertex3f(ProbeXYZ[0] + 4 * 0.05, ProbeXYZ[1] + 4 * 0.05, ProbeXYZ[2] + 4 * 0.05);
			for (int n = 0; n < 7; n++)
			{
				glVertex3f(lcx[n], lcy[n], lcz[n]);
			}
			glVertex3f(ProbeXYZ[0] - 4 * 0.05, ProbeXYZ[1] - 4 * 0.05, ProbeXYZ[2] - 4 * 0.05);
			glEnd();
		}
		break;
	case BLOBTRACE:
		//draw blob
		//create little cube
		struct node littleCube[8];
		littleCube[0].x = ProbeXYZ[0] - 0.05;
		littleCube[0].y = ProbeXYZ[1] - 0.05;
		littleCube[0].z = ProbeXYZ[2] + 0.05;
		littleCube[1].x = ProbeXYZ[0] + 0.05;
		littleCube[1].y = ProbeXYZ[1] - 0.05;
		littleCube[1].z = ProbeXYZ[2] + 0.05;
		littleCube[2].x = ProbeXYZ[0] - 0.05;
		littleCube[2].y = ProbeXYZ[1] + 0.05;
		littleCube[2].z = ProbeXYZ[2] + 0.05;
		littleCube[3].x = ProbeXYZ[0] + 0.05;
		littleCube[3].y = ProbeXYZ[1] + 0.05;
		littleCube[3].z = ProbeXYZ[2] + 0.05;
		littleCube[4].x = ProbeXYZ[0] - 0.05;
		littleCube[4].y = ProbeXYZ[1] - 0.05;
		littleCube[4].z = ProbeXYZ[2] - 0.05;
		littleCube[5].x = ProbeXYZ[0] + 0.05;
		littleCube[5].y = ProbeXYZ[1] - 0.05;
		littleCube[5].z = ProbeXYZ[2] - 0.05;
		littleCube[6].x = ProbeXYZ[0] - 0.05;
		littleCube[6].y = ProbeXYZ[1] + 0.05;
		littleCube[6].z = ProbeXYZ[2] - 0.05;
		littleCube[7].x = ProbeXYZ[0] + 0.05;
		littleCube[7].y = ProbeXYZ[1] + 0.05;
		littleCube[7].z = ProbeXYZ[2] - 0.05;
		//tracing
		if (redraw_streamline == true)
		{
			for (int i = 0; i < iterations; i++)
			{
				for (int j = 0; j < 8; j++)
				{
					if (littleCube[j].x < -1 || littleCube[j].x > 1)
						break;
					if (littleCube[j].y < -1 || littleCube[j].y > 1)
						break;
					if (littleCube[j].z < -1 || littleCube[j].z > 1)
						break;
					Advect(&littleCube[j].x, &littleCube[j].y, &littleCube[j].z);
				}
			}
			//redraw object
			DrawObject(ProbeXYZ[0], ProbeXYZ[1], ProbeXYZ[2]);
			glLineWidth(2.);
			//draw little cube
			glBegin(GL_QUADS);
			glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
			glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
			glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);
			glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);

			glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
			glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
			glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);
			glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);

			glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
			glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);
			glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);
			glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);

			glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);
			glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
			glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);
			glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);

			glVertex3f(littleCube[0].x, littleCube[0].y, littleCube[0].z);
			glVertex3f(littleCube[1].x, littleCube[1].y, littleCube[1].z);
			glVertex3f(littleCube[5].x, littleCube[5].y, littleCube[5].z);
			glVertex3f(littleCube[4].x, littleCube[4].y, littleCube[4].z);

			glVertex3f(littleCube[2].x, littleCube[2].y, littleCube[2].z);
			glVertex3f(littleCube[3].x, littleCube[3].y, littleCube[3].z);
			glVertex3f(littleCube[7].x, littleCube[7].y, littleCube[7].z);
			glVertex3f(littleCube[6].x, littleCube[6].y, littleCube[6].z);
			glEnd();
		}
		break;
	}



	// draw some gratuitous text:

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0., 1., 1. );
	DoRasterString( 0., 1., 0., "Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	DoRasterString( 5., 5., 0., "Text That Doesn't" );


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}





void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:

			Glui->close();
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	Glui->sync_live();

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}


//
// initialize the glui window:
//

void
InitGlui( )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;

	GLUI_Panel *bigpanel;
	GLUI_Spinner *spinner;
	char str[256];

	if( DebugOn )
		fprintf( stderr, "InitGlui\n" );

	// setup the glui window:

	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );

	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();


	Glui->add_checkbox("Axes", &AxesOn);

	Glui->add_checkbox("Bounding Box", &BoundOn);

	Glui->add_checkbox("Perspective", &WhichProjection);

	Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	Glui->add_checkbox("Vector Cloud", &VectorOn);

	Glui->add_checkbox("Streamlines", &StreamlinesOn);

	//added for project6

	bigpanel = Glui->add_panel("Probe");
	group = Glui->add_radiogroup_to_panel(bigpanel, &WhichWayDoProbe);
	Glui->add_radiobutton_to_group(group, "Off");
	Glui->add_radiobutton_to_group(group, "Single");
	Glui->add_radiobutton_to_group(group, "Multi");
	Glui->add_radiobutton_to_group(group, "Ribbon");
	Glui->add_radiobutton_to_group(group, "Line Trace");
	Glui->add_radiobutton_to_group(group, "Blob");
	panel = Glui->add_panel_to_panel(bigpanel, "Move the probe");
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe XY", GLUI_TRANSLATION_XY, &ProbeXY[0]);
	trans->set_speed(0.05f);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe YZ", GLUI_TRANSLATION_XY, &ProbeYZ[0]);
	trans->set_speed(0.05f);
	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Probe ZX", GLUI_TRANSLATION_XY, &ProbeXZ[0]);
	trans->set_speed(0.05f);

	Glui->add_separator();

	spinner = Glui->add_spinner("Time Step", GLUI_SPINNER_FLOAT, &TimeStep, 0, (GLUI_Update_CB)SetTimeStep);
	spinner->set_float_limits(0.02, 1.);

	Glui->add_separator();

	VectorScaleSlider = Glui->add_slider(false, GLUI_HSLIDER_FLOAT, VectorScaleValue, VECTORSCALE, (GLUI_Update_CB)Sliders);
	VectorScaleSlider->set_float_limits(VECTORSCALEMIN, VECTORSCALEMAX);
	VectorScaleSlider->set_w(200);		// good slider width
	sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
	VectorScaleLable = Glui->add_statictext(str);

	Glui->add_separator();

	FSRSSlider = Glui->add_slider(true, GLUI_HSLIDER_FLOAT, FSRSLowHigh, FSRS, (GLUI_Update_CB)Sliders);
	FSRSSlider->set_float_limits(FSRSMIN, FSRSMAX);
	FSRSSlider->set_w(200);		// good slider width
	sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
	FSRSLable = Glui->add_statictext(str);

	Glui->add_separator();


	/*GLUI_Panel * panel = Glui->add_panel(  "" );
	Glui->add_checkbox_to_panel( panel, "Points", &PointsOn );
	Glui->add_checkbox( "Grayscale", &Grayscale );

	GLUI_Rollout * rollout = Glui->add_rollout( "Range Sliders", true );
	GLUI_HSlider * slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, SLowHigh, S, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( SLowHigh[0], SLowHigh[1] );
	slider->set_slider_val( SLowHigh[0], SLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, SFORMAT, SLowHigh[0], SLowHigh[1] );
	SLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, XLowHigh, X, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( XLowHigh[0], XLowHigh[1] );
	slider->set_slider_val( XLowHigh[0], XLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, XFORMAT, XLowHigh[0], XLowHigh[1] );
	XLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, YLowHigh, Y, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( YLowHigh[0], YLowHigh[1] );
	slider->set_slider_val( YLowHigh[0], YLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, YFORMAT, YLowHigh[0], YLowHigh[1] );
	YLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, ZLowHigh, Z, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( ZLowHigh[0], ZLowHigh[1] );
	slider->set_slider_val( ZLowHigh[0], ZLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, ZFORMAT, ZLowHigh[0], ZLowHigh[1] );
	ZLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, RLowHigh, R, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( RLowHigh[0], RLowHigh[1] );
	slider->set_slider_val( RLowHigh[0], RLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, RFORMAT, RLowHigh[0], RLowHigh[1] );
	RLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	slider = Glui->add_slider_to_panel( rollout, true, GLUI_HSLIDER_FLOAT, GLowHigh, G, (GLUI_Update_CB) Sliders );
	slider->set_float_limits( GLowHigh[0], GLowHigh[1] );
	slider->set_slider_val( GLowHigh[0], GLowHigh[1] );
	slider->set_w( SLIDERWIDTH );
	sprintf( str, GFORMAT, GLowHigh[0], GLowHigh[1] );
	GLabel = Glui->add_statictext_to_panel( rollout, str );
	Glui->add_separator_to_panel( rollout );

	Glui->add_checkbox( "Perspective", &WhichProjection );

	Glui->add_checkbox( "Axes", &AxesOn );

	Glui->add_checkbox( "Intensity Depth Cueing", &DepthCueOn );*/



	/*panel = Glui->add_panel( "Transformations" );

		GLUI_Rotation * rot = Glui->add_rotation_to_panel( panel, "Rotation", (float *) RotMatrix );
		rot->set_spin( 1. );*/

		/*Glui->add_column_to_panel( panel, FALSE );
		GLUI_Translation * scale = Glui->add_translation_to_panel( panel, "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
		scale->set_speed( 0.01f );

		Glui->add_column_to_panel( panel, FALSE );
		GLUI_Translation * trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
		trans->set_speed( 0.1f );

		Glui->add_column_to_panel( panel, FALSE );
		trans = Glui->add_translation_to_panel( panel, "Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
		trans->set_speed( 0.1f );*/
		panel = Glui->add_panel("Object Transformation");

		rot = Glui->add_rotation_to_panel(panel, "Rotation", (float *)RotMatrix);

		// allow the object to be spun via the glui rotation widget:

		rot->set_spin(1.0);


		
	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox( "Debug", &DebugOn );


	panel = Glui->add_panel( "", FALSE );

	Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );

	Glui->add_column_to_panel( panel, FALSE );

	Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window( MainWindow );


	// set the graphics window's idle function:

	GLUI_Master.set_glutIdleFunc( NULL );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	float hsv[3], rgb[3];
	float vx, vy, vz;

	for (int i = 0; i < NX; i++)
	{
		float x = -1. + 2. * (float)i / (float)(NX - 1);
		for (int j = 0; j < NY; j++)
		{
			float y = -1. + 2. * (float)j / (float)(NY - 1);
			for (int k = 0; k < NZ; k++)
			{
				float z = -1. + 2. * (float)k / (float)(NZ - 1);
				//calculate x, y, z
				Nodes[i][j][k].x = x;
				Nodes[i][j][k].y = y;
				Nodes[i][j][k].z = z;
				//calculate vx, vy, vz
				Vector(x, y, z, &vx, &vy, &vz);
				Nodes[i][j][k].vx = vx;
				Nodes[i][j][k].vy = vy;
				Nodes[i][j][k].vz = vz;
				//calculate spped
				Nodes[i][j][k].s = sqrt(SQR(vx) + SQR(vy) + SQR(vz));
				//calculate hsv and translate to rgb
				hsv[0] = 240. - 240. * ((Nodes[i][j][k].s - 0.) / (2 * sqrt(3.) - 0.));
				hsv[1] = 1.;
				hsv[2] = 1.;
				HsvRgb(hsv, rgb);
				//store rgb
				Nodes[i][j][k].rgb[0] = rgb[0];
				Nodes[i][j][k].rgb[1] = rgb[1];
				Nodes[i][j][k].rgb[2] = rgb[2];
			}
		}
	}

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );


        // DO NOT SET THE GLUT IDLE FUNCTION HERE !!
        // glutIdleFunc( NULL );
        // let glui take care of it in InitGlui()


	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	glBegin(GL_LINES);
	glColor3f(255., 255., 255.);
	glVertex3f(-1., -1., -1.);
	glVertex3f(-1., -1., 1);
	glVertex3f(-1., -1., -1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(-1., -1., -1.);
	glVertex3f(1., -1., -1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(1., 1., 1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(-1., 1., -1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(-1., -1., 1.);
	glVertex3f(1., -1., 1.);
	glVertex3f(1., -1., -1.);
	glVertex3f(-1., 1., 1.);
	glVertex3f(-1., -1., 1.);
	glVertex3f(1., 1., -1.);
	glVertex3f(1., -1., -1.);
	glEnd();

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'r':
		case 'R':
			LeftButton = ROTATE;
			break;

		case 's':
		case 'S':
			LeftButton = SCALE;
			break;
		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	 Glui->sync_live();

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}


	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );


	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT*dy);
			Yrot += (ANGFACT*dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}
	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthCueOn = 0;
	Grayscale = GLUITRUE;
	PointsOn = GLUITRUE;
	Scale  = 1.0;
	Scale2 = 0.0;
	LeftButton = ROTATE;

	WhichProjection = PERSP;
	Xrot = Yrot = 0.;

	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	                  RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;


	/*SLowHigh[0] = SMIN;
	SLowHigh[1] = SMAX;
	XLowHigh[0] = XMIN;
	XLowHigh[1] = XMAX;
	YLowHigh[0] = YMIN;
	YLowHigh[1] = YMAX;
	ZLowHigh[0] = ZMIN;
	ZLowHigh[1] = ZMAX;
	RLowHigh[0] = RMIN;
	RLowHigh[1] = RMAX;
	GLowHigh[0] = GMIN;
	GLowHigh[1] = GMAX;*/

	BoundOn = GLUITRUE;
	VectorOn = GLUIFALSE;
	VectorScaleValue[0] = 0.01;
	StreamlinesOn = GLUIFALSE;
	FSRSLowHigh[0] = FSRSMIN;
	FSRSLowHigh[1] = FSRSMAX;
	TimeStep = 0.02;
	ProbeXY[0] = ProbeXY[1] = ProbeYZ[0] = ProbeYZ[1] = ProbeXZ[0] = ProbeXZ[1] = 0.;
	WhichWayDoProbe = OFF;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// slider callback:

void
Sliders( int id )
{
	char str[256];

	switch( id )
	{
		/*case S:
			sprintf( str, SFORMAT, SLowHigh[0], SLowHigh[1] );
			SLabel->set_text( str );
			break;

		case X:
			sprintf( str, XFORMAT, XLowHigh[0], XLowHigh[1] );
			XLabel->set_text( str );
			break;

		case Y:
			sprintf( str, YFORMAT, YLowHigh[0], YLowHigh[1] );
			YLabel->set_text( str );
			break;

		case Z:
			sprintf( str, ZFORMAT, ZLowHigh[0], ZLowHigh[1] );
			ZLabel->set_text( str );
			break;

		case R:
			sprintf( str, RFORMAT, RLowHigh[0], RLowHigh[1] );
			RLabel->set_text( str );
			break;

		case G:
			sprintf( str, GFORMAT, GLowHigh[0], GLowHigh[1] );
			GLabel->set_text( str );
			break;*/


		case VECTORSCALE:
			sprintf(str, VECTORSCALEFORMAT, VectorScaleValue[0]);
			VectorScaleLable->set_text(str);
			break;
		case FSRS:
			sprintf(str, FSRSFORMAT, FSRSLowHigh[0], FSRSLowHigh[1]);
			FSRSLable->set_text(str);
			break;
	}


	glutSetWindow( MainWindow );
	glutPostRedisplay();
}

#define WINGS	0.10

// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];		// arrow coordinate system

	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	int axis = X;
	float mag = fabs(w[0]);
	if (fabs(w[1]) > mag)
	{
		axis = Y;
		mag = fabs(w[1]);
	}
	if (fabs(w[2]) > mag)
	{
		axis = Z;
		mag = fabs(w[2]);
	}


	// set size of wings and turn w into a Unit vector:

	float d = WINGS * Unit(w, w);


	// draw the shaft of the arrow:

	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	float x, y, z;

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}



	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}




float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}
// produce the scalar temperature value:


struct centers
{
	float xc, yc, zc;	/* center location			*/
	float a;		/* amplitude				*/
} Centers[] =
{
	{	 1.00f,	 0.00f,	 0.00f,	 90.00f	},
	{	-1.00f,	 0.30f,	 0.00f,	120.00f	},
	{	 0.00f,	 1.00f,	 0.00f,	120.00f	},
	{	 0.00f,	 0.40f,	 1.00f,	170.00f	},
};

const int NUMCENTERS = { sizeof(Centers) / sizeof(struct centers) };

inline float
SQR( float x )
{
	return x * x;
}


float
Temperature( float x, float y, float z )
{
	float s = 0.0;
	for( int i = 0; i < NUMCENTERS; i++ )
	{
		float dx = x - Centers[i].xc;
		float dy = y - Centers[i].yc;
		float dz = z - Centers[i].zc;
		float dsqd = SQR(dx) + SQR(dy) + SQR(dz);
		s += Centers[i].a * (float)exp( -5.*dsqd );
	}

	return s;
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r, g, b;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
