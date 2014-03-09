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

#include "core_classes.h"
#include "CubicSpline.h"

#define SERIAL_EXTENSION_FILENAME ".lsr"
#define STATIC_OBJECT_FILE_NAME "lphsx_so_"
#define FRAME_FILE_NAME "lphsx_fr_"
#define CAMERA_FILE_NAME "lphsx_cam_"

namespace LarmorPhysx
{

	void StaticObject::calculateVolumeMassInertia()
	{
		std::vector<double> volumeInertia;

		if (isConvex)
		{
			//The object is convex so we can use the tetrahedras build with the mesh triangles and the origin
			volumeInertia = internalVolumeAndInertiaMeshConvex(meshData.first.first, Point(0.0, 0.0, 0.0));
		}
		else
		{
			//The object is not-convex so we have to use the volume Tetrahedralization.
			//  Call 2 times internalVolumeAndInertiaMesh using for each time a smallest facetDistance if the return volume is 0.0
			double minBBDimension = minimumBoundingDimension(meshData.first.first);
			unsigned int numCycle = 0;
			int multipleValue = 1;
			do
			{
				std::cout << "StaticObject::calculateVolumeMassInertia: using facetDistance:" << (ConfigManager::facet_distance_coef * multipleValue) << std::endl;
				volumeInertia = internalVolumeAndInertiaMesh(meshData.first.first, minBBDimension / (ConfigManager::facet_distance_coef * multipleValue));
				numCycle++;
				multipleValue *= 10;
			} while ((volumeInertia.at(0) == 0.0) && (numCycle < 2));

			//If the volume is still < 0.05 we use the internalVolumeAndInertiaMeshConvex that can approximate the non-convex object volume and inertia
			if (volumeInertia.at(0) < 0.05)
			{
				volumeInertia = internalVolumeAndInertiaMeshConvex(meshData.first.first, Point(0.0, 0.0, 0.0));
			}
		}
//TODO: fix for nan number
		//Populate the object properties: volume, inertia and mass
		volume = volumeInertia.at(0);
		inertia = LVector3(volumeInertia.at(1) * density, volumeInertia.at(2) * density, volumeInertia.at(3) * density);
		mass = volume * density;
	}


	void saveStaticObject(StaticObject &staticObject, const char * filename)
	{
		std::cout << "saveStaticObject in file: " << filename << std::endl;
		std::ofstream fileIn(filename, std::ios::out | std::ios::binary);
		boost::archive::binary_oarchive_pointsmap oa(fileIn);
		oa & staticObject;
		fileIn.close();
	}

	StaticObject loadStaticObject(const char * filename)
	{
		std::cout << "loadStaticObject in file: " << filename << std::endl;
		StaticObject staticObject;
		std::ifstream fileOut(filename, std::ios::in | std::ios::binary);
		if (!fileOut.good())
		{
			throw LarmorException(std::string("LarmorPhysx::loadStaticObject: file ")+std::string(filename)+std::string(" doesn't exist!"));
		}

		boost::archive::binary_iarchive ia(fileOut);
		ia & staticObject;
		fileOut.close();
		return staticObject;
	}

	void saveStaticObject(StaticObject &staticObject)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << STATIC_OBJECT_FILE_NAME << staticObject.idObj << SERIAL_EXTENSION_FILENAME;
		saveStaticObject(staticObject, fileName.str().c_str());
	}

	StaticObject loadStaticObject(int idObj)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << STATIC_OBJECT_FILE_NAME << idObj << SERIAL_EXTENSION_FILENAME;
		return loadStaticObject(fileName.str().c_str());
	}


	void saveFrame(Frame &frame, const char * filename)
	{
		std::cout << "saveFrame in file: " << filename << std::endl;
		std::ofstream fileIn(filename, std::ios::out | std::ios::binary);
		boost::archive::binary_oarchive_pointsmap oa(fileIn);
		oa & frame;
		fileIn.close();
	}

	Frame loadFrame(const char * filename)
	{
		std::cout << "loadFrame in file: " << filename << std::endl;
		Frame frame;
		std::ifstream fileOut(filename, std::ios::in | std::ios::binary);
		if (!fileOut.good())
		{
			throw LarmorException(std::string("LarmorPhysx::loadFrame: file ")+std::string(filename)+std::string(" doesn't exist!"));
		}

		boost::archive::binary_iarchive ia(fileOut);
		ia & frame;
		fileOut.close();
		return frame;
	}

	void saveFrame(Frame &frame)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << FRAME_FILE_NAME << frame.idFrame << SERIAL_EXTENSION_FILENAME;
		saveFrame(frame, fileName.str().c_str());
	}

	void savePFrame(Frame &frame)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << FRAME_FILE_NAME << "P" << frame.idFrame << SERIAL_EXTENSION_FILENAME;
		saveFrame(frame, fileName.str().c_str());
	}

	Frame loadFrame(int idFrame)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << FRAME_FILE_NAME << idFrame << SERIAL_EXTENSION_FILENAME;
		return loadFrame(fileName.str().c_str());
	}

	Frame loadPFrame(int idFrame)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << FRAME_FILE_NAME << "P" << idFrame << SERIAL_EXTENSION_FILENAME;
		return loadFrame(fileName.str().c_str());
	}


	void saveCamera(Camera &camera, const char * filename)
	{
		std::cout << "saveCamera in file: " << filename << std::endl;
		std::ofstream fileIn(filename, std::ios::out | std::ios::binary);
		boost::archive::binary_oarchive_pointsmap oa(fileIn);
		oa & camera;
		fileIn.close();
	}

	Camera loadCamera(const char * filename)
	{
		std::cout << "loadCamera in file: " << filename << std::endl;
		Camera camera;
		std::ifstream fileOut(filename, std::ios::in | std::ios::binary);
		if (!fileOut.good())
		{
			std::cout << "loadCamera not found: " << filename << std::endl;
			camera.idFrame = 0;
			camera.eyePosition = LVector3(0.0, 0.0, 0.0);
			camera.lookAt = LVector3(0.0, 0.0, 0.0);
			camera.keyframe = false;
		}
		else
		{
			boost::archive::binary_iarchive ia(fileOut);
			ia & camera;
			fileOut.close();
		}

		return camera;
	}

	void saveCamera(Camera &camera)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << CAMERA_FILE_NAME << camera.idFrame << SERIAL_EXTENSION_FILENAME;
		saveCamera(camera, fileName.str().c_str());
	}

	Camera loadCamera(int idFrame)
	{
		std::stringstream fileName;
		fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << CAMERA_FILE_NAME << idFrame << SERIAL_EXTENSION_FILENAME;
		return loadCamera(fileName.str().c_str());
	}

	//compute the camera path using the camera keyframes
	void computeCubicSplineCameraPath()
	{
		//Build the cubic splines
		CubicSpline splineEyePosition;
		CubicSpline splineLookAt;
		vector<int> keyIdFrame;
		//Add points CubicSpline
		for (unsigned int fc = 0; fc <= LarmorPhysx::ConfigManager::total_anim_steps; ++fc)
		{
			Camera cam = loadCamera(fc);
			if(cam.keyframe)
			{
				cout << "Add point to camera path: frame: " << fc << endl;
				splineEyePosition.addPoint(cam.eyePosition);
				splineLookAt.addPoint(cam.lookAt);
				keyIdFrame.push_back(fc);
			}
		}
		//compute paths
		splineEyePosition.compute();
		splineLookAt.compute();

		//draw camera path getting the points from the spline objects
		for(int kf = 0; kf < keyIdFrame.size()-1; kf++)
		{
			int fc0 = keyIdFrame.at(kf);
			int fc1 = keyIdFrame.at(kf+1);
			int framesInterval = fc1 - fc0;
			double frameStep = 1.0L / framesInterval;
			double cubicPos = 0.0;

			cout << "interpolation from keyframe: " << fc0 << endl;
			for(int pf = fc0+1; pf < fc1; pf++)
			{
				cout << "interpolation frame: " << pf << endl;
				cubicPos += frameStep;
				LVector3 eyePosition = splineEyePosition.getPoint(kf, cubicPos);
				LVector3 lookAt = splineLookAt.getPoint(kf, cubicPos);

				//Create the Camera object and save
				Camera camera;
				camera.idFrame = pf;
				camera.eyePosition = eyePosition;
				camera.lookAt = lookAt;
				camera.keyframe = false;
				//Save camera
				saveCamera(camera);
			}
			cout << "interpolation to keyframe: " << fc1 << endl;
		}
	}


	const std::string currentDateTime()
	{
	    time_t now = time(0);
	    struct tm tstruct;
	    char buf[80];
	    tstruct = *localtime(&now);
	    // Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	    // for more information about date/time format
	    strftime(buf, sizeof(buf), "%d-%m-%Y.%X", &tstruct);
	    return buf;
	}

	void createNotExistsDir(const char * dirname)
	{
		struct stat s;
		int err = stat(dirname, &s);
		if(err == -1)
		{
			if ( errno == ENOENT )
			{
				//No such file or directory
				std::cout << "Create Dir: " << dirname << std::endl;
				if (mkdir(dirname) == -1)
				{
					std::cout << "Fatal Error creating the dir: " << dirname << std::endl;
					exit(-1);
				}
			}
			else
			{
				std::cout << "createNotExistsDir undefinited error Dir: " << dirname << std::endl;
				exit(-1);
			}
		}
		else
		{
			if(S_ISDIR(s.st_mode))
			{
				//it's a dir
				std::cout << "Using dir: " << dirname << std::endl;
			}
			else
			{
				//exists but is not dir
				std::cout << "Fatal Error: dir exists but is not at dir: " << dirname << std::endl;
				exit(-1);
			}
		}
	}

	void getMaxCollisionPoint(CollisionPointVector& collisionPointVector,
				MapOrderedPairCollisionPoint& maxMapCollisionPointPair, MapObjectIdCollisionPoint& maxMapCollisionPoint)
	{

		//Get only the maximum collision for each couple and of each idObj
		CollisionPointVector::iterator collisionPointVectorIterX;
		for(collisionPointVectorIterX = collisionPointVector.begin();
				collisionPointVectorIterX != collisionPointVector.end();
				++collisionPointVectorIterX)
		{
			CollisionPoint cp = *collisionPointVectorIterX;

			//Max for objects couple
			MapOrderedPairCollisionPoint::iterator foundPair = maxMapCollisionPointPair.find(PairUnsignedInt(cp.idObj1, cp.idObj2));
			if (foundPair == maxMapCollisionPointPair.end())
			{
				//CollisionPoint is not found in map
				maxMapCollisionPointPair.insert(PairCollisionPoint(PairUnsignedInt(cp.idObj1, cp.idObj2), cp));
				//std::cout << "TEST NOT FOUND: " << cp.idObj1 << ", " << cp.idObj2 << " new:" << cp.forceModule << std::endl;
			}
			else
			{
				CollisionPoint cpMax = foundPair->second;
				if (cp.forceModule > cpMax.forceModule )
				{
					maxMapCollisionPointPair.erase(foundPair);
					maxMapCollisionPointPair.insert(PairCollisionPoint(PairUnsignedInt(cp.idObj1, cp.idObj2), cp));
					//std::cout << "TEST MAX: " << cp.idObj1 << ", " << cp.idObj2 << " old:" << cpMax.forceModule << " new:" << cp.forceModule << std::endl;
				}
			}

			//Max for object
			MapObjectIdCollisionPoint::iterator foundCollision = maxMapCollisionPoint.find(cp.idObj1);
			if (foundCollision == maxMapCollisionPoint.end())
			{
				//cp.idObj1 is not found in map
				maxMapCollisionPoint.insert(PairObjectIdCollisionPoint(cp.idObj1, cp));
				//std::cout << "TEST NOT FOUND: " << cp.idObj1 << " new:" << cp.forceModule << std::endl;
			}
			else
			{
				CollisionPoint cpMax = foundCollision->second;
				if (cp.forceModule > cpMax.forceModule )
				{
					maxMapCollisionPoint.erase(foundCollision);
					maxMapCollisionPoint.insert(PairObjectIdCollisionPoint(cp.idObj1, cp));
					//std::cout << "TEST MAX: " << cp.idObj1 << " old:" << cpMax.forceModule << " new:" << cp.forceModule << std::endl;
				}
			}
		}
	}


}
