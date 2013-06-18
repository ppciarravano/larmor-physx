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

#include "animation_viewer.h"

#define DEGREES_PER_PIXEL_LPV 0.6f
#define UNITS_PER_PIXEL_LPV 0.5f
#define CONV_PI  3.14159265358979323846f

//#define USE_TWO_MESH_COLORS

namespace LarmorPhysxViewer
{

	unsigned int idFrame = 0;
	unsigned int idScreenFrame = 0;

	static float focusx, focusy, focusz; // the point the eye is looking at
	float hRotation, vRotation, eyeDist;

	// Timing Variables
	int g_nFPS = 0;
	int g_nFrames = 0;
	DWORD g_dwLastFPS = 0;
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
	bool doAutoRotation = false;
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

			std::cout << "  Total collisionPoints size: " << stepFrame.collisionPoints.size() << std::endl;
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
	//C:\LARMOR\video\FFF\ffmpeg\ffmpeg.exe -r 30 -i C:\LARMOR\DEVELOP\WORKSPACE_CDT\RevanoVX\Release\scenes_output_wall3\images\frameimg_%05d.bmp -vcodec libx264 -vpre ./libx264-hq.ffpreset -b 20M -bt 20M test_wall2.mp4
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

		free(pixels);

		idScreenFrame++;
		//std::cout << "idScreenFrame: " << idScreenFrame <<std::endl;
	}

	void draw()
	{

		//FPS counter
		if( GetTickCount() - g_dwLastFPS >= 1000 )
		{
			g_dwLastFPS = GetTickCount();
			g_nFPS = g_nFrames;
			g_nFrames = 0;

			//std::cout << "FPS: " << g_nFPS <<std::endl;
		}
		g_nFrames++;


		// Clear frame buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up viewing transformation, looking down -Z axis
		glLoadIdentity();

		gluLookAt(cos(hRotation*CONV_PI/180.0)*sin(vRotation*CONV_PI/180.0)*eyeDist,
				cos(vRotation*CONV_PI/180.0)*eyeDist,
				sin(hRotation*CONV_PI/180.0)*sin(vRotation*CONV_PI/180.0)*eyeDist,
				focusx, focusy, focusz, 0, 1, 0);

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

				//www.larmor.com
				glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) - 150, 20);
				print_bitmap_string(GLUT_BITMAP_HELVETICA_18, "www.larmor.com");
			}
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
		if (time_now - movement_timer > (CLK_TCK/(LarmorPhysx::ConfigManager::steps_per_second * 1.0)))
		{
			//std::cout << "s: " << (time_now - movement_timer) << std::endl;
			movement_timer = time_now;

			if (doAnim)
			{
				//idFrame++;
				idFrame += 15;
				if (idFrame > LarmorPhysx::ConfigManager::total_anim_steps)
				{
					idFrame = 0;
					std::cout << "Restart Animation from frame 0" << std::endl;
				}
				//std::cout << "animate idFrame: " << idFrame << std::endl;
			}

			if (doAutoRotation)
			{
				hRotation += 0.7;
			}

			if (doAnim || doAutoRotation)
			{
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

		// init focus location location and rotation and distance
		focusx = focusy = focusz = 0.0f;
		hRotation = 0;
		vRotation = 60;
		eyeDist = 50;

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
			// rotate by the delta
			hRotation -= xDelta * DEGREES_PER_PIXEL_LPV;
			vRotation -= yDelta * DEGREES_PER_PIXEL_LPV;
		}

		// zoom
		else if(!MouseState.leftButton && MouseState.rightButton && !MouseState.middleButton)
		{
			// move distance
			eyeDist -= yDelta * UNITS_PER_PIXEL_LPV;
		}

		// if we need to move translate (left and right buttons are down
		if(!MouseState.leftButton && !MouseState.rightButton && MouseState.middleButton)
		{
			// move the focus
			focusx -= yDelta * UNITS_PER_PIXEL_LPV;
			focusz -= -xDelta * UNITS_PER_PIXEL_LPV;
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
					doAutoRotation = !doAutoRotation;
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
					glutPostRedisplay();
					break;
			case GLUT_KEY_RIGHT :
					idFrame++;
					if (idFrame > LarmorPhysx::ConfigManager::total_anim_steps)
					{
						idFrame = 0;
						std::cout << "Restart Animation from frame 0" << std::endl;
					}
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

