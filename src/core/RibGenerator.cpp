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

#include "RibGenerator.h"

#define CONV_PI  3.14159265358979323846f

namespace LarmorPhysx
{

	//to create the video file use:
	//C:\LARMOR\video\FFF\ffmpeg\ffmpeg.exe -r 30 -i C:\LARMOR\DEVELOP\WORKSPACE_CDT\RevanoVX\Release\rib_scene\renders\frame_%05d.tif -vcodec libx264 -vpre ./libx264-hq.ffpreset -b 20M -bt 20M test_rib.mp4
	//or
	//C:\LARMOR\video\FFF\ffmpeg\ffmpeg.exe -r 30 -i C:\LARMOR\DEVELOP\WORKSPACE_CDT\RevanoVX\Release\rib_scene\renders\frame_%05d.tif -vb 30M -b 30M -bt 30M test_srib.mpg

	//Complete RIB Scene generation
	void generateRIB()
	{
		//Create directory structure
		std::cout << "Rib Generator: Create directory structure" << std::endl;
		createNotExistsDir("rib_scene");
		createNotExistsDir("rib_scene\\cameras");
		createNotExistsDir("rib_scene\\lights");
		createNotExistsDir("rib_scene\\objects");
		createNotExistsDir("rib_scene\\renders");
		createNotExistsDir("rib_scene\\scenes");
		createNotExistsDir("rib_scene\\surfaces");

		//Load Frames and rib generation
		unsigned int idFrame = LarmorPhysx::ConfigManager::start_load_frame;
		unsigned int staticIdObjLoaded = 1;
		unsigned int diplayImageNumber = 0;
		MapOrderedPairDynamicObject mapIdFrameIdObjectToDynamicObjects; //Used for fast access in motion blur rib generation

		for (unsigned int frameCounter = 0;
						frameCounter <= LarmorPhysx::ConfigManager::total_anim_steps;
						++frameCounter)
		{
			Frame stepFrame = LarmorPhysx::loadFrame(idFrame);

			DynamicObjectVector::iterator dynamicObjectVectorIter;
			for(dynamicObjectVectorIter = stepFrame.dynamicObjects.begin();
					dynamicObjectVectorIter != stepFrame.dynamicObjects.end();
					++dynamicObjectVectorIter)
			{
				DynamicObject dynamicObject = *dynamicObjectVectorIter;

				if(staticIdObjLoaded == dynamicObject.idObj)
				{
					StaticObject staticObject = LarmorPhysx::loadStaticObject( dynamicObject.idObj );
					//Generate Object
					generateRIBMesh(staticObject);
					staticIdObjLoaded++;
				}

				//Add in mapIdFrameIdObjectToDynamicObjects for motion blur
				if (LarmorPhysx::ConfigManager::rib_motion_blur)
				{
					MapOrderedPairDynamicObject::iterator foundPair = mapIdFrameIdObjectToDynamicObjects.find(PairUnsignedInt(idFrame, dynamicObject.idObj));
					if (foundPair == mapIdFrameIdObjectToDynamicObjects.end())
					{
						mapIdFrameIdObjectToDynamicObjects.insert(PairDynamicObject(PairUnsignedInt(idFrame, dynamicObject.idObj), dynamicObject));
					}
				}

			}


			if (idFrame % (LarmorPhysx::ConfigManager::steps_per_second / LarmorPhysx::ConfigManager::rib_frames_per_second) == 0)
			{
				std::cout << "Rib Generator: generate frame: " << idFrame << " in diplayImageNumber: " << diplayImageNumber << std::endl;

				//Generate Frame
				generateRIBFrame(idFrame, diplayImageNumber);

				//Generate Camera
				generateRIBCamera(idFrame);

				//Generate Rib Scene
				if (LarmorPhysx::ConfigManager::rib_motion_blur)
				{
					generateRIBSceneMotionBlur(idFrame, stepFrame.dynamicObjects, mapIdFrameIdObjectToDynamicObjects);
				}
				else
				{
					generateRIBScene(idFrame, stepFrame.dynamicObjects);
				}

				diplayImageNumber++;
			}

			idFrame++;
		}


	}


	//Generate scene_000nnn.rib
	void generateRIBScene(unsigned int idFrame, DynamicObjectVector &dynamicObjects)
	{
		char bufferFileNumber[10];
		sprintf(bufferFileNumber, "%05d", idFrame);
		std::stringstream fileName;
		fileName << "rib_scene\\scenes\\scene_" << bufferFileNumber << ".rib";
		FILE* oFile = fopen(fileName.str().c_str(), "w");

		fprintf(oFile, "## Scene frame: %s\n\n", bufferFileNumber);

		DynamicObjectVector::iterator dynamicObjectVectorIter;
		for(dynamicObjectVectorIter = dynamicObjects.begin();
				dynamicObjectVectorIter != dynamicObjects.end();
				++dynamicObjectVectorIter)
		{
			DynamicObject dynamicObject = *dynamicObjectVectorIter;
			fprintf(oFile, "# Object id: %d\n", dynamicObject.idObj);
			fprintf(oFile, "TransformBegin\n");

			fprintf(oFile, "Translate %f %f %f\n", dynamicObject.position.x, dynamicObject.position.y, dynamicObject.position.z);
			float rotationDegrees = dynamicObject.rotationAngle * 180.0 / CONV_PI;
			fprintf(oFile, "Rotate %f %f %f %f\n", rotationDegrees, dynamicObject.rotationAxis.x, dynamicObject.rotationAxis.y, dynamicObject.rotationAxis.z);

			fprintf(oFile, "    ReadArchive \"objects/mesh_%d.rib\"\n", dynamicObject.idObj);
			fprintf(oFile, "TransformEnd\n\n");

		}

		fclose (oFile);
	}


	//Generate scene_000nnn.rib
	// with motion blur using MotionBegin and MotionEnd and the previous object positions
	void generateRIBSceneMotionBlur(unsigned int idFrame, DynamicObjectVector &dynamicObjects,
				MapOrderedPairDynamicObject &mapIdFrameIdObjectToDynamicObjects)
	{
		float motionStepInterval = 1.0 / (LarmorPhysx::ConfigManager::rib_frames_per_motion_blur - 1);

		char bufferFileNumber[10];
		sprintf(bufferFileNumber, "%05d", idFrame);
		std::stringstream fileName;
		fileName << "rib_scene\\scenes\\scene_" << bufferFileNumber << ".rib";
		FILE* oFile = fopen(fileName.str().c_str(), "w");

		fprintf(oFile, "## Scene frame: %s\n\n", bufferFileNumber);

		DynamicObjectVector::iterator dynamicObjectVectorIter;
		for(dynamicObjectVectorIter = dynamicObjects.begin();
				dynamicObjectVectorIter != dynamicObjects.end();
				++dynamicObjectVectorIter)
		{
			DynamicObject dynamicObject = *dynamicObjectVectorIter;
			fprintf(oFile, "# Object id: %d\n", dynamicObject.idObj);
			fprintf(oFile, "TransformBegin\n");


			//Find all the previous frames where is present dynamicObject.idObj
			int olderStepObjectPresent = 0;
			for (int motionStep = 0; motionStep < LarmorPhysx::ConfigManager::rib_frames_per_motion_blur; motionStep++)
			{
				int idFrameToSearch = idFrame - motionStep;
				if (idFrameToSearch >= 0)
				{
					MapOrderedPairDynamicObject::iterator foundPair = mapIdFrameIdObjectToDynamicObjects.find(PairUnsignedInt(idFrameToSearch, dynamicObject.idObj));
					if (foundPair != mapIdFrameIdObjectToDynamicObjects.end())
					{
						olderStepObjectPresent = motionStep;
					}
				}
			}


			//MotionBegin Translate
			fprintf(oFile, "MotionBegin [ ");
			for (int motionStep = olderStepObjectPresent; motionStep >= 0; motionStep--)
			{
				float motionStepTime = 1.0 - motionStep * motionStepInterval;
				fprintf(oFile, "%f ", motionStepTime);
			}
			fprintf(oFile, "]\n");
			for (int motionStep = olderStepObjectPresent; motionStep >= 0; motionStep--)
			{
				int idFrameToSearch = idFrame - motionStep;
				MapOrderedPairDynamicObject::iterator foundPair = mapIdFrameIdObjectToDynamicObjects.find(PairUnsignedInt(idFrameToSearch, dynamicObject.idObj));
				if (foundPair != mapIdFrameIdObjectToDynamicObjects.end())
				{
					DynamicObject prevDynamicObject = foundPair->second;
					fprintf(oFile, "	Translate %f %f %f\n", prevDynamicObject.position.x, prevDynamicObject.position.y, prevDynamicObject.position.z);
				}
				else
				{
					std::cout << "Rib Generator: generateRIBSceneMotionBlur FATAL ERROR: previous object not found: frame: " << idFrame << " idObj: " <<  dynamicObject.idObj << std::endl;
					exit(-1);
				}
			}
			fprintf(oFile, "MotionEnd\n");


			//MotionBegin Rotate
			fprintf(oFile, "MotionBegin [ ");
			for (int motionStep = olderStepObjectPresent; motionStep >= 0; motionStep--)
			{
				float motionStepTime = 1.0 - motionStep * motionStepInterval;
				fprintf(oFile, "%f ", motionStepTime);
			}
			fprintf(oFile, "]\n");
			for (int motionStep = olderStepObjectPresent; motionStep >= 0; motionStep--)
			{
				int idFrameToSearch = idFrame - motionStep;
				MapOrderedPairDynamicObject::iterator foundPair = mapIdFrameIdObjectToDynamicObjects.find(PairUnsignedInt(idFrameToSearch, dynamicObject.idObj));
				if (foundPair != mapIdFrameIdObjectToDynamicObjects.end())
				{
					DynamicObject prevDynamicObject = foundPair->second;
					float rotationDegrees = prevDynamicObject.rotationAngle * 180.0 / CONV_PI;
					fprintf(oFile, "	Rotate %f %f %f %f\n", rotationDegrees, prevDynamicObject.rotationAxis.x, prevDynamicObject.rotationAxis.y, prevDynamicObject.rotationAxis.z);
				}
				else
				{
					std::cout << "Rib Generator: generateRIBSceneMotionBlur FATAL ERROR: previous object not found: frame: " << idFrame << " idObj: " <<  dynamicObject.idObj << std::endl;
					exit(-1);
				}
			}
			fprintf(oFile, "MotionEnd\n");

			fprintf(oFile, "    ReadArchive \"objects/mesh_%d.rib\"\n", dynamicObject.idObj);
			fprintf(oFile, "TransformEnd\n\n");

		}

		fclose (oFile);
	}


	//Generate frame_000nnn.rib
	void generateRIBFrame(unsigned int idFrame, unsigned int diplayImageNumber)
	{
		char bufferImageNumber[10];
		sprintf(bufferImageNumber, "%05d", diplayImageNumber);

		char bufferFileNumber[10];
		sprintf(bufferFileNumber, "%05d", idFrame);
		std::stringstream fileName;
		fileName << "rib_scene\\frame_" << bufferFileNumber << ".rib";
		FILE* oFile = fopen(fileName.str().c_str(), "w");

		fprintf(oFile, "## Pier Paolo Ciarravano LarmorPhysics RIB Generator\n");
		fprintf(oFile, "## Scene: %s\n", ConfigManager::scene_output_directory.c_str());
		fprintf(oFile, "## Frame creation date: %s\n", currentDateTime().c_str());
		fprintf(oFile, "## Frame: %s\n\n", bufferFileNumber);

		fprintf(oFile, "FrameBegin 1\n");
		fprintf(oFile, "	ReadArchive \"general.rib\"\n");
		fprintf(oFile, "	Display \"renders/frame_%s.tif\" \"file\" \"rgb\"\n", bufferImageNumber); //bufferFileNumber
		fprintf(oFile, "	ReadArchive \"cameras/camera_%s.rib\"\n", bufferFileNumber);
		fprintf(oFile, "	WorldBegin\n");
		//Uncomment to rotate lights (Used for dragon shatter animation)
		//fprintf(oFile, "	TransformBegin\n"); //For rotate lights
		//fprintf(oFile, "		Rotate %f 0 1 0\n", idFrame/135.0); //For rotate lights
		fprintf(oFile, "		ReadArchive \"lights/lights_1.rib\"\n");
		//fprintf(oFile, "	TransformEnd\n"); //For rotate lights
		fprintf(oFile, "	    ReadArchive \"scenes/scene_%s.rib\"\n", bufferFileNumber);
		if (LarmorPhysx::ConfigManager::is_ground_present)
		{
			fprintf(oFile, "		ReadArchive \"objects/floor_1.rib\"\n");
		}
		fprintf(oFile, "	WorldEnd\n");
		fprintf(oFile, "FrameEnd\n\n");

		fclose (oFile);

	}


	//Generate camera_000nnn.rib
	void generateRIBCamera(unsigned int idFrame)
	{
		char bufferFileNumber[10];
		sprintf(bufferFileNumber, "%05d", idFrame);
		std::stringstream fileName;
		fileName << "rib_scene\\cameras\\camera_" << bufferFileNumber << ".rib";
		FILE* oFile = fopen(fileName.str().c_str(), "w");

		fprintf(oFile, "## Camera frame: %s\n\n", bufferFileNumber);

		//fprintf(oFile, "Translate 0 -15 70\n");
		//fprintf(oFile, "Rotate -30 1 0 0\n");
		//fprintf(oFile, "Rotate 45 0 1 0\n\n");

		fprintf(oFile, "Translate 0 -3 40\n");
		fprintf(oFile, "Rotate -15 1 0 0\n");
		fprintf(oFile, "Rotate 45 0 1 0\n\n");

		//For Dragon shatter animation
		//fprintf(oFile, "Rotate -18 1 0 0\n");
		//fprintf(oFile, "Translate 0 -15 48\n");
		//fprintf(oFile, "Rotate -30 0 1 0\n\n");

		fclose (oFile);
	}


	//Generate objects/mesh_nnn.rib
	void generateRIBMesh(StaticObject &staticObject)
	{
		unsigned int idObj = staticObject.idObj;
		std::stringstream fileName;
		fileName << "rib_scene\\objects\\mesh_" << idObj << ".rib";

		FILE* oFile = fopen(fileName.str().c_str(), "w");
		fprintf(oFile, "# Object id: %d\n\n", idObj);

		//CUTTERMESHER_TRIANGLE_NO_CUTTED
		fprintf(oFile, "# CUTTERMESHER_TRIANGLE_NO_CUTTED, CUTTERMESHER_TRIANGLE_CUTTED\n");
		fprintf(oFile, "AttributeBegin\n");
		fprintf(oFile, "ReadArchive \"surfaces/surface_1.rib\"\n");
		TrianglesList::iterator triangleIter;
		TrianglesInfoList::iterator triangleInfoIter;
		for(triangleIter = staticObject.meshData.first.first.begin(),
			triangleInfoIter = staticObject.meshData.first.second.begin();
				triangleIter != staticObject.meshData.first.first.end() &&
				triangleInfoIter != staticObject.meshData.first.second.end();
				++triangleIter,
				++triangleInfoIter)
		{
			Triangle t = *triangleIter;
			TriangleInfo ti = *triangleInfoIter;

			if ((ti.cutType == CUTTERMESHER_TRIANGLE_NO_CUTTED) ||
					(ti.cutType == CUTTERMESHER_TRIANGLE_CUTTED))
			{
				LVector3 tbtv[3];
				for (int i = 0; i < 3; ++i)
				{
					Point tv = t.vertex(i);
					double tx = CGAL::to_double(tv.x());
					double ty = CGAL::to_double(tv.y());
					double tz = CGAL::to_double(tv.z());
					tbtv[i] = LVector3(tx, ty, tz);
				}
				fprintf(oFile, "Polygon \"P\" [ ");
				fprintf(oFile, "%f %f %f  %f %f %f  %f %f %f",
						tbtv[0].x, tbtv[0].y, tbtv[0].z,
						tbtv[1].x, tbtv[1].y, tbtv[1].z,
						tbtv[2].x, tbtv[2].y, tbtv[2].z);
				fprintf(oFile, " ]\n");
			}

		}
		fprintf(oFile, "AttributeEnd\n\n");

		//CUTTERMESHER_TRIANGLE_IN_CUT_PLANE
		fprintf(oFile, "# CUTTERMESHER_TRIANGLE_IN_CUT_PLANE\n");
		fprintf(oFile, "AttributeBegin\n");
		fprintf(oFile, "ReadArchive \"surfaces/surface_2.rib\"\n");
		for(triangleIter = staticObject.meshData.first.first.begin(),
			triangleInfoIter = staticObject.meshData.first.second.begin();
				triangleIter != staticObject.meshData.first.first.end() &&
				triangleInfoIter != staticObject.meshData.first.second.end();
				++triangleIter,
				++triangleInfoIter)
		{
			Triangle t = *triangleIter;
			TriangleInfo ti = *triangleInfoIter;

			if (ti.cutType == CUTTERMESHER_TRIANGLE_IN_CUT_PLANE)
			{
				LVector3 tbtv[3];
				for (int i = 0; i < 3; ++i)
				{
					Point tv = t.vertex(i);
					double tx = CGAL::to_double(tv.x());
					double ty = CGAL::to_double(tv.y());
					double tz = CGAL::to_double(tv.z());
					tbtv[i] = LVector3(tx, ty, tz);
				}
				fprintf(oFile, "Polygon \"P\" [ ");
				fprintf(oFile, "%f %f %f  %f %f %f  %f %f %f",
						tbtv[0].x, tbtv[0].y, tbtv[0].z,
						tbtv[1].x, tbtv[1].y, tbtv[1].z,
						tbtv[2].x, tbtv[2].y, tbtv[2].z);
				fprintf(oFile, " ]\n");
			}

		}
		fprintf(oFile, "AttributeEnd\n\n");

		fclose (oFile);

	}


}



