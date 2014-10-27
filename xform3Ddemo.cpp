//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789_
//
//	OpenGL/ GLUT 'starter' code to demonstrate 3D transforms;
//					play with this to learn how GL_PROJECTION and GL_MODELVIEW
//					matrices affect how 3D vertices appear on-screen.
//	OVERVIEW:
//		OpenGL transforms all vertices first by GL_MODELVIEW matrix and then by
//		the GL_PROJECTION matrix before display on screen. These matrices act as
//		converters from 'model' space to 'world' space to 'eye' space:
//
//		model space --GL_MODELVIEW ---> world space --GL_PROJECTION--> eye space
//		(vertices)  --GL_MODELVIEW --->  (ground)   --GL_PROJECTION--> (film)
//		
//		Note that we're used to thinking of *everything* in world space, both
//		the models and the camera that views them. The behavior of MODELVIEW
//		matrix makes sense to most people--it transforms basic shapes to their 
//		world-space positions.  But the GL_PROJECTION is usually confusing; this
//		matrix transforms away from world-space and into eye-space coords. The 
//		origin of eye-space is the cameras position, the camera's direction of 
//		view is the -Z direction, and camera image x,y define eye-space x,y.  
//		The GL_PROJECTION matrix changes all world-space coordinates of each 
//		point so they are measured using those camera coords.
//			It is *dangerous* to think of 'GL_PROJECTION as the 'camera-
//		position-setting' matrix, because you are probably thinking of the 
//		*INVERSE* of GL_PROJECTION.  Remember, the camera is at the origin of
//		eye space; if we want to find the camera position in world space, we
//		must transform that eye-space origin BACKWARDS through GL_PROJECTION
//		to get back to world-space coordinates.
//
//	OPERATION:
//	  Draws 'world-space' axes as 3 colored lines: +x=RED, +y=GREEN, +z = BLUE
//	  Draws 'model-space' axes as 3 lines: +x = YELLOW, +y = CYAN, +z = PURPLE
//			(and a gray wire-frame teapot in model-space axes)
//		--MOUSE left-click/drag applies x,y glRotation() to GL_PROJECTION 
//		--MOUSE right-click/drag applies x,y glRotation() to GL_MODELVIEW
//		--ARROW keys applies x,y glTranslate() to GL_MODELVIEW
//		-- 'R' key to reset GL_MODELVIEW matrix to initial values.
//		-- 'r' key to reset GL_PROJECTION matrix to initial values.
//		-- 'm' key to enlarge the object.
//		-- 'n' key to shrink the object.
//		-- 'Q' key to quit.
//
//	To compile this under Microsoft Visual Studio (VC++ 6.0 or later) create
//		a new Project, Win32 Console Application, and make an 'empty' project.
//		Then add this file as 'source file', be sure you have the GLUT files
//		in the directory with this source code or installed on your machine.
//
//  If you don't have it, search the web for 'GLUT' --Marc Kilgard's
//		elegant little library that makes OpenGL platform-independent.  GLUT
//		uses 'callback' functions--you tell GLUT the names of your functions,
//		then it calls them whenever users do something that requires a response
//		from your program; for example, they moved the mouse, they resized a
//		window or uncovered it so that it must be re-drawn.
//
//  for CS 351, Northwestern University, Jack Tumblin, jet@cs.northwestern.edu
//
//	12/11/2004 - J. Tumblin--Created.
//	01/01/2005 - J. Tumblin--Added teapot; changed camera position to (0,3,5),
//								and field-of-view to 20 degrees.
//	10/20/2005 - J. Tumblin--Made GLUT local, updated comments for CS351 2005.
//  10/24/2005 - Modified by Seunghoon Kim for CS351 Project B.
//==============================================================================
#include <math.h>							// for sin(), cos(), tan(), etc.
#include <stdlib.h>							// for all non-core C routines.
#include <stdio.h>							// for printf(), scanf(), etc.
#include <iostream>							// for cout, cin, etc.
#include <assert.h>							// for error checking by ASSERT().
#include "glut.h"							// Mark Kilgard's GLUT library.
											// (Error here? be sure you have
											// glut.h, glut.dll, glut.lib
											// in your project directory.
											// What's GLUT? ask google...
using namespace std;

#define JT_TITLE	"CS351 Xform Starter"	// Display window's title bar:
#define JT_WIDTH	480						// window size in pixels
#define JT_HEIGHT	480
#define JT_XPOS		  0						// initial window position
#define JT_YPOS		256
#define JT_ZNEAR	1.0						// near, far clipping planes for
#define JT_ZFAR		50.0					// a 3D camera.

//====================
//
//	Function Prototypes  (these belong in a '.h' file if we have one)
//	
//====================

void glut_init(int *argc, char **argv);	// GLUT initialization
void ogl_init();						// OpenGL initialization

					// GLUT callback functions. Called when:
void display(void);						// GLUT says re-draw the display window.
void reshape(int w, int h);				// GLUT says window was re-sized to w,h
void keyboard(unsigned char key, int x, int y);	//GLUT says user pressed a key
void keySpecial(int key, int x, int y);	// GLUT says user pressed a 'special'key
void mouseMove(int xpos,int ypos);		// GLUT says user moved the mouse to..
void mouseClik(int,int,int,int);		// GLUT says user click/dragged mouse to
//void idle(void);

class CTransRot
//==============================================================================
// Declares a new class that holds how much rotation and translation we want
// for a matrix (such as a GL_PROJECTION or GL_MODELVIEW matrix).  We'll make
// two instances of this class, one for each matrix we'll want to change with
// the mouse.
// PURPOSE:
// Record/accumulate offset amounts and rotation amounts from mouse & keyboard
{
public:
double	x_pos, y_pos, z_pos;	// cumulative position offset
double	x_rot, y_rot, z_rot;	// cumulative rotation on x,y,z axes
double  x_scale, y_scale, z_scale;	// scale for the object
double  x_trans, y_trans, z_trans;	// translation for the object
int		isDragging;				// TRUE if user is holding down the mouse button
								// that affects our value(s); else FALSE.
int m_x,m_y;					// last mouse-dragging position.

~CTransRot(void);				// default destructor
 CTransRot(void);				// default constructor
void reset(void);				// reset everything to zero.
void applyMatrix(void);			// apply translations, rotations to openGL.
};


//===================
//
// GLOBAL VARIABLES (bad idea!)
//
//====================
CTransRot setModel;			// Changes to initial GL_MODELVIEW matrix
CTransRot setProj;			// Changes to initial GL_PROJECTION matrix

int main(int argc, char** argv)
//------------------------------------------------------------------------------
{
	glut_init(&argc, argv);				// First initialize GLUT,
	ogl_init();							// Then initialize any non-default 
										// states we want in openGL,

	glutMainLoop();
	// Then give program control to GLUT.  This is an infinite loop, and from
	// within it GLUT will call the 'callback' functions below as needed.
	return 0;							// orderly exit.
}

//=====================
//
//  Other Function Bodies
//
//=====================

void glut_init(int *argc, char **argv)
//------------------------------------------------------------------------------
// A handy place to put all the GLUT library initial settings; note that we
// 'registered' all the function names for the callbacks we want GLUT to use.
{
	
	glutInit(argc, argv);				// GLUT's own internal initializations.

							// single buffered display, 
							//  RGB color model, use Z-buffering (depth buffer)
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(JT_WIDTH, JT_HEIGHT);	// set display-window size
	glutInitWindowPosition(JT_XPOS, JT_XPOS);	// and position,
	glutCreateWindow(JT_TITLE);					// then create it.

	// Register GLUT Callback function names. (these names aren't very creative)
	glutDisplayFunc(display);			// 'display'  callback:  display();
	glutKeyboardFunc(keyboard);			// 'keyboard' callback:  keyboard(); 
	glutSpecialFunc(keySpecial);		// 'special'keys callback: keyspecial()
	glutReshapeFunc(reshape);			// 'reshape'  callback:  reshape();
//	glutIdleFunc(idle);					// 'idle'	  callback:  idle(); 
// CAREFUL! WE DON'T NEED IDLE()!
	glutMouseFunc(mouseClik);			// callbacks for mouse click, move
	glutMotionFunc(mouseMove);		
}


void ogl_init()
//------------------------------------------------------------------------------
// A handy place to put all the OpenGL initial settings-- remember, you only 
// have to change things if you don't like openGL's default settings.
{
	glClearColor(0.0, 0.0, 0.0, 0.0);	// Display-screen-clearing color;
										// acts as 'background color'
	glColor3f(0.0, 0.0, 0.0);			// Select current color  for drawing
	glShadeModel(GL_FLAT);				// Choose 'flat shading' model  
//	glDisable(GL_LIGHTING);				// No lighting needed  
}


void reshape(int w, int h)
//------------------------------------------------------------------------------
// GLUT 'reshape' Callback. Called when user resizes the window from its current
// shape to one with width w, height h.
// We usually initialize (or re-initialize)openGL's 'GL_PROJECTION' matrix here.
{
	// set size of viewport to window size
	glViewport(0, 0, (GLsizei) w, (GLsizei) h); 

	// Set the projection matrix  
	glMatrixMode(GL_PROJECTION);		// Select the Projection matrix,

//***CHOOSE A CAMERA:***

/*
//----------------------------------------------------
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluOrtho2D(0, w, 0, h);	// orthographic projection(left,right,bottom,top): 
							// using (0,w,0,h) maps x,y world space to screen
							// space in pixel units.
*/

//**OR**----------------------------------------------

/*
	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		45.0,					// vertical (y-axis) field-of-view in degrees,
		(double)w/(double)h,	// display image aspect ratio (width/height),
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.
*/ 
//**OR**----------------------------------------------
/*
	glLoadIdentity();		// (Clear out any previous camera settings)
	gluPerspective(20.0,	// Set camera's vertical field-of-view  (y-axis)
							// measured in degrees, and set the display image
		(double)w/(double)h,// (width/height) and finally
		JT_ZNEAR,JT_ZFAR);	// set near, far clipping planes.
							// if GL_PERSPECTIVE matrix was identity before we
							// called gluPerspective, then we're at the world
							// space origin, but looking in the (world space)
							// -Z direction. 
							// (if current matrix is NOT identity, then the
							// current matrix M is pre-multiplied by the matrix
							// 'T' spec'd by gluPerspective: new matrix is MT).
							//
	// REMEMBER, all vertices are first multipled by the GL_MODELVIEW matrix,
	// and then by the GL_PROJECTION matrix before the 'viewport' maps them
	// to the display window.
	// REMEMBER when you call glTranslate() or glRotate() in openGL, existing 
	// GL_PROJECTION or GL_MODELVIEW is ***PRE_MULTIPLIED*** by the specified
	// translate or rotate matrix to make the GL_PROJECTION or GL_MODELVIEW
	// matrix!  This is *NOT* intuitive!
	//		THUS, if we call glTranslatef(0,-3,-5), then the world-space origin
	// is transformed to (0,-3,-5) *BEFORE* we apply the camera 
	// matrix that turns it into a picture:

		glTranslatef(0.0f, -3.0f, -5.0f);

	//		This gives you the same picture you'd get if you'd translated the
	// camera to the world-space location (0,+3,+5). Confusing, isn't it?!
	// Here's a good way to think of it; 
	//		1) the camera is at the origin of 'eye' space, and looking in the 
	//			-Z direction in 'eye' space.  
	//		2) The glTranslate(0,-3,-5) above converts world space coords to 
	//			eye-space coordinates. 
	// The INVERSE transform (e.g. glTranslate(0,+3,+5) converts eye-space
	// coords to world space.  The camera is always the origin of eye-space; 
	// if we transform the eye-space origin to world space, we find the camera's
	// world-space position is 0,+3,+5.

		glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

	//		Similarly, if we next call glRotationf(30.0,0,1,0) (e.g. rotate by
	// 30 degrees around the y axis) the current contents of the GL_PROJECTION
	// matrix is again pre-multiplied by the new rotation matrix we made.  Any 
	// point in world space is rotated  (about the world-space origin) first, 
	// then translated to make eye-space coordinates (where the camera is at
	// the origin and looking down the -Z axis).  
	// Just as before, the INVERSE transform (eye-space-to-world space) tells
	// us the camera position in world space. Take the origin of eye space
	// (e.g. the camera position) translate(0,+3,+5) so now camera is at 0,3,5
	// and still looking in -Z direction towards origin. Next, rotate about
	// the Y axis by -30 degrees, causing the camera to swing around from the
	// Z axis towards the -X axis. 
*/

//**OR**-----------------------------------------------

	glLoadIdentity();			// (Clear out any previous camera settings)
	gluPerspective(				// Set camera's internal parameters: 
		20.0,					// vertical (y-axis) field-of-view in degrees,
		(double)w/(double)h,	// display image aspect ratio (width/height),
		JT_ZNEAR,JT_ZFAR);		// near,far clipping planes for camera depth.

	gluLookAt(-5.0, 2.0, 8.66,	// VRP: eyepoint x,y,z position in world space.
			   0.0, 0.0, 0.0,	// 'look-at' point--we're looking at origin.
								// (VPN = look-at-point - VRP)
			   0.0, 1.0, 0.0);	// VUP: view 'up' vector; set 'y' as up...
	//*** SURPRISE****
	// the matrix made by gluLookAt() *POST-MULTIPLIES* the current matrix,
	// unlike the glRotate() and glTranslate() functions.

	// Puzzle: What would happen now if you called 'glTranslate(0,0,-10)?
	// can you explain what happens if you then call 'glRotate(30f,0,1,0)?

	// Initialize the modelview matrix to do nothing.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					// Set it to 'do nothing'.
}

void display(void)
//------------------------------------------------------------------------------
// GLUT 'display' Callback.  GLUT calls this fcn when it needs you to redraw 
// the dislay window's contents.  Your program should never call 'display()',
// because it will confuse GLUT--instead, call glutPostRedisplay() if you need
// to trigger a redrawing of the screen.
{
	// Clear the frame-buffer  
	glClear(GL_COLOR_BUFFER_BIT);

// =============================================================================
// START DRAWING CODE HERE 
// =============================================================================
	glMatrixMode(GL_PROJECTION);	// select projection matrix,
	glPushMatrix();					// save current version, then
	setProj.applyMatrix();			// apply results of mouse, keyboard

	// Draw model-space axes:
	glMatrixMode(GL_MODELVIEW);		// select the modelview matrix,
	glPushMatrix();					// save current version, then
	setModel.applyMatrix();			// apply results of mouse, keyboard

//	glColor3f(0.7,0.7,0.7);			// Set color to light grey, and draw a
//	glutWireTeapot(0.8);			// Little Teapot(see OpenGL Red Book, pg 660)
									// (GL_LIGHTING would make this look better)

	glColor3f(0.0,0.1,1.0);
	glutWireSphere(setModel.x_scale, 131, 131);
	glColor3f(0.0,1.0,0.2);
	glutWireSphere(setModel.x_scale, 71, 71);

	glBegin(GL_LINES);				// draw axes in model-space.
		glColor3f ( 1.0, 1.0, 0.0);	// Yellow X axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f(setModel.x_scale, 0.0, 0.0);

		glColor3f ( 0.0, 1.0, 1.0);	// Cyan Y axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f( 0.0, setModel.y_scale, 0.0);

		glColor3f ( 1.0, 0.0, 1.0);	// Purple Z axis
		glVertex3f( 0.0, 0.0, 0.0);
		glVertex3f( 0.0, 0.0, setModel.z_scale);
	glEnd();


	// Draw axes in world-space:	
	glLoadIdentity();			// wipe out current GL_MODELVIEW matrix so that
								// model-space vertices become world-space
								// vertices without change.
	
	glColor3f(1.0,0.1,0.1);
	glutWireSphere(0.5, 133, 133);
	glColor3f(0.8,0.8,0.0);
	glutWireSphere(0.5, 71, 71);

	glBegin(GL_LINES);			// start drawing lines:
		glColor3f ( 1.0, 0.0, 0.0);	// Red X axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f( 1.0, 0.0, 0.0);

		glColor3f ( 0.0, 1.0, 0.0);	// Green Y axis
		glVertex3f( 0.0, 0.0, 0.0);	
		glVertex3f( 0.0, 1.0, 0.0);

		glColor3f ( 0.0, 0.0, 1.0);	// Blue Z axis
		glVertex3f( 0.0, 0.0, 0.0);
		glVertex3f( 0.0, 0.0, 1.0);

	glEnd();					// end drawing lines
	glPopMatrix();				// restore original MODELVIEW matrix.
	glMatrixMode(GL_PROJECTION);// Restore the original GL_PROJECTION matrix
	glPopMatrix();
	// ================================================================================
	// END DRAWING CODE HERE 
	// ================================================================================
	
	cout << "Screen ReDrawn" << endl;
	glFlush();
}                                                                                                                                                                                 

void keyboard(unsigned char key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'keyboard' Callback.  User pressed an alphanumeric keyboard key.
// ('special' keys such as return, function keys, arrow keys? keyboardspecial)
{
	switch(key) {
		case 27: // Esc  
		case 'Q':
		case 'q':
			exit(0);		// Quit application  
			break;
		case 'm':				// enlarge the object
			setModel.x_scale *= 1.1;
			setModel.y_scale *= 1.1;
			setModel.z_scale *= 1.1;
			break;
		case 'n':					// shrink the object
			setModel.x_scale *= 0.9;
			setModel.y_scale *= 0.9;
			setModel.z_scale *= 0.9;
			break;
		case 'r':
			setProj.reset();
			break;
		case 'R':
			setModel.reset();
			break;
		default:
			printf("unknown key.  Try arrow keys, r, R, or q");
			break;
	}
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void keySpecial(int key, int x, int y)
//------------------------------------------------------------------------------
// GLUT 'special' Callback.  User pressed an non-alphanumeric keyboard key, such
// as function keys, arrow keys, etc.
{
static double x_pos, y_pos;

	switch(key)	
	{
		case GLUT_KEY_UP:		// up arrow key
			setModel.z_pos += 0.1;
			break;
		case GLUT_KEY_DOWN:		// dn arrow key
			setModel.z_pos -= 0.1;
			break;
		case GLUT_KEY_LEFT:		// left arrow key
			setModel.x_pos -= 0.1;
			break;
		case GLUT_KEY_RIGHT:	// right arrow key
			setModel.x_pos += 0.1;
			break;
		default:
			break;
	}
	printf("key=%d, setModel.x_pos=%f, setModel.y_pos=%f\n",
							key,setModel.x_pos,setModel.y_pos);
	// We might have changed something. Force a re-display  
	glutPostRedisplay();
}

void mouseClik(int buttonID,int upDown,int xpos,int ypos)
//------------------------------------------------------------------------------
// GLUT 'mouse' Callback.  User caused a click/unclick event with the mouse:
//     buttonID== 0 for left mouse button,
//			  (== 1 for middle mouse button?)
//			   == 2 for right mouse button;
//		upDown == 0 if mouse button was pressed down,
//			   == 1 if mouse button released.
//		xpos,ypos == position of mouse cursor, in pixel units within the window.
// *CAREFUL!* Microsoft puts origin at UPPER LEFT corner of the window.
{
	if(buttonID==0)				// if left mouse button,
	{
		if(upDown==0)			// on mouse press,
		{
			setProj.isDragging = 1;	// get set to record GL_PROJECTION changes.
			setProj.m_x = xpos;		// Dragging begins here.
			setProj.m_y = ypos;
		}
		else setProj.isDragging = 0;
	}
	else if(buttonID==2)		// if right mouse button,
	{
		if(upDown==0)
		{
			setModel.isDragging = 1;// get set to record GL_MODELVIEW changes.
			setModel.m_x = xpos;	// Dragging begins here.
			setModel.m_y = ypos;
		}
		else setModel.isDragging = 0;
	}
	else						// something else.
	{
		setProj.isDragging  = 0;	// default; DON'T change GL_PROJECTION
		setModel.isDragging = 0;	//					or  GL_MODELVIEW
	}

	printf("clik: buttonID=%d, upDown=%d, xpos=%d, ypos%d\n",
										buttonID,upDown,xpos,ypos);
}

void mouseMove(int xpos,int ypos)
//------------------------------------------------------------------------------
// GLUT 'move' Callback.  User moved the mouse while pressing 1 or more of the
// mouse buttons.  xpos,ypos is the MS-Windows position of the mouse cursor in
// pixel units within the window.
// CAREFUL! MSoft puts origin at UPPER LEFT corner pixel of the window!
{
#define JT_INCR 1.0					// Degrees rotation per pixel of mouse move

	if(setModel.isDragging==1)			// if we're dragging the left mouse,
	{								// increment the x,y rotation amounts.
		setModel.x_rot += JT_INCR*(xpos - setModel.m_x);
		setModel.y_rot += JT_INCR*(ypos - setModel.m_y);
		setModel.m_x = xpos;		// and update current mouse position.
		setModel.m_y = ypos;
	}
	if(setProj.isDragging==1)		// if we're dragging theright mouse,
	{								// increment the x,y rotation amounts.
		setProj.x_rot += JT_INCR*(xpos - setProj.m_x);
		setProj.y_rot += JT_INCR*(ypos - setProj.m_y);
		setProj.m_x = xpos;
		setProj.m_y = ypos;
	}
	printf("move %d, %d\n", xpos,ypos);	// print what we did.

	// We might have changed something. Force a re-display  
	glutPostRedisplay();

#undef JT_INCR
}

/*
void idle(void)
//------------------------------------------------------------------------------
// GLUT 'idle' Callback. Called when OS has nothing else to do; a 'clock tick'.  
// Use 'idle' *ONLY IF* your program does anything that needs continual updates, even 
// when users are not pressing keys, then put code to do the updates here.
// If you need to redraw the screen after your update, don't forget to call
// glutPostRedisplay() too.
//
//			*** A COMMON MISTAKE TO AVOID: ***
// 'idle()' gets called VERY OFTEN.  If you register 'idle()' and leave the idle
// function empty, GLUT will waste most/all CPU time not otherwise used on
// useless calls to idle().  If idle() contains only glutPostRedisplay(), you
// will force GLUT to redraw the screen as often as possible--even if the 
// contents of the screen has not changed.  If your program ONLY changes screen 
// contents when user moves,clicks, or drags the mouse, presses a key, etc.,
// then you don't need idle() at all! Instead, call glutPostRedisplay() at the 
// end of each of the GLUT callbacks that change the screen contents.  
// Then you'll update the screen only when there is something new to show on it.
{

}
*/

//==============================================================================
// jt_transRot function bodies:

CTransRot::~CTransRot(void)
//------------------------------------------------------------------------------
// Default destructor
{
}

CTransRot::CTransRot(void)
//------------------------------------------------------------------------------
// Default constructor
{
	reset();						// set all values to zero.
}

void CTransRot::reset(void)
//------------------------------------------------------------------------------
// Set all values to zero.
{
	x_pos = 0.0; y_pos = 0.0; z_pos = 0.0;
	x_rot = 0.0; y_rot = 0.0; z_rot = 0.0;
	x_trans = 0.0; y_trans = 0.0; z_trans = 0.0;
	x_scale = 1.0; y_scale = 1.0; z_scale = 1.0;
}

void CTransRot::applyMatrix(void)
//------------------------------------------------------------------------------
// Apply rotations, then translations to the coordinate axes.  
// (Note OpenGL pre-multiplies matrices,
//   so commands appear to be in reverse order!)
{
	glTranslated(x_pos, y_pos, z_pos);	// OpenGL call to make a translate
									// matrix (see Blue Book) Uses 'd'
									// suffix to specify 'doubles' as arguments.

	glRotated(z_rot, 0.0, 0.0, 1.0);	// OpenGL call to make & apply a rotate
	glRotated(y_rot, 0.0, 1.0, 0.0);	// matrix; we want to rotate the coord
	glRotated(x_rot, 1.0, 0.0, 0.0);	// system FIRST in x, then y, then
										// z, BUT have to call them in
										// !!!REVERSE ORDER!!! because OpenGL
										//  glRotate, glTranslate each make
										// a matrix and then PRE_MULTIPLY the
										// existing 'active' matrix in storage
										// (e.g. the GL_MODELVIEW or 
										// GL_PROJECTION, depending on which
										// one you selected with 
										// 'glMatrixMode()'.
										// Note how we do all this in our
										// 'reshape()' callback function.

//	double xtemp, ytemp, ztemp;			// for temporary x,y,z coordinates storage

//	xtemp = x_pos*x_scale + 1*x_trans;		//		corresponds to	|Xpos|	|Xscale	0		0		Xtrans|	|Xtemp|
//	ytemp = y_pos*y_scale + 1*y_trans;		//		scaling &		|Ypos|*	|0		Yscale	0		Ytrans|=|Ytemp|
//	ztemp = z_pos*z_scale + 1*z_trans;		//		translation		|Zpos|	|0		0		Zscale	Ztrans|	|Ztemp|
	// w = w = 1													|1(W)|	|0		0		0		1	  |	|1(W) |

//	if (y_rot > 0.0)
//	{
//		x_pos = xtemp*cos(y_rot) + ztemp*sin(y_rot);	//		corresponds to	|Xtemp|	|cos0	0	sin0	0	| |Xpos|
//		y_pos = 1 * ytemp;								//		Y-axis rotation	|Ytemp|*|0		1	0		0	|=|Ypos|
//		z_pos = ztemp*cos(y_rot) - xtemp*sin(y_rot);	//						|Ztemp|	|-sin0	0	cos0	0	| |Zpos|
		// w = w = 1															|1(W) |	|0		0	0		1   | |1(W)|
//		xtemp = x_pos;
//		ytemp = y_pos;							//		in case	x rotation and y rotation occur at the same time
//		ztemp = z_pos;
//	}
	
//	if	(x_rot > 0.0)
//	{
//		x_pos = 1 * xtemp;								//		corresponds to	|Xtemp|	|1	0		0		0	| |Xpos|
//		y_pos = ztemp*cos(y_rot) - xtemp*sin(y_rot);	//		X-axis rotation	|Ytemp|*|0	cos0	-sin0	0	|=|Ypos|
//		z_pos = xtemp*cos(y_rot) + ztemp*sin(y_rot);	//						|Ztemp|	|0	sin0	cos0	0	| |Zpos|
		// w = w = 1															|1(W) |	|0	0		0		1   | |1(W)|
//	}
//	else
//	{
//		x_pos = xtemp;
//		y_pos = ytemp;
//		z_pos = ztemp;
//	}
	

}

