/*****************************************************************************
 * Larmor-Physx Version 1.0 2013
 * Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of Larmor-Physx (http://code.google.com/p/larmor-physx/).
 *
 * Larmor-Physx is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Larmor-Physx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Larmor-Physx. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 * $Id$
 *
 ****************************************************************************/

//Lines editor to test SegmentArrangement
// Thanks to http://girish-errishuman.blogspot.com/2011/05/line-drawing-in-opengl.html

#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(WIN64)
	#include "windows.h"
	#include "gl/gl.h"
	#include "gl/glu.h"
#elif defined(__APPLE__) || defined(LINUX) || defined(linux)
	#include "GL/gl.h"
	#include "GL/glu.h"
#endif

//#define FREEGLUT_STATIC
#include "GL/freeglut.h"
//#include "GL/glut.h"

#include "LinesEditor.h"

#include "math.h"
#include <string.h>
#include <iostream>
using namespace std;

float rotate =0.0;
float start = 0.0;
float z_dep = 0.0;

int poi_size = 0;
int poi_arr[100][2];
int m_x = 0;
int m_y = 0;

int rubber_band_on = 0;

int down_flag = 0;
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void *fonts[] =
{
  GLUT_BITMAP_9_BY_15,
  GLUT_BITMAP_TIMES_ROMAN_10,
  GLUT_BITMAP_TIMES_ROMAN_24
};

void selectFont(int newfont)
{
  font = fonts[newfont];
  glutPostRedisplay();
}

void drawCircle(float radius, float x1, float y1)
{
    float angle = 0;
    float x2,y2,cx,cy,fx,fy;
    int cache = 0;
    glBegin(GL_LINES);
    for (angle = 0; angle < 360; angle+=1.0) {
        float rad_angle = angle * 3.14 / 180;
        x2 = x1+radius * sin((double)rad_angle);
        y2 = y1+radius * cos((double)rad_angle);
        if (cache) {
            glVertex2f(cx,cy);
            glVertex2f(x2,y2);
        } else {
            fx = x2;
            fy = y2;
        }
        cache = 1;
        cx = x2;
        cy = y2;
    }
    glVertex2f(x2,y2);
    glVertex2f(fx,fy);

    glEnd();
}


void tick(void)
{
  glutPostRedisplay();
}

void output(int x, int y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}

void display(void)
{
    // Show the X and Y coordinate location of the mouse
  glClear(GL_COLOR_BUFFER_BIT);
  char x_coord[10];
  sprintf(x_coord,"X:%d",m_x);
  char y_coord[10];
  sprintf(y_coord,"Y:%d",m_y);
  output(0, 24, x_coord);
  output(100, 24, y_coord);

  // Draw the lines created by user
   glBegin(GL_LINES);
   for(int i=0; i<poi_size; i++) {
      glVertex2f(poi_arr[i][0],poi_arr[i][1]);
   }
   if (rubber_band_on && (poi_size%2==1)) {
     glVertex2f(m_x,m_y);
   }

   glEnd();

   // Draw small circles at the line start ane end point
   //for(int i=0; i<poi_size; i++) {
   // drawCircle(2,poi_arr[i][0],poi_arr[i][1]);
   //}
  glutSwapBuffers();
}

void handleKeypress(unsigned char key, int x, int y) {
    //switch (key) {
    //    case 27: //Escape key
    //	  glutLeaveMainLoop();
    //}
}

void reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, h, 0);
  glMatrixMode(GL_MODELVIEW);
}

// the mouse callback function:
void mouse_move(int x, int y)
{
    m_x = x;
    m_y = y;
    display();
}

// the mouse callback function:
void mouse(int button, int state, int x, int y)
{
    m_x = x;
    m_y = y;

    switch (button) {
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN) {
        //    glutIdleFunc(spinDisplay);
        }
        if (state == GLUT_UP) {
            if (poi_size>0) {
              poi_size-=2;
            }
            rubber_band_on = 0;
            glutIdleFunc(NULL);
        }
        break;
    case GLUT_LEFT_BUTTON:
        rubber_band_on = 1;
        if (state == GLUT_DOWN) {
            down_flag = 1;
        }
        if (state == GLUT_UP) {
            if(down_flag) {
                poi_arr[poi_size][0]=x;
                poi_arr[poi_size][1]=y;
                poi_size++;
                down_flag = 0;
            }
            glutIdleFunc(NULL);
        }
        break;

    default:
        break;
    }
    display();
}

int openLinesEditor()
{
	int argc = 1;
	char *argv[1];
	argv[0]="test.exe";

  glutInit(&argc, argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

  font = GLUT_BITMAP_9_BY_15;

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(1000, 800);
  glutCreateWindow("OPENGL LINE DRAWING");

  glClearColor(0.0, 0.0, 0.0, 1.0);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutIdleFunc(tick);
  glutKeyboardFunc(handleKeypress);
  glutMouseFunc(mouse);
  glutMotionFunc(mouse_move);
  glutPassiveMotionFunc(mouse_move);

  glutMainLoop();

  return 0;             /* ANSI C requires main to return int. */
}


