/*
 * Project name: Larmor-Physx
 * Released: 14 June 2013
 * Author: Pier Paolo Ciarravano
 * http://www.larmor.com
 *
 * License: This project is released under the Qt Public License (QPL - OSI-Approved Open Source license).
 * http://opensource.org/licenses/QPL-1.0
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 */

#include "MeshViewer.h"

#include <stdio.h>
#include <stdlib.h>
#include "windows.h"
#include "gl/gl.h"
#include "gl/glu.h"

//#define FREEGLUT_STATIC
#include "GL/freeglut.h"

#include "math.h"

#include <iostream>
using namespace std;

//Mesh data
#define REAL float
int numVertexs, numFaces;
REAL* gVertices; //[NUM_VERTICES * 3];
int* gIndices; //[NUM_TRIANGLES][3];


#ifndef M_PI
#define M_PI  3.14159265358979323846f
#endif

static float eyex, eyey, eyez;  // eye x,y,z values for gluLookAt (location of eye)
static float focusx, focusy, focusz; // the point the eye is looking at

struct g_mouseState{
	bool leftButton;
	bool rightButton;
	bool middleButton;
	int x;
	int y;
} MouseState;

#define DEGREES_PER_PIXEL 0.6f
#define RADIANS_PER_PIXEL 0.002f
#define UNITS_PER_PIXEL 0.01f
#define UNITS_PER_WHEELTICK 0.35f
#define ZOOM_FACTOR .04f
float g_xRotation, g_yRotation;


VectListCTriangle3d inputMeshes;
VectListCSegment3d inputSegments3d;
int meshToView = 0;
bool colorViewActive = false;
bool specularActive = false;

static float g_lightPos[4] = { 10, 10, -100, 1 };  // Position of light

static float lmodel_ambient[4]    = {0.1f, 0.1f, 0.1f, 1.0f};

static float material1[4]    = {0.9f, 0.0f, 0.9f, 1.0f};
static float material2[4]    = {0.9f, 0.0f, 0.9f, 1.0f};
static float material3[4]    = {0.9f, 0.9f, 0.9f, 1.0f};
static float material4[4]    = {0.0f, 0.0f, 0.0f, 0.0f};
//
void normalise(CPoint3d &vec)
{
   float length;

   length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));

   if (length == 0.0f)
      length = 1.0f;

   vec.x /= length;
   vec.y /= length;
   vec.z /= length;
}

void calcnormal(CTriangle3d t, CPoint3d &normal)
{
	CPoint3d v1, v2;

   v1.x = t.aX - t.bX;
   v1.y = t.aY - t.bY;
   v1.z = t.aZ - t.bZ;

   v2.x = t.bX - t.cX;
   v2.y = t.bY - t.cY;
   v2.z = t.bZ - t.cZ;

   normal.x = v1.y * v2.z - v1.z * v2.y;
   normal.y = v1.z * v2.x - v1.x * v2.z;
   normal.z = v1.x * v2.y - v1.y * v2.x;

   normalise(normal);
}

void Draw()
{

	// Clear frame buffer and depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   // Set up viewing transformation, looking down -Z axis
   glLoadIdentity();
   gluLookAt(eyex, eyey, eyez, focusx, focusy, focusz, 0,1,0);
   // Set up the stationary light
   //glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);
   // Render the scene


     float colorWhite[4]       = { 1.0, 1.0, 1.0, 1.0 };
     float colorRed[4]       = { 1.0, 0.0, 0.0, 1.0 };
     float colorBlue[4]       = { 0.0, 0.0, 1.0, 1.0 };
     float colorNone[4]       = { 0.0, 0.0, 0.0, 0.0 };

     if (specularActive) {
    	 colorNone[0] = 1.0;
    	 colorNone[1] = 1.0;
    	 colorNone[2] = 1.0;
    	 colorNone[3] = 1.0;
     }

     glMatrixMode(GL_MODELVIEW);
     glPushMatrix();
     	 // rotate our box in the x and y directions
     	glRotatef(g_xRotation, 0,1,0);
     	glRotatef(g_yRotation, 1,0,0);

     glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
     glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
     glColor4fv(colorWhite);


		ListCTriangle3d listTriangls = inputMeshes.at(meshToView);
		ListCTriangle3d::iterator triangleIter;
		for(triangleIter=listTriangls.begin(); triangleIter!= listTriangls.end(); ++triangleIter)
		{
			CTriangle3d t = *triangleIter;
			CPoint3d normal;
			calcnormal(t, normal);

			if (colorViewActive) {
				if (t.type == 1) {
					 glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
					 glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
					 glColor4fv(colorWhite);
				} else if (t.type == 2) {
					glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBlue);
					 glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
					 glColor4fv(colorBlue);
				} else if (t.type == 3) {
					glMaterialfv(GL_FRONT, GL_DIFFUSE, colorRed);
					glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
					glColor4fv(colorRed);
				} else {
					printf("ERROR COLOR:: t.type is not 1,2 or 3\n");
				}
			}

			//glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_TRIANGLES);
				glNormal3d(normal.x, normal.y, normal.z);
				glVertex3f( t.aX, t.aY, t.aZ);
				glVertex3f( t.bX, t.bY, t.bZ);
				glVertex3f( t.cX, t.cY, t.cZ);
			glEnd();

			//glColor3f(1.0f, 1.0f, 1.0f);
			glBegin(GL_TRIANGLES);
				glNormal3d(-normal.x, -normal.y, -normal.z);
				glVertex3f( t.cX, t.cY, t.cZ);
				glVertex3f( t.bX, t.bY, t.bZ);
				glVertex3f( t.aX, t.aY, t.aZ);
			glEnd();


		}

		ListCSegment3d listSegments = inputSegments3d.at(meshToView);
		ListCSegment3d::iterator segmentsIter;
		for(segmentsIter=listSegments.begin(); segmentsIter!= listSegments.end(); ++segmentsIter)
		{
			CSegment3d s = *segmentsIter;
			glColor3f(1.0f, 0.0f, 0.0f);
			glBegin(GL_LINES);
				glVertex3f(s.startX, s.startY, s.startZ);
				glVertex3f(s.endX, s.endY, s.endZ);
			glEnd();
		}



     glPopMatrix();







   // Make sure changes appear onscreen
   glutSwapBuffers();

}


/*
  some basic initialization of our GL code
*/
void InitRenderer()
{
	/*
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	*/

	// starting rotation.
	g_yRotation = 20.0f;
	g_xRotation = 30.0f;

	//initialize the mouse state
	MouseState.leftButton = MouseState.rightButton = MouseState.middleButton = false;
	MouseState.x = MouseState.y = 0;

	// init our eye location
	eyex = 0;
	eyey = 0;
	eyez = 5;
	focusx = focusy = focusz = 0.0f;


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// set up our viewing frustum
	int m_width = glutGet(GLUT_WINDOW_WIDTH);
	int m_height = glutGet(GLUT_WINDOW_HEIGHT);
	float aspect = (float)m_width/(float)m_height;
	gluPerspective(40.0f, aspect, 0.0f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //Aggiunto



	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_ambient);
	glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f );
	glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION,  0.0f );
	glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f );


	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, material1 );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, material2 );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, material3 );
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, material4 );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 10.0f );

	// Solid render
	glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
	//glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );

	// Shade mode
	//glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);

	// Turn on the zbuffer
	glEnable( GL_DEPTH_TEST );
	//glDisable( GL_DEPTH_TEST );

	// Turn on zbuffer writing
	glDepthMask(1);

	// Turn on zbuffer function
	glDepthFunc(GL_LESS);

	// Turn on culling (CCW --> ogl default mode - like d3d)
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );

}

void PrintMouseState()
{
	printf("mouse x: %d, y:%d left: %s, right:%s, middle:%s\n", MouseState.x,
		MouseState.y, MouseState.leftButton ? "down":"up",
		MouseState.rightButton ? "down":"up", MouseState.middleButton ? "down":"up");
}

/*
  this function is called when any mouse buttons are pressed
*/
void HandleMouseState(int button, int state, int x, int y)
{
	//printf("Mouse button: %d\n", button);

	// update our button state
	if(button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseState.leftButton = true;
			//printf("leftButton true\n");
		} else {
			MouseState.leftButton = false;
			//printf("leftButton false\n");
		}
	}
	if(button == GLUT_RIGHT_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseState.rightButton = true;
			//printf("rightButton true\n");
		} else {
			MouseState.rightButton = false;
			//printf("rightButton false\n");
		}
	}
	if(button == GLUT_MIDDLE_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseState.middleButton = true;
			//printf("middleButton true\n");
		} else {
			MouseState.middleButton = false;
			//printf("middleButton false\n");
		}
	}

	// update our position so we know a delta when the mouse is moved
	MouseState.x = x;
	MouseState.y = y;

	//PrintMouseState();
}

void HandleMouseMove(int x, int y)
{
	// calculate a delta in movement
	int yDelta = MouseState.y - y;
	int xDelta = MouseState.x - x;

	// commit the mouse position
	MouseState.x = x;
	MouseState.y = y;

	// when we need to rotate (only the left button is down)
	if(MouseState.leftButton && !MouseState.rightButton && !MouseState.middleButton)
	{
		// rotate by the delta
		g_xRotation -= xDelta * DEGREES_PER_PIXEL;
		g_yRotation -= yDelta * DEGREES_PER_PIXEL;
	}
	// if we need to move translate (left and right buttons are down
	else if(!MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
	{
		// move our eye
		eyex += xDelta * UNITS_PER_PIXEL;
		eyey -= yDelta * UNITS_PER_PIXEL;

		// move our focus point
		focusx += xDelta * UNITS_PER_PIXEL;
		focusy -= yDelta * UNITS_PER_PIXEL;
	}

	// zoom
	if(!MouseState.leftButton && !MouseState.rightButton && MouseState.middleButton)
	{
		eyex = (1 + yDelta/30.0) * eyex - focusx * yDelta/30.0;
		eyey = (1 + yDelta/30.0) * eyey - focusy * yDelta/30.0;
		eyez = (1 + yDelta/30.0) * eyez - focusz * yDelta/30.0;
		//int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
		//printf("Mouse button: %d\n",  yDelta);
		//eyez = eyez + yDelta/10.0;
	}

	glutPostRedisplay();
	//PrintMouseState();
}


void HandleKeyboard(unsigned char c, int x, int y)
{
	//printf("Keyboard: %d %d %d\n", c, x, y);
	if (c == 113) {
		printf("Quit!!\n");
		exit(0);
	}
}


void processSpecialKeys(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_F1 :
				glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
				glutPostRedisplay();
				break;
		case GLUT_KEY_F2 :
				glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
				glutPostRedisplay();
				break;
		case GLUT_KEY_F3 :
				glPolygonMode( GL_FRONT_AND_BACK , GL_POINT );
				glutPostRedisplay();
				break;

		case GLUT_KEY_F5 :
				meshToView--;
				if (meshToView < 0)
					meshToView = inputMeshes.size() - 1;
				printf("meshToView: %d\n", meshToView);
				glutPostRedisplay();
				break;
		case GLUT_KEY_F6 :
				meshToView++;
				if (meshToView > (inputMeshes.size()  - 1))
					meshToView = 0;
				printf("meshToView: %d\n", meshToView);
				glutPostRedisplay();
				break;

		case GLUT_KEY_F7 :
				colorViewActive = true;
				glutPostRedisplay();
				break;
		case GLUT_KEY_F8 :
				colorViewActive = false;
				glutPostRedisplay();
				break;

		case GLUT_KEY_F9 :
			specularActive = true;
				glutPostRedisplay();
				break;
		case GLUT_KEY_F10 :
			specularActive = false;
				glutPostRedisplay();
				break;

	}

}

void ReshapeWindow(int width, int height)
{
	//std::cout << "ReshapeWindow" << std::endl;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = (float)width/(float)height;
	//gluPerspective(40.0f, aspect, 0.0f, 10000.0f);
	gluPerspective(65.0f, aspect, 0.001f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
}

//int openMeshViewer(int argc, char* argv[])
void openMeshViewer(VectListCTriangle3d input, VectListCSegment3d segments)
{
	inputMeshes = input;
	inputSegments3d = segments;

	//readAndInitMesh("C:\\Users\\Iaia\\Downloads\\dragon_recon\\dragon_recon\\dragon_vrip_res4.ply");
		int argc = 1;
		char *argv[1];
		argv[0]="test.exe";

	glutInit(&argc, argv);

	glutInitWindowSize(1024,700);
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Simple Mesh Viewer :: Larmor");

	InitRenderer();

	glutDisplayFunc(Draw);
	glutReshapeFunc(ReshapeWindow);
	glutMouseFunc(HandleMouseState);
	glutMotionFunc(HandleMouseMove);
	glutSpecialFunc(processSpecialKeys);
	glutKeyboardFunc(HandleKeyboard);

	glutMainLoop();

}





