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

#include "LinesViewer.h"

#include <iostream>
using namespace std;

static float eyex, eyey, eyez;  // eye x,y,z values for gluLookAt (location of eye)
static float focusx, focusy, focusz; // the point the eye is looking at

struct g_mouseState{
	bool leftButton;
	bool rightButton;
	bool middleButton;
	int x;
	int y;
} MouseStateL;

//#define DEGREES_PER_PIXEL 0.6f
//#define RADIANS_PER_PIXEL 0.002f
#define UNITS_PER_PIXEL 0.001f
static float units_per_pixel = UNITS_PER_PIXEL;
//#define UNITS_PER_WHEELTICK 0.35f
#define ZOOM_FACTOR .04f

static float lmodel_ambient[4]    = {0.1f, 0.1f, 0.1f, 1.0f};

VectListCSegment2d inputSegments;
int listToView = 0;

/*
 our display function, draws a box with proper rotation and
 translation according to our mouse interaction.
*/
void DrawLines()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	// move our eye to the most recent place
	gluLookAt(eyex, eyey, eyez, focusx, focusy, focusz, 0,1,0);

	glShadeModel(GL_SMOOTH);

	// Wireframe render
	//glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
	// Turn off culling
	//glDisable( GL_CULL_FACE );

	// set our color to something
	//glColor3f(1.0f, 1.0f, 1.0f);
	// draw a cube
	//glutSolidCube(1.0);


	glColor3f(1.0f, 0.0f, 0.0f);

	//printf("contorno2d_index drawlines: %d\n", contorno2d_index);

	ListCSegment2d listSegs = inputSegments.at(listToView);

		ListCSegment2d::iterator segmentIter;
		for(segmentIter=listSegs.begin(); segmentIter!= listSegs.end(); ++segmentIter)
		{
			CSegment2d seg = *segmentIter;
			glBegin(GL_LINES);

				glVertex3f( seg.startX, seg.startY, 0);
				glVertex3f( seg.endX, seg.endY, 0);

			glEnd();

		}

	glPopMatrix();
	glutSwapBuffers();
}

/*
 add a light to make the box we're showing a bit more interesting
*/
void InitializeLightLines()
{
	glMatrixMode(GL_MODELVIEW);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	//glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_ambient);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glPolygonMode( GL_FRONT_AND_BACK , GL_LINE );
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_CULL_FACE);
}

/*
  some basic initialization of our GL code
*/
void InitRendererLines()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// set up our viewing frustum
	int m_width = glutGet(GLUT_WINDOW_WIDTH);
	int m_height = glutGet(GLUT_WINDOW_HEIGHT);
	float aspect = (float)m_width/(float)m_height;
	gluPerspective(40.0f, aspect, 0.0f, 10000.0f);

	//initialize the mouse state
	MouseStateL.leftButton = MouseStateL.rightButton = MouseStateL.middleButton = false;
	MouseStateL.x = MouseStateL.y = 0;

	// init our eye location
	eyex = 0;
	eyey = 0;
	//eyez = 5;
	eyez = 3000;
	focusx = focusy = focusz = 0.0f;

	// create some lighting
	InitializeLightLines();
}

void PrintMouseStateLines()
{
	printf("mouse x: %d, y:%d left: %s, right:%s, middle:%s\n", MouseStateL.x,
		MouseStateL.y, MouseStateL.leftButton ? "down":"up",
		MouseStateL.rightButton ? "down":"up", MouseStateL.middleButton ? "down":"up");
}

/*
  this function is called when any mouse buttons are pressed
*/
void HandleMouseStateLines(int button, int state, int x, int y)
{
	//printf("Mouse button: %d\n", button);

	// update our button state
	if(button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseStateL.leftButton = true;
			//printf("leftButton true\n");
		} else {
			MouseStateL.leftButton = false;
			//printf("leftButton false\n");
		}
	}
	if(button == GLUT_RIGHT_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseStateL.rightButton = true;
			//printf("rightButton true\n");
		} else {
			MouseStateL.rightButton = false;
			//printf("rightButton false\n");
		}
	}
	if(button == GLUT_MIDDLE_BUTTON)
	{
		if(state == GLUT_DOWN) {
			MouseStateL.middleButton = true;
			//printf("middleButton true\n");
		} else {
			MouseStateL.middleButton = false;
			//printf("middleButton false\n");
		}
	}

	// update our position so we know a delta when the mouse is moved
	MouseStateL.x = x;
	MouseStateL.y = y;

	//PrintMouseStateLines();
}

void HandleMouseMoveLines(int x, int y)
{
	// calculate a delta in movement
	int yDelta = MouseStateL.y - y;
	int xDelta = MouseStateL.x - x;

	// commit the mouse position
	MouseStateL.x = x;
	MouseStateL.y = y;

	// when we need to rotate (only the left button is down)
	if(MouseStateL.leftButton && !MouseStateL.rightButton && !MouseStateL.middleButton)
	{
		// move our eye
		eyex += xDelta * units_per_pixel;
		eyey -= yDelta * units_per_pixel;

		// move our focus point
		focusx += xDelta * units_per_pixel;
		focusy -= yDelta * units_per_pixel;

	}
	// if we need to move translate (left and right buttons are down
	else if(!MouseStateL.leftButton && MouseStateL.rightButton && !MouseStateL.middleButton)
	{
		eyex = (1 + yDelta/30.0) * eyex - focusx * yDelta/30.0;
		eyey = (1 + yDelta/30.0) * eyey - focusy * yDelta/30.0;
		eyez = (1 + yDelta/30.0) * eyez - focusz * yDelta/30.0;

		//std::cout << eyex<< ", "  <<eyey<< ", "  << eyez  << std::endl;
	}


	glutPostRedisplay();
	//PrintMouseStateLines();
}


void HandleKeyboardLines(unsigned char c, int x, int y)
{
	//printf("Keyboard: %d %d %d\n", c, x, y);
	if (c == 113) {
		exit(0);
	}
}


void processSpecialKeysLines(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_F1 :
				listToView--;
				if (listToView < 0)
					listToView = inputSegments.size() - 1;
				printf("listToView: %d  size: %d\n", listToView, inputSegments.at(listToView).size());
				glutPostRedisplay();
				break;
		case GLUT_KEY_F2 :
				listToView++;
				if (listToView > (inputSegments.size()  - 1))
					listToView = 0;
				printf("listToView: %d  size: %d\n", listToView, inputSegments.at(listToView).size());
				glutPostRedisplay();
				break;
		case GLUT_KEY_F3 :
				units_per_pixel = units_per_pixel * 10.0f;
				printf("units_per_pixel: %f\n", units_per_pixel);
				break;
		case GLUT_KEY_F4 :
				units_per_pixel = units_per_pixel / 10.0f;
				printf("units_per_pixel: %f\n", units_per_pixel);
				break;
	}

}

void ReshapeWindowLines(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = (float)width/(float)height;
	gluPerspective(40.0f, aspect, 0.0f, 10000.0f);
	glMatrixMode(GL_MODELVIEW);
}


//int openLinesViewer(int argc, char* argv[])
 void openLinesViewer(VectListCSegment2d input)
{
	inputSegments = input;

	int argc = 1;
	char *argv[1];
	argv[0]="test.exe";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024,700);
	glutCreateWindow("Simple Lines Viewer :: Larmor");
	glutDisplayFunc(DrawLines);
	glutMouseFunc(HandleMouseStateLines);
	glutMotionFunc(HandleMouseMoveLines);
	glutSpecialFunc(processSpecialKeysLines);
	glutKeyboardFunc(HandleKeyboardLines);
	glutReshapeFunc(ReshapeWindowLines);

	InitRendererLines();

	glutMainLoop();

}

