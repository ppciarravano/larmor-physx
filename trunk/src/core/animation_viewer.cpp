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

#include "animation_viewer.h"

#define DEGREES_PER_PIXEL_ROT 0.3f
#define DEGREES_PER_PIXEL_TLT 0.3f
#define UNITS_PER_PIXEL_ZOM 0.5f

#define CONV_PI  3.14159265358979323846f

//#define USE_TWO_MESH_COLORS

namespace LarmorPhysxViewer
{

	unsigned int idFrame = 0;
	unsigned int idScreenFrame = 0;

	// Camera and lookat position
	Camera eyeCamera;

	// Timing Variables
	int g_nFPS = 0;
	int g_nFrames = 0;
	clock_t g_lastFPS = 0;
	clock_t time_now;
	clock_t movement_timer = 0;

	struct g_mouseState {
		bool leftButton;
		bool rightButton;
		bool middleButton;
		int x;
		int y;
	} MouseState;

	int meshToView = 0;
	bool colorViewActive = false;
	bool specularActive = false;
	bool doAnim = false;
	bool doAnimCamera = false;
	bool doSaveAnimScreenshot = false;
	bool viewCollision = false;

	//static float g_lightPos[4] = { 10, 10, -100, 1 };  // Position of light
	static float lmodel_ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	static float material1[4] = { 0.9f, 0.0f, 0.9f, 1.0f };
	static float material2[4] = { 0.9f, 0.0f, 0.9f, 1.0f };
	static float material3[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
	static float material4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 };
	float colorWhiteRed[4] = { 1.0, 1.0, 0.0, 1.0 };
	float colorGray[4] = { 0.5, 0.5, 0.5, 1.0 };
	float colorRed[4] = { 1.0, 0.0, 0.0, 1.0 };
	float colorBlue[4] = { 0.0, 0.0, 1.0, 1.0 };
	float colorGreen[4] = { 0.0, 1.0, 0.0, 1.0 };
	float colorNone[4] = { 0.0, 0.0, 0.0, 0.0 };

	std::vector<StaticObject> staticObjectVector;
	std::vector<DynamicObjectVector> dynamicObjectVectorFrames;
	std::vector<CollisionPointVector> collisionPointVectorFrames;

	Camera animCamera;
	bool isAnimCameraValid = false;

	void loadAnimation()
	{
		idFrame = LarmorPhysx::ConfigManager::start_load_frame;
		//TODO: Fix to use a hash map int->StaticObject
		staticObjectVector.push_back(StaticObject()); //Fake element, to not use a set instead of a vector

		for (unsigned int frameCounter = 0;
				frameCounter <= LarmorPhysx::ConfigManager::total_anim_steps;
				++frameCounter)
		{
			Frame stepFrame = LarmorPhysx::loadFrame(idFrame);

			dynamicObjectVectorFrames.push_back(stepFrame.dynamicObjects);

			//std::cout << "  Total collisionPoints size: " << stepFrame.collisionPoints.size() << std::endl;
			collisionPointVectorFrames.push_back(stepFrame.collisionPoints);

			DynamicObjectVector::iterator dynamicObjectVectorIter;
			for(dynamicObjectVectorIter = stepFrame.dynamicObjects.begin();
					dynamicObjectVectorIter != stepFrame.dynamicObjects.end();
					++dynamicObjectVectorIter)
			{
				DynamicObject dynamicObject = *dynamicObjectVectorIter;

				if(staticObjectVector.size() == dynamicObject.idObj)
				{
					StaticObject staticObject = LarmorPhysx::loadStaticObject( dynamicObject.idObj );
					staticObjectVector.push_back(staticObject);
					//std::cout << "OK inserted staticObject: " << staticObject.idObj << std::endl;
				}
			}

			idFrame++;
		}

		idFrame = 0;
	}

	void normalise(LVector3 &vec)
	{
	   float length;

	   length = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));

	   if (length == 0.0f)
		  length = 1.0f;

	   vec.x /= length;
	   vec.y /= length;
	   vec.z /= length;
	}

	void calcNormal(LVector3 ta, LVector3 tb, LVector3 tc, LVector3 &normal)
	{
		LVector3 v1, v2;

		v1.x = ta.x - tb.x;
		v1.y = ta.y - tb.y;
		v1.z = ta.z - tb.z;

		v2.x = tb.x - tc.x;
		v2.y = tb.y - tc.y;
		v2.z = tb.z - tc.z;

		normal.x = v1.y * v2.z - v1.z * v2.y;
		normal.y = v1.z * v2.x - v1.x * v2.z;
		normal.z = v1.x * v2.y - v1.y * v2.x;

		normalise(normal);
	}

	void print_bitmap_string(void* font, const char* s)
	{
	   if (s && strlen(s)) {
	      while (*s) {
	         glutBitmapCharacter(font, *s);
	         s++;
	      }
	   }
	}


	//C:\LARMOR\video\FFF\ffmpeg\ffmpeg.exe -r 30 -i C:\LARMOR\DEVELOP\WORKSPACE_CDT\RevanoVX\Release\scenes_output_wall3\images\frameimg_%05d.bmp -vcodec libx264 -vpre ./libx264-hq.ffpreset -b 10M -bt 10M test_wall1.mp4
	//C:\LARMOR\video\FFF\ffmpeg\ffmpeg.exe -r 30 -i C:\LARMOR\DEVELOP\WORKSPACE_CDT\RevanoVX\Release\scenes_output_wall3\images\frameimg_%05d.bmp -vcodec libx264 -vpre C:\LARMOR\video\FFF\ffmpeg\ffpresets\libx264-hq.ffpreset -b 20M -bt 20M test_video1.mp4
	void save_bmp_screenshot()
	{
		unsigned char *pixels = (unsigned char*)malloc(glutGet(GLUT_WINDOW_WIDTH)*glutGet(GLUT_WINDOW_HEIGHT)*3);  // Assuming GL_RGB

		//opengl capture
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), GL_RGB, GL_UNSIGNED_BYTE, pixels);

		//Save BMP image
		unsigned int w = glutGet(GLUT_WINDOW_WIDTH);
		unsigned int h = glutGet(GLUT_WINDOW_HEIGHT);
		int filesize = 54 + w*h*3;
		unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
		unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
		unsigned char bmppad[3] = {0,0,0};
		bmpfileheader[ 2] = (unsigned char)(filesize);
		bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
		bmpfileheader[ 4] = (unsigned char)(filesize>>16);
		bmpfileheader[ 5] = (unsigned char)(filesize>>24);
		bmpinfoheader[ 4] = (unsigned char)(w);
		bmpinfoheader[ 5] = (unsigned char)(w>> 8);
		bmpinfoheader[ 6] = (unsigned char)(w>>16);
		bmpinfoheader[ 7] = (unsigned char)(w>>24);
		bmpinfoheader[ 8] = (unsigned char)(h);
		bmpinfoheader[ 9] = (unsigned char)(h>> 8);
		bmpinfoheader[10] = (unsigned char)(h>>16);
		bmpinfoheader[11] = (unsigned char)(h>>24);

		char bufferFileNumber[10];
		sprintf(bufferFileNumber, "%05d", idScreenFrame);
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << "images" << FILE_SEPARATOR_CHAR << "frameimg_" << bufferFileNumber << ".bmp";
		FILE * ppmFile = fopen(fileName.str().c_str(), "wb");
		fwrite(bmpfileheader,1,14,ppmFile);
		fwrite(bmpinfoheader,1,40,ppmFile);
		for(int i=(h-1); i>=0; i--)
		{
		    fwrite(pixels+(w*(h-i-1)*3),3,w,ppmFile);
		    fwrite(bmppad,1,(4-(w*3)%4)%4,ppmFile);
		}
		fclose(ppmFile);
		std::cout << "save_bmp_screenshot in: " << fileName.str() <<std::endl;

		free(pixels);

		idScreenFrame++;
		//std::cout << "idScreenFrame: " << idScreenFrame <<std::endl;
	}

	void draw()
	{

		//FPS counter
		clock_t g_nowFPS = clock();
		if( g_nowFPS - g_lastFPS >= CLOCKS_PER_SEC )
		{
			g_lastFPS = g_nowFPS;
			g_nFPS = g_nFrames;
			g_nFrames = 0;

			//std::cout << "FPS: " << g_nFPS <<std::endl;
		}
		g_nFrames++;


		if (!isAnimCameraValid)
		{
			animCamera = loadCamera(idFrame);
			isAnimCameraValid = true;
		}

		if (doAnimCamera)
		{
			 eyeCamera.eyePosition = animCamera.eyePosition;
			 eyeCamera.lookAt = animCamera.lookAt;
		}


		// Clear frame buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up viewing transformation, looking down -Z axis
		glLoadIdentity();

		// set camera and lookat position
		gluLookAt(eyeCamera.eyePosition.x, eyeCamera.eyePosition.y, eyeCamera.eyePosition.z,
				eyeCamera.lookAt.x, eyeCamera.lookAt.y, eyeCamera.lookAt.z,
				0, 1, 0);

		// Set up the stationary light
		//glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);

		 if (specularActive) {
			 colorNone[0] = 1.0;
			 colorNone[1] = 1.0;
			 colorNone[2] = 1.0;
			 colorNone[3] = 1.0;
		 }


		 	//text Labels
		 	glDisable(GL_LIGHTING);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glColor3f(1.0, 1.0, 1.0);
			if (!doSaveAnimScreenshot)
			{
				//Number Frame
				glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20);
				std::stringstream stringText1;
				stringText1 << "Frame: " << idFrame;
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringText1.str().c_str());

				//FPS
				glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 40);
				std::stringstream stringText2;
				stringText2 << "FPS: " << g_nFPS;
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringText2.str().c_str());

				//Dump camera coordinate
				glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 60);
				std::stringstream stringTextCamCoo;
				stringTextCamCoo << "Camera at: " << eyeCamera.eyePosition.x;
				stringTextCamCoo << ", " << eyeCamera.eyePosition.y;
				stringTextCamCoo << ", " << eyeCamera.eyePosition.z;
				stringTextCamCoo << "  KF:" << (animCamera.keyframe ? "Y" : "-");
				print_bitmap_string(GLUT_BITMAP_HELVETICA_12, stringTextCamCoo.str().c_str());

			}
			//www.larmor.com
			glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) - 150, 20);
			print_bitmap_string(GLUT_BITMAP_HELVETICA_18, "www.larmor.com");
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glEnable(GL_LIGHTING);


		 glMatrixMode(GL_MODELVIEW);
		 glPushMatrix();

		 if (LarmorPhysx::ConfigManager::is_ground_present)
		 {
			 for (int i = -10; i <= 10; ++i)
			 {
				 for (int j = -10; j <= 10; ++j)
				 {
					 float ax = i * 10.0;
					 float ay = j * 10.0;
					 float bx = (i+1) * 10.0;
					 float by = j * 10.0;
					 float cx = (i+1) * 10.0;
					 float cy = (j+1) * 10.0;
					 float dx = i * 10.0;
					 float dy = (j+1) * 10.0;

					 if ( ((i+j) % 2) == 0)
					 {
						 glMaterialfv(GL_FRONT, GL_DIFFUSE, colorRed);
						 glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
						 glColor4fv(colorRed);
					 }
					 else
					 {
						 glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBlue);
						 glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
						 glColor4fv(colorBlue);
					 }

					 glBegin(GL_TRIANGLES);
					    glNormal3d(0.0, -10.0, 0.0);
						glVertex3f( ax, 0.0, ay );
						glVertex3f( bx, 0.0, by );
						glVertex3f( dx, 0.0, dy );
					glEnd();
					glBegin(GL_TRIANGLES);
						glNormal3d(0.0, 10.0, 0.0);
						glVertex3f( dx, 0.0, dy );
						glVertex3f( bx, 0.0, by );
						glVertex3f( ax, 0.0, ay );
					glEnd();

					glBegin(GL_TRIANGLES);
						glNormal3d(0.0, -10.0, 0.0);
						glVertex3f( bx, 0.0, by );
						glVertex3f( cx, 0.0, cy );
						glVertex3f( dx, 0.0, dy );
					glEnd();
					glBegin(GL_TRIANGLES);
						glNormal3d(0.0, 10.0, 0.0);
						glVertex3f( dx, 0.0, dy );
						glVertex3f( cx, 0.0, cy );
						glVertex3f( bx, 0.0, by );
					glEnd();

				 }
			 }
		 }


		 //Render frame idFrame
		 int numObject = 0;
		 DynamicObjectVector dynamicObjectVector = dynamicObjectVectorFrames.at(idFrame);
		 DynamicObjectVector::iterator dynamicObjectVectorIter;
			for(dynamicObjectVectorIter = dynamicObjectVector.begin();
					dynamicObjectVectorIter != dynamicObjectVector.end();
					++dynamicObjectVectorIter)
			{
				numObject++;
				DynamicObject dynamicObject = *dynamicObjectVectorIter;

				glPushMatrix(); //Save last matrix
				glTranslatef(dynamicObject.position.x, dynamicObject.position.y, dynamicObject.position.z);
				float rotationDegrees = dynamicObject.rotationAngle * 180.0 / CONV_PI;
				glRotatef(rotationDegrees, dynamicObject.rotationAxis.x, dynamicObject.rotationAxis.y, dynamicObject.rotationAxis.z);

				std::list<Triangle>::iterator triangleIter;
				TrianglesInfoList::iterator triangleInfoIter;
				for(triangleIter = staticObjectVector.at(dynamicObject.idObj).meshData.first.first.begin(),
					triangleInfoIter = staticObjectVector.at(dynamicObject.idObj).meshData.first.second.begin();
						triangleIter != staticObjectVector.at(dynamicObject.idObj).meshData.first.first.end(),
						triangleInfoIter != staticObjectVector.at(dynamicObject.idObj).meshData.first.second.end();
						++triangleIter,
						++triangleInfoIter)
				{
					Triangle t = *triangleIter;
					TriangleInfo ti = *triangleInfoIter;
					LVector3 tbtv[3];

					for (int i = 0; i < 3; ++i)
					{
						Point tv = t.vertex(i);
						double tx = CGAL::to_double(tv.x());
						double ty = CGAL::to_double(tv.y());
						double tz = CGAL::to_double(tv.z());
						tbtv[i] = LVector3(tx, ty, tz);
					}

					LVector3 normal;
					calcNormal(tbtv[0], tbtv[1], tbtv[2], normal);

					#ifdef USE_TWO_MESH_COLORS
						if (numObject % 2 == 0)
						{
							glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
						}
						else
						{
							glMaterialfv(GL_FRONT, GL_DIFFUSE, colorGray);
						}
					#else
						if ((ti.cutType == CUTTERMESHER_TRIANGLE_NO_CUTTED) ||
							(ti.cutType == CUTTERMESHER_TRIANGLE_CUTTED))
						{
							glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
						}
						else
						{
							glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhiteRed);
						}
					#endif

					glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
					glColor4fv(colorWhite);

					//glColor3f(1.0f, 1.0f, 1.0f);
					glBegin(GL_TRIANGLES);
						glNormal3d(normal.x, normal.y, normal.z);
						glVertex3f( tbtv[0].x, tbtv[0].y, tbtv[0].z );
						glVertex3f( tbtv[1].x, tbtv[1].y, tbtv[1].z );
						glVertex3f( tbtv[2].x, tbtv[2].y, tbtv[2].z );
					glEnd();

					//glColor3f(1.0f, 1.0f, 1.0f);
					glBegin(GL_TRIANGLES);
						glNormal3d(-normal.x, -normal.y, -normal.z);
						glVertex3f( tbtv[2].x, tbtv[2].y, tbtv[2].z );
						glVertex3f( tbtv[1].x, tbtv[1].y, tbtv[1].z );
						glVertex3f( tbtv[0].x, tbtv[0].y, tbtv[0].z );
					glEnd();

				}

				glPopMatrix(); //Recover last saved matrix
			}

		glPopMatrix();


		//visualize animation camera position
		glPushMatrix(); //Save last matrix
		float xac = animCamera.eyePosition.x - animCamera.lookAt.x; //X
		float yac = animCamera.eyePosition.y - animCamera.lookAt.y; //Z
		float zac = animCamera.eyePosition.z - animCamera.lookAt.z; //Y
		float rac = sqrt(xac*xac + yac*yac + zac*zac);
		float tac = acos(yac / rac) * 180.0 / CONV_PI - 90;
		float pac = - atan2(zac, xac) * 180.0 / CONV_PI + 90;
		glTranslatef(animCamera.eyePosition.x, animCamera.eyePosition.y, animCamera.eyePosition.z); //Position
		glRotatef(pac, 0, 1, 0); //Rotation
		glRotatef(tac, 1, 0, 0); //Tilt
		// Wire cone
		glutWireCone(0.5, 2.0, 4, 4);
		// Vector line
		glLineWidth(2.5);
		glBegin(GL_LINES);
			glVertex3f(0.0, 0.0, 2.0);
			glVertex3f(0.0, 0.0, -1.0);
		glEnd();
		glLineWidth(1.0);
		glPopMatrix(); //Recover last saved matrix


		if (!doSaveAnimScreenshot)
		{
			// visualize lookAt position
			glPushMatrix();
				glTranslatef(eyeCamera.lookAt.x, eyeCamera.lookAt.y, eyeCamera.lookAt.z);
				glutSolidSphere(0.2, 8, 8);
			glPopMatrix();
		}


		//Render collision points
		if (viewCollision)
		{
			glPushMatrix();
				glMaterialfv(GL_FRONT, GL_DIFFUSE, colorGreen);
				glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);
				glColor4fv(colorGreen);

				CollisionPointVector collisionPointVector = collisionPointVectorFrames.at(idFrame);

				MapOrderedPairCollisionPoint maxMapCollisionPointPair;
				MapObjectIdCollisionPoint maxMapCollisionPoint;
				LarmorPhysx::getMaxCollisionPoint(collisionPointVector, maxMapCollisionPointPair, maxMapCollisionPoint);
				CollisionPointVector maxCollisionPointForObject = LarmorPhysx::getCollisionPointVectorFromMap(maxMapCollisionPoint);

				CollisionPointVector::iterator collisionPointVectorIter;
				for(collisionPointVectorIter = maxCollisionPointForObject.begin();
						collisionPointVectorIter != maxCollisionPointForObject.end();
						++collisionPointVectorIter)
				{
					CollisionPoint collisionPoint = *collisionPointVectorIter;

					//std::cout << "forceModule: " << collisionPoint.forceModule << std::endl;

					//if (collisionPoint.forceModule > 100.0)
					//{
						std::cout << "Draw contact point: " << collisionPoint.idObj1 << ", " << collisionPoint.idObj2 << " impulse: " << collisionPoint.forceModule << std::endl;

						glPushMatrix();

							if (collisionPoint.idObj1 != 0)
							{
								//TODO: Change and use an hash map id->obj
								for(dynamicObjectVectorIter = dynamicObjectVector.begin();
										dynamicObjectVectorIter != dynamicObjectVector.end();
										++dynamicObjectVectorIter)
								{
									DynamicObject dynamicObject = *dynamicObjectVectorIter;
									if (dynamicObject.idObj == collisionPoint.idObj1)
									{
										LVector3 obj1Position = dynamicObject.position;
										//printf("glTranslatef: (%f, %f, %f)\n", obj1Position.x, obj1Position.y, obj1Position.z );
										glTranslatef(obj1Position.x, obj1Position.y, obj1Position.z);
										float rotationDegrees = dynamicObject.rotationAngle * 180.0 / CONV_PI;
										glRotatef(rotationDegrees, dynamicObject.rotationAxis.x, dynamicObject.rotationAxis.y, dynamicObject.rotationAxis.z);
									}
								}
							}

							glPushMatrix();
								glTranslatef(collisionPoint.contactPoint.x, collisionPoint.contactPoint.y, collisionPoint.contactPoint.z);
								glutSolidSphere(0.2, 8, 8);
							glPopMatrix();

							glLineWidth(2.5);
							glBegin(GL_LINES);
								glVertex3f(collisionPoint.contactPoint.x, collisionPoint.contactPoint.y, collisionPoint.contactPoint.z);
								glVertex3f(collisionPoint.contactPoint.x + collisionPoint.collisionDirection.x/10.0,
											collisionPoint.contactPoint.y + collisionPoint.collisionDirection.y/10.0,
											collisionPoint.contactPoint.z + collisionPoint.collisionDirection.z/10.0);
							glEnd();
							glLineWidth(1.0);

						glPopMatrix();
					//}
				}
			glPopMatrix();
		}

		// Make sure changes appear onscreen
		glutSwapBuffers();

		//Save animation screenshot
		if (doSaveAnimScreenshot)
		{
			save_bmp_screenshot();
		}

	}

	void animate()
	{
		//framerate limit
		time_now = clock();
		if (time_now - movement_timer > (CLOCKS_PER_SEC / (LarmorPhysx::ConfigManager::steps_per_second * 1.0)))
		{
			//std::cout << "s: " << (time_now - movement_timer) << std::endl;
			movement_timer = time_now;

			if (doAnim)
			{
				//idFrame++;
				idFrame +=  LarmorPhysx::ConfigManager::frames_per_step_animationviewer;
				if (idFrame > LarmorPhysx::ConfigManager::total_anim_steps)
				{
					idFrame = 0;
					std::cout << "Restart Animation from frame 0" << std::endl;
				}
				//std::cout << "animate idFrame: " << idFrame << std::endl;
				isAnimCameraValid = false;
				glutPostRedisplay();
			}

		}
	}


	void initRenderer()
	{
		/*
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glShadeModel(GL_SMOOTH);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		*/

		//initialize the mouse state
		MouseState.leftButton = MouseState.rightButton = MouseState.middleButton = false;
		MouseState.x = MouseState.y = 0;

		//init eye camera
		eyeCamera.eyePosition = LVector3(40.0, 40.0, 0.0);
		eyeCamera.lookAt = LVector3(0.0, 0.0, 0.0);

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

	void handleMouseState(int button, int state, int x, int y)
	{
		//printf("Mouse button: %d\n", button);

		// update the button state
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
	}

	void handleMouseMove(int x, int y)
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
			/*
			// rotate around lookat horizontal axes
			float rotZ = -yDelta * DEGREES_PER_PIXEL_LPV;
			float xc = eyeCamera.eyePosition.x;
			float yc = eyeCamera.eyePosition.y;
			float zc = eyeCamera.eyePosition.z;
			float xct = xc * cos(rotZ*CONV_PI/180.0) - yc * sin(rotZ*CONV_PI/180.0);
			float yct = xc * sin(rotZ*CONV_PI/180.0) + yc * cos(rotZ*CONV_PI/180.0);
			float zct = zc;
			eyeCamera.eyePosition.x = xct;
			eyeCamera.eyePosition.y = yct;
			eyeCamera.eyePosition.z = zct;
			*/
			/*
			// rotate around lookat vertical axes
			float rotY = xDelta * DEGREES_PER_PIXEL_LPV;
			xc = eyeCamera.eyePosition.x;
			yc = eyeCamera.eyePosition.y;
			zc = eyeCamera.eyePosition.z;
			xct = zc * sin(rotY*CONV_PI/180.0) + xc * cos(rotY*CONV_PI/180.0);
			yct = yc;
			zct = zc * cos(rotY*CONV_PI/180.0) - xc * sin(rotY*CONV_PI/180.0);
			eyeCamera.eyePosition.x = xct;
			eyeCamera.eyePosition.y = yct;
			eyeCamera.eyePosition.z = zct;
			*/

			float xac = eyeCamera.eyePosition.x - eyeCamera.lookAt.x; //X
			float yac = eyeCamera.eyePosition.y - eyeCamera.lookAt.y; //Z
			float zac = eyeCamera.eyePosition.z - eyeCamera.lookAt.z; //Y
			float rac = sqrt(xac*xac + yac*yac + zac*zac);
			float tac = acos(yac / rac); //Tilt
			float pac = atan2(zac, xac); //Rotation
			float rotY = -xDelta * DEGREES_PER_PIXEL_ROT * CONV_PI/180.0;
			float rotZ = yDelta * DEGREES_PER_PIXEL_TLT * CONV_PI/180.0;
			rotY += pac;
			rotZ += tac;
			if (rotZ < 0 )
				rotZ = 0.001;
			if (rotZ > CONV_PI )
				rotZ = CONV_PI - 0.001;
			eyeCamera.eyePosition.x = rac * sin(rotZ) * cos(rotY) + eyeCamera.lookAt.x;
			eyeCamera.eyePosition.y = rac * cos(rotZ) + eyeCamera.lookAt.y;
			eyeCamera.eyePosition.z = rac * sin(rotZ) * sin(rotY) + eyeCamera.lookAt.z;

		}

		// zoom
		else if(!MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
		{
			// zoom
			float tranR = yDelta * 0.01;
			// save the eye position coordinates
			float xt = eyeCamera.eyePosition.x;
			float yt = eyeCamera.eyePosition.y;
			float zt = eyeCamera.eyePosition.z;
			// move the eyePosition
			eyeCamera.eyePosition.x += (eyeCamera.eyePosition.x - eyeCamera.lookAt.x) * tranR;
			eyeCamera.eyePosition.y += (eyeCamera.eyePosition.y - eyeCamera.lookAt.y) * tranR;
			eyeCamera.eyePosition.z += (eyeCamera.eyePosition.z - eyeCamera.lookAt.z) * tranR;
			// move the lookAt
			eyeCamera.lookAt.x += (xt - eyeCamera.lookAt.x) * tranR;
			eyeCamera.lookAt.y += (yt - eyeCamera.lookAt.y) * tranR;
			eyeCamera.lookAt.z += (zt - eyeCamera.lookAt.z) * tranR;
		}

		// if we need to move the lookat position (left and right buttons are down)
		//if(!MouseState.leftButton && !MouseState.rightButton && MouseState.middleButton)
		if(MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
		{
			float xac = eyeCamera.lookAt.x - eyeCamera.eyePosition.x; //X
			float yac = eyeCamera.lookAt.y - eyeCamera.eyePosition.y; //Z
			float zac = eyeCamera.lookAt.z - eyeCamera.eyePosition.z; //Y
			float rac = sqrt(xac*xac + yac*yac + zac*zac);
			float tac = acos(yac / rac); //Tilt
			float pac = atan2(zac, xac); //Rotation
			float rotY = -xDelta * DEGREES_PER_PIXEL_ROT * CONV_PI/180.0;
			float rotZ = yDelta * DEGREES_PER_PIXEL_TLT * CONV_PI/180.0;
			rotY += pac;
			rotZ += tac;
			if (rotZ < 0 )
				rotZ = 0.001;
			if (rotZ > CONV_PI )
				rotZ = CONV_PI - 0.001;
			eyeCamera.lookAt.x = rac * sin(rotZ) * cos(rotY) + eyeCamera.eyePosition.x;
			eyeCamera.lookAt.y = rac * cos(rotZ) + eyeCamera.eyePosition.y;
			eyeCamera.lookAt.z = rac * sin(rotZ) * sin(rotY) + eyeCamera.eyePosition.z;
		}

		glutPostRedisplay();

	}

	void handleKeyboard(unsigned char c, int x, int y)
	{
		//printf("Keyboard: %d %d %d\n", c, x, y);
		if (c == 113) {
			printf("Quit!!\n");
			exit(0);
		}

		//save camera position
		if(c == char('c')) {
			printf("Camera From %f, %f, %f look at %f, %f, %f\n",
					eyeCamera.eyePosition.x, eyeCamera.eyePosition.y, eyeCamera.eyePosition.z,
					eyeCamera.lookAt.x, eyeCamera.lookAt.y, eyeCamera.lookAt.z);

			//Create the Camera object and save
			Camera camera;
			camera.idFrame = idFrame;
			camera.eyePosition = LVector3(eyeCamera.eyePosition.x, eyeCamera.eyePosition.y, eyeCamera.eyePosition.z);
			camera.lookAt = LVector3(eyeCamera.lookAt.x, eyeCamera.lookAt.y, eyeCamera.lookAt.z);
			camera.keyframe = true;
			//Save camera
			saveCamera(camera);

			//Invalid camera so it will be in the new position
			isAnimCameraValid = false;
			glutPostRedisplay();
		}

		//delete camera position
		if(c == char('r')) {
			cout << "Delete camera idFrame:" << idFrame << endl;

			//actually simple override the camera file with a camera object with keyframe false
			Camera camera;
			camera.idFrame = idFrame;
			camera.eyePosition = LVector3(0.0, 0.0, 0.0);
			camera.lookAt = LVector3(0.0, 0.0, 0.0);
			camera.keyframe = false;
			//Save camera
			saveCamera(camera);

			//Invalid camera so it will be in the new position
			isAnimCameraValid = false;
			glutPostRedisplay();
		}

		//computeSplineCameraPath
		if(c == char('p')) {
			cout << "Start compute Camera path...." << endl;

			computeCubicSplineCameraPath();

			cout << "End compute Camera path" << endl;

			//Invalid camera so it will be in the new position
			isAnimCameraValid = false;
			glutPostRedisplay();
		}

		if(c == char('s')) {
			eyeCamera.lookAt.x += 0.3;
			glutPostRedisplay();
		}

		if(c == char('a')) {
			eyeCamera.lookAt.x -= 0.3;
			glutPostRedisplay();
		}

		if(c == char('w')) {
			eyeCamera.lookAt.y += 0.3;
			glutPostRedisplay();
		}

		if(c == char('z')) {
			eyeCamera.lookAt.y -= 0.3;
			glutPostRedisplay();
		}

		if(c == char('x')) {
			eyeCamera.lookAt.z += 0.3;
			glutPostRedisplay();
		}

		if(c == char('c')) {
			eyeCamera.lookAt.z -= 0.3;
			glutPostRedisplay();
		}
	}

	void processSpecialKeys(int key, int x, int y)
	{
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
			case GLUT_KEY_F4 :
					viewCollision = !viewCollision;
					glutPostRedisplay();
					break;
			case GLUT_KEY_F5 :
					doSaveAnimScreenshot = !doSaveAnimScreenshot;
					break;
			case GLUT_KEY_F6 :
					save_bmp_screenshot();
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
			case GLUT_KEY_F11 :
					doAnim = !doAnim;
					break;
			case GLUT_KEY_F12 :
					doAnimCamera = !doAnimCamera;
					glutPostRedisplay();
					break;

			case GLUT_KEY_LEFT :
					if (idFrame == 0)
					{
						idFrame = LarmorPhysx::ConfigManager::total_anim_steps;
						std::cout << "Restart Animation from frame: " << LarmorPhysx::ConfigManager::total_anim_steps << std::endl;
					}
					else
					{
						idFrame--;
					}
					isAnimCameraValid = false;
					glutPostRedisplay();
					break;
			case GLUT_KEY_RIGHT :
					idFrame++;
					if (idFrame > LarmorPhysx::ConfigManager::total_anim_steps)
					{
						idFrame = 0;
						std::cout << "Restart Animation from frame 0" << std::endl;
					}
					isAnimCameraValid = false;
					glutPostRedisplay();
					break;
		}

	}

	void reshapeWindow(int width, int height)
	{
		//std::cout << "reshapeWindow" << std::endl;
		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float aspect = (float)width/(float)height;
		//gluPerspective(40.0f, aspect, 0.0f, 10000.0f);
		gluPerspective(65.0f, aspect, 0.001f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);
	}


	void run_animation_viewer()
	{
		int argc = 1;
		char *argv[1];
		argv[0] = "test.exe";

		loadAnimation();

		glutInit(&argc, argv);

		glutInitWindowSize(1280,720); //720x480
		glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
		glutCreateWindow("LarmorPhysxViewer :: Larmor");

		initRenderer();

		glutDisplayFunc(draw);
		glutReshapeFunc(reshapeWindow);
		glutMouseFunc(handleMouseState);
		glutMotionFunc(handleMouseMove);
		glutSpecialFunc(processSpecialKeys);
		glutKeyboardFunc(handleKeyboard);
		glutIdleFunc(animate);
		glutMainLoop();

	}


}

