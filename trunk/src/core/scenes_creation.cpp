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

#include "scenes_creation.h"

namespace LarmorPhysx
{

	//Generate exploding scene for demo
	// Explode all scene object from world origin (0,0,0)
	void generateExplodingScene()
	{
		//Load initial Frame
		Frame stepFrame = LarmorPhysx::loadFrame(0);

		for (unsigned int frameCounter = 1;
			frameCounter <= LarmorPhysx::ConfigManager::total_anim_steps;
			++frameCounter)
		{
			Frame frame;
			frame.idFrame = frameCounter;
			float dist = 0.01 * frameCounter;

			DynamicObjectVector::iterator dynamicObjectVectorIter;
			for(dynamicObjectVectorIter = stepFrame.dynamicObjects.begin();
					dynamicObjectVectorIter != stepFrame.dynamicObjects.end();
					++dynamicObjectVectorIter)
			{
				DynamicObject dobj = *dynamicObjectVectorIter;
				//StaticObject sobj = LarmorPhysx::loadStaticObject( dobj.idObj );

				dobj.position.x = dobj.position.x * dist + dobj.position.x;
				dobj.position.y = dobj.position.y * dist + dobj.position.y;
				dobj.position.z = dobj.position.z * dist + dobj.position.z;

				frame.dynamicObjects.push_back(dobj);

			}

			//Save Frame
			LarmorPhysx::saveFrame(frame);
		}

	}


	//Sphere voronoi shatter: use with steps_per_second = 300 and internal_sub_steps = 10
	void createFirstFrame_Scene1()
	{
		std::cout << "createFirstFrame_Scene1 Starting..." << std::endl;

		std::list<Triangle> readedTriangles = readOffMeshToTriangles("mesh_sphere_05.off");
		//std::list<Triangle> readedTriangles = readObjMeshToTriangles("lattice.obj"); //Use Scale 0.03
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = scaleAndCenterMesh(readedTriangles, 2.0);
		//std::list<Triangle> outTriangles = readedTriangles;


		//Add empty sphere
		// Total volume: (4/3 * pi * (2)^3  - 4/3 * pi * (1.8)^3)  = 9.0812
		std::list<Triangle> outTrianglesInner = scaleAndCenterMesh(readedTriangles, 1.8);
		std::list<Triangle>::iterator triangleIter;
		for(triangleIter = outTrianglesInner.begin(); triangleIter != outTrianglesInner.end(); ++triangleIter)
		{
			Triangle t = *triangleIter;
			outTriangles.push_back(t);
		}


		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		//TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		//std::list<MeshData> shatterMeshesNoCentered;
		//shatterMeshesNoCentered.push_back(MeshData(outTriangles, cutInfo));

		//Build Complete shattered pieces of mesh
		//std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(outTriangles, ConfigManager::break_pieces_test, false);
		//Manual method to use randomPointsInConcaveSphere distribution or randomPointsInSphere distribution
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		MeshData meshData(outTriangles, cutInfo);
		std::list<KDPoint> randomPoints = randomPointsInConcaveSphere(ConfigManager::break_pieces_test, 1.85, 1.95);
		std::list<MeshData> shatterMeshesNoCentered = voronoiShatter(meshData, randomPoints);

		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		double testVolume = 0.0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.density = 10.0;
			so.simplifyMesh = TrianglesList();
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 100.0;
			so.calculateVolumeMassInertia();
			testVolume += so.mass / so.density; //Sum the total volume for test
			if (ConfigManager::use_simplified_meshes_for_bullet)
			{
				so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
			}
			//Normalize mass and inertia
			//so.mass *= 100.0; //use 1 or 10 or 100, do some test
			//so.inertia.x *= 100.0;
			//so.inertia.y *= 100.0;
			//so.inertia.z *= 100.0;

			//Prevent static object is mass and inertia is 0
			if (so.mass < 0.1)
				so.mass = 1.0;
			if (so.inertia.x < 0.00001)
				so.inertia.x = 0.001;
			if (so.inertia.y < 0.00001)
				so.inertia.y = 0.001;
			if (so.inertia.z < 0.00001)
				so.inertia.z = 0.001;

			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(cm_x+0.0, cm_y+2.0, cm_z+0.0);//was 12
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(1.0, 0.0, 0.0);
			//dobj.linearVelocity = LVector3(cm_x+0.0, cm_y-100.0, cm_z+0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		std::cout << "------> Total Volume for test:" << testVolume << std::endl;

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//cube voronoi shatter
	void createFirstFrame_Scene2()
	{
		std::cout << "createFirstFrame_Scene2 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		//Use r for a empty box
		for (int r = 0; r < 1; ++r)
		{
			float a = 1.0 - r*0.2;
			float b = 1.0 - r*0.2;
			float c = 1.0 - r*0.2;
			Point a1(-a, b, c);
			Point b1(-a, -b, c);
			Point c1(a, -b, c);
			Point d1(a, b, c);
			Point a2(-a, b, -c);
			Point b2(-a, -b, -c);
			Point c2(a, -b, -c);
			Point d2(a, b, -c);
			readedTriangles.push_back(Triangle(a1,d1,d2));
			readedTriangles.push_back(Triangle(d2,a2,a1));
			readedTriangles.push_back(Triangle(d1,c1,c2));
			readedTriangles.push_back(Triangle(c2,d2,d1));
			readedTriangles.push_back(Triangle(c1,b1,b2));
			readedTriangles.push_back(Triangle(b2,c2,c1));
			readedTriangles.push_back(Triangle(b1,a1,a2));
			readedTriangles.push_back(Triangle(a2,b2,b1));
			readedTriangles.push_back(Triangle(a1,b1,c1));
			readedTriangles.push_back(Triangle(c1,d1,a1));
			readedTriangles.push_back(Triangle(a2,d2,c2));
			readedTriangles.push_back(Triangle(c2,b2,a2));
		}
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		//TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		//std::list<MeshData> shatterMeshesNoCentered;
		//shatterMeshesNoCentered.push_back(MeshData(outTriangles, cutInfo));

		//Build Complete shattered pieces of mesh
		std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(outTriangles, ConfigManager::break_pieces_test, false);

		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.density = 10.0;
			so.simplifyMesh = TrianglesList();
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 100.0;
			so.calculateVolumeMassInertia();
			if (ConfigManager::use_simplified_meshes_for_bullet)
			{
				so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
			}
			//Normalize mass and inertia
			//so.mass *= 100.0;
			//so.inertia.x *= 100.0;
			//so.inertia.y *= 100.0;
			//so.inertia.z *= 100.0;

			//Prevent static object is mass and inertia is 0
			if (so.mass < 0.1)
				so.mass = 1.0;
			if (so.inertia.x < 0.00001)
				so.inertia.x = 0.001;
			if (so.inertia.y < 0.00001)
				so.inertia.y = 0.001;
			if (so.inertia.z < 0.00001)
				so.inertia.z = 0.001;

			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(cm_x+0.0, cm_y+12.0, cm_z+0.0);
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(1.0, 0.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Cubi uno sull'altro che si penetrano
	void createFirstFrame_Scene3()
	{
		std::cout << "createFirstFrame_Scene3 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		float a = 1.0;
		float b = 1.0;
		float c = 1.0;
		Point a1(-a, b, c);
		Point b1(-a, -b, c);
		Point c1(a, -b, c);
		Point d1(a, b, c);
		Point a2(-a, b, -c);
		Point b2(-a, -b, -c);
		Point c2(a, -b, -c);
		Point d2(a, b, -c);
		readedTriangles.push_back(Triangle(a1,d1,d2));
		readedTriangles.push_back(Triangle(d2,a2,a1));
		readedTriangles.push_back(Triangle(d1,c1,c2));
		readedTriangles.push_back(Triangle(c2,d2,d1));
		readedTriangles.push_back(Triangle(c1,b1,b2));
		readedTriangles.push_back(Triangle(b2,c2,c1));
		readedTriangles.push_back(Triangle(b1,a1,a2));
		readedTriangles.push_back(Triangle(a2,b2,b1));
		readedTriangles.push_back(Triangle(a1,b1,c1));
		readedTriangles.push_back(Triangle(c1,d1,a1));
		readedTriangles.push_back(Triangle(a2,d2,c2));
		readedTriangles.push_back(Triangle(c2,b2,a2));

		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		MeshData cubeMeshData = MeshData(outTriangles, cutInfo);
		TranslatedMeshData cubeTranslatedMeshData = TranslatedMeshData(cubeMeshData, Point(0.0, 0.0, 0.0));

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		for (int r = 0; r < 2; ++r)
		{
			//Add cube
			frame.lastCreatedIdObj++;
			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;
			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = cubeTranslatedMeshData;
			so.density = 10.0;
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 100.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			//Mass and inertia
			//so.mass = 80.0 * 100.0;
			//so.inertia.x = 50.0 * 100.0;
			//so.inertia.y = 50.0 * 100.0;
			//so.inertia.z = 50.0 * 100.0;
			//Save staticObject
			LarmorPhysx::saveStaticObject(so);
			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = 1; //All the boxes are in the same group to avoid the restitution brick-to-brick
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(0.0, 2.5*r + 1.0, 0.0);
			dobj.rotationAngle = 0.1 * r;
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);
		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Muro di mattoni e sfera
	void createFirstFrame_Scene4()
	{
		std::cout << "createFirstFrame_Scene4 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		float a = 0.5;
		float b = 0.5;
		float c = 1.0;
		Point a1(-a, b, c);
		Point b1(-a, -b, c);
		Point c1(a, -b, c);
		Point d1(a, b, c);
		Point a2(-a, b, -c);
		Point b2(-a, -b, -c);
		Point c2(a, -b, -c);
		Point d2(a, b, -c);
		readedTriangles.push_back(Triangle(a1,d1,d2));
		readedTriangles.push_back(Triangle(d2,a2,a1));
		readedTriangles.push_back(Triangle(d1,c1,c2));
		readedTriangles.push_back(Triangle(c2,d2,d1));
		readedTriangles.push_back(Triangle(c1,b1,b2));
		readedTriangles.push_back(Triangle(b2,c2,c1));
		readedTriangles.push_back(Triangle(b1,a1,a2));
		readedTriangles.push_back(Triangle(a2,b2,b1));
		readedTriangles.push_back(Triangle(a1,b1,c1));
		readedTriangles.push_back(Triangle(c1,d1,a1));
		readedTriangles.push_back(Triangle(a2,d2,c2));
		readedTriangles.push_back(Triangle(c2,b2,a2));

		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		MeshData cubeMeshData = MeshData(outTriangles, cutInfo);
		TranslatedMeshData cubeTranslatedMeshData = TranslatedMeshData(cubeMeshData, Point(0.0, 0.0, 0.0));

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		bool isFirstBrick = true;
		double fmass = 0.0;
		double finertiax = 0.0;
		double finertiay = 0.0;
		double finertiaz = 0.0;
		for (int w = 0; w < 1; ++w) //different wall in z
			for (int h = 0; h < 20; ++h) //bricks in y
				for (int r = -10; r < 10; ++r) //bricks in x
		{
			//Add brick
			frame.lastCreatedIdObj++;
			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;
			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = so.idObj;
			so.meshData = cubeTranslatedMeshData;
			so.density = 10.0;
			so.breakCoefficient = 0.4;
			so.hardnessCoefficient = 80.0;
			so.simplifyMesh = TrianglesList();
			if (isFirstBrick)
			{
				so.calculateVolumeMassInertia();
				isFirstBrick = false;
				fmass = so.mass;
				finertiax = so.inertia.x;
				finertiay = so.inertia.y;
				finertiaz = so.inertia.z;
			}
			else
			{
				so.mass = fmass;
				so.inertia.x = finertiax;
				so.inertia.y = finertiay;
				so.inertia.z = finertiaz;
			}
			std::cout << "---> Object Mass:" << so.mass
								<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;
			//Mass and inertia
			//so.mass = 80.0 * 100.0;
			//so.inertia.x = 50.0 * 100.0;
			//so.inertia.y = 50.0 * 100.0;
			//so.inertia.z = 50.0 * 100.0;
			//Save staticObject
			LarmorPhysx::saveStaticObject(so);
			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = 1; //All the bricks are in the same group to avoid the restitution brick-to-brick
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(0.0 - w*5, 2*b*h + b, 2*c*r + 0.0 +  (h % 2)*c);
			//dobj.rotationAngle = 0.1 * r;
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);
		}


		//Add sphere
		std::list<Triangle> readedTrianglesSphere = readOffMeshToTriangles("mesh_sphere_05.off");
		std::cout << "readedTrianglesSphere size:" << readedTrianglesSphere.size() << std::endl;
		readedTrianglesSphere = scaleAndCenterMesh(readedTrianglesSphere, 2.0);
		//No voronoi
		TrianglesInfoList cutInfoSphere = createNewTriangleInfoList(readedTrianglesSphere);
		MeshData sphereMeshData = MeshData(readedTrianglesSphere, cutInfoSphere);
		TranslatedMeshData sphereTranslatedMeshData = TranslatedMeshData(sphereMeshData, Point(0.0, 0.0, 0.0));
		//Add to frame
		frame.lastCreatedIdObj++;
		std::cout << "***** Creating Object Sphere N." << frame.lastCreatedIdObj << std::endl;
		//Create StaticObject
		StaticObject so;
		so.idObj = frame.lastCreatedIdObj;
		so.idParentObj = so.idObj;
		so.meshData = sphereTranslatedMeshData;
		so.density = 10.0;
		so.breakCoefficient = 5.0;
		so.hardnessCoefficient = 5000.0;
		so.simplifyMesh = TrianglesList();
		so.calculateVolumeMassInertia();
		std::cout << "---> Object Mass:" << so.mass
								<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;
		//Mass and inertia
		//so.mass = 1000.0 * 100.0;
		//so.inertia.x = 50.0 * 100.0;
		//so.inertia.y = 50.0 * 100.0;
		//so.inertia.z = 50.0 * 100.0;
		//Save staticObject
		LarmorPhysx::saveStaticObject(so);
		//Create DynamicObject
		DynamicObject dobj;
		dobj.idObj = frame.lastCreatedIdObj;
		dobj.idGroup = 2;
		//Add a translation on Y axis of 12.0
		dobj.position = LVector3(30.0, 10.0, 0.0);
		//dobj.rotationAngle = 0.1 * r;
		dobj.rotationAngle = 0.0;
		dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
		dobj.linearVelocity = LVector3(-100.0, 8.0, 0.0);
		dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
		dobj.isDynamic = true;
		//Add in frame
		frame.dynamicObjects.push_back(dobj);


		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Cubi uno adiacente all'altro in alto per valutare collisioni tra facce
	void createFirstFrame_Scene5()
	{
		std::cout << "createFirstFrame_Scene5 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		float a = 1.0;
		float b = 1.0;
		float c = 1.0;
		Point a1(-a, b, c);
		Point b1(-a, -b, c);
		Point c1(a, -b, c);
		Point d1(a, b, c);
		Point a2(-a, b, -c);
		Point b2(-a, -b, -c);
		Point c2(a, -b, -c);
		Point d2(a, b, -c);
		readedTriangles.push_back(Triangle(a1,d1,d2));
		readedTriangles.push_back(Triangle(d2,a2,a1));
		readedTriangles.push_back(Triangle(d1,c1,c2));
		readedTriangles.push_back(Triangle(c2,d2,d1));
		readedTriangles.push_back(Triangle(c1,b1,b2));
		readedTriangles.push_back(Triangle(b2,c2,c1));
		readedTriangles.push_back(Triangle(b1,a1,a2));
		readedTriangles.push_back(Triangle(a2,b2,b1));
		readedTriangles.push_back(Triangle(a1,b1,c1));
		readedTriangles.push_back(Triangle(c1,d1,a1));
		readedTriangles.push_back(Triangle(a2,d2,c2));
		readedTriangles.push_back(Triangle(c2,b2,a2));

		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		MeshData cubeMeshData = MeshData(outTriangles, cutInfo);
		TranslatedMeshData cubeTranslatedMeshData = TranslatedMeshData(cubeMeshData, Point(0.0, 0.0, 0.0));

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		for (int r = 0; r < 2; ++r)
		{
			//Add cube
			frame.lastCreatedIdObj++;
			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;
			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = cubeTranslatedMeshData;
			so.density = 10.0;
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 100.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			//Mass and inertia
			//so.mass = 80.0 * 100.0;
			//so.inertia.x = 50.0 * 100.0;
			//so.inertia.y = 50.0 * 100.0;
			//so.inertia.z = 50.0 * 100.0;
			//Save staticObject
			LarmorPhysx::saveStaticObject(so);
			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(0.0 + 2.0 *r, 10.0, 0.0);
			dobj.rotationAngle = 0.1 * r;
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);
		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Due sfere che si vengono incontro e collidono in (0,y,0)
	void createFirstFrame_Scene6()
	{
		std::cout << "createFirstFrame_Scene6 Starting..." << std::endl;

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;


		//Add sphere
		std::list<Triangle> readedTrianglesSphere = readOffMeshToTriangles("mesh_sphere_05.off");
		std::cout << "readedTrianglesSphere size:" << readedTrianglesSphere.size() << std::endl;
		readedTrianglesSphere = scaleAndCenterMesh(readedTrianglesSphere, 2.0);
		//No voronoi
		TrianglesInfoList cutInfoSphere = createNewTriangleInfoList(readedTrianglesSphere);
		MeshData sphereMeshData = MeshData(readedTrianglesSphere, cutInfoSphere);
		TranslatedMeshData sphereTranslatedMeshData = TranslatedMeshData(sphereMeshData, Point(0.0, 0.0, 0.0));

		//Add to frame
		frame.lastCreatedIdObj++;
		std::cout << "***** Creating Object Sphere N." << frame.lastCreatedIdObj << std::endl;
		//Create StaticObject
		StaticObject so1;
		so1.idObj = frame.lastCreatedIdObj;
		so1.idParentObj = 0;
		so1.meshData = sphereTranslatedMeshData;
		so1.density = 10.0;
		so1.breakCoefficient = 5.0;
		so1.hardnessCoefficient = 100.0;
		so1.simplifyMesh = TrianglesList();
		so1.calculateVolumeMassInertia();
		std::cout << "---> Object Mass:" << so1.mass
							<< " Inertia: (" << so1.inertia.x << ", " << so1.inertia.y << ", " << so1.inertia.z << ")" << std::endl;
		//Mass and inertia
		//so1.mass = 100.0 * 100.0;
		//so1.inertia.x = 50.0 * 100.0;
		//so1.inertia.y = 50.0 * 100.0;
		//so1.inertia.z = 50.0 * 100.0;
		//Save staticObject
		LarmorPhysx::saveStaticObject(so1);
		//Create DynamicObject
		DynamicObject dobj1;
		dobj1.idObj = frame.lastCreatedIdObj;
		dobj1.idGroup = 1;
		//Add a translation on Y axis of 12.0
		dobj1.position = LVector3(30.0, 12.0, 0.0);
		dobj1.rotationAngle = 0.0;
		dobj1.rotationAxis = LVector3(0.0, 1.0, 0.0);
		dobj1.linearVelocity = LVector3(-100.0, 0.0, 0.0);
		dobj1.angularVelocity = LVector3(0.0, 0.0, 0.0);
		dobj1.isDynamic = true;
		//Add in frame
		frame.dynamicObjects.push_back(dobj1);

		//Add to frame
		frame.lastCreatedIdObj++;
		std::cout << "***** Creating Object Sphere N." << frame.lastCreatedIdObj << std::endl;
		//Create StaticObject
		StaticObject so2;
		so2.idObj = frame.lastCreatedIdObj;
		so2.idParentObj = 0;
		so2.meshData = sphereTranslatedMeshData;
		so2.density = 10.0;
		so2.breakCoefficient = 5.0;
		so2.hardnessCoefficient = 100.0;
		so2.simplifyMesh = TrianglesList();
		so2.calculateVolumeMassInertia();
		std::cout << "---> Object Mass:" << so2.mass
							<< " Inertia: (" << so2.inertia.x << ", " << so2.inertia.y << ", " << so2.inertia.z << ")" << std::endl;
		//Mass and inertia
		//so2.mass = 100.0 * 100.0;
		//so2.inertia.x = 50.0 * 100.0;
		//so2.inertia.y = 50.0 * 100.0;
		//so2.inertia.z = 50.0 * 100.0;
		//Save staticObject
		LarmorPhysx::saveStaticObject(so2);
		//Create DynamicObject
		DynamicObject dobj2;
		dobj2.idObj = frame.lastCreatedIdObj;
		dobj2.idGroup = 2;
		//Add a translation on Y axis of 12.0
		dobj2.position = LVector3(-30.0, 12.0, 0.0);
		dobj2.rotationAngle = 0.0;
		dobj2.rotationAxis = LVector3(0.0, 1.0, 0.0);
		dobj2.linearVelocity = LVector3(100.0, 0.0, 0.0);
		dobj2.angularVelocity = LVector3(0.0, 0.0, 0.0);
		dobj2.isDynamic = true;
		//Add in frame
		frame.dynamicObjects.push_back(dobj2);

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//box shattered on a plane
	// Values:
	//	Box: 2, 4, 6
	//	volume: 48
	//	Density: 10
	//	Mass: 480
	//	Inertia: 2080, 1600, 800
	// or
	//	Box: 1, 2, 3
	//	volume: 6
	//	Density: 10
	//	Mass: 60
	//	Inertia: 60.5, 50, 20.5
	// or
	//	Box: 0.5, 1, 1.5
	//	volume: 0.75
	//	Density: 10
	//	Mass: 7.5
	//	Inertia: 2.0, 1.5, 0.7
	void createFirstFrame_Scene7()
	{
		std::cout << "createFirstFrame_Scene7 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		//Use r for a empty box
		for (int r = 0; r < 1; ++r)
		{
			float a = 1.0 - r*0.2;
			float b = 2.0 - r*0.2;
			float c = 3.0 - r*0.2;
			Point a1(-a, b, c);
			Point b1(-a, -b, c);
			Point c1(a, -b, c);
			Point d1(a, b, c);
			Point a2(-a, b, -c);
			Point b2(-a, -b, -c);
			Point c2(a, -b, -c);
			Point d2(a, b, -c);
			readedTriangles.push_back(Triangle(a1,d1,d2));
			readedTriangles.push_back(Triangle(d2,a2,a1));
			readedTriangles.push_back(Triangle(d1,c1,c2));
			readedTriangles.push_back(Triangle(c2,d2,d1));
			readedTriangles.push_back(Triangle(c1,b1,b2));
			readedTriangles.push_back(Triangle(b2,c2,c1));
			readedTriangles.push_back(Triangle(b1,a1,a2));
			readedTriangles.push_back(Triangle(a2,b2,b1));
			readedTriangles.push_back(Triangle(a1,b1,c1));
			readedTriangles.push_back(Triangle(c1,d1,a1));
			readedTriangles.push_back(Triangle(a2,d2,c2));
			readedTriangles.push_back(Triangle(c2,b2,a2));
		}
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//Build Complete shattered pieces of mesh
		std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(outTriangles, 3, false);

		//No voronoi
		//TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		//std::list<MeshData> shatterMeshesNoCentered;
		//shatterMeshesNoCentered.push_back(MeshData(outTriangles, cutInfo));

		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.density = 10.0;
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 100.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			if (ConfigManager::use_simplified_meshes_for_bullet)
			{
				so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
			}
			//Normalize mass and inertia
			//so.mass *= 100.0;
			//so.inertia.x *= 100.0;
			//so.inertia.y *= 100.0;
			//so.inertia.z *= 100.0;

			//Prevent static object is mass and inertia is 0
			if (so.mass < 0.1)
				so.mass = 1.0;
			if (so.inertia.x < 0.00001)
				so.inertia.x = 0.001;
			if (so.inertia.y < 0.00001)
				so.inertia.y = 0.001;
			if (so.inertia.z < 0.00001)
				so.inertia.z = 0.001;

			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());
			btVector3 cmVector(cm_x, cm_y, cm_z);
			dobj.rotationAngle = 0.78539816339; // 0.52; //30 degrees
			btVector3 rotationAxis(1.0, 2.0, 3.0);
			dobj.rotationAxis = getLFromBtVector3(rotationAxis);

			//Position
			btTransform pt(btQuaternion(rotationAxis, dobj.rotationAngle), btVector3(0.0, 0.0, 0.0));
			btVector3 rotatedPosition = pt * cmVector;
			//init translation: Add a translation on Y axis of 12.0
			btVector3 initTranslation(0.0 + rotatedPosition.getX(), 12.0 + rotatedPosition.getY(), 0.0 + rotatedPosition.getZ());
			//Add a translation on Y axis of 12.0
			dobj.position = getLFromBtVector3(initTranslation);

			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;

			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//box on a plane for voronoi PhysicsCore shatter
	void createFirstFrame_Scene8()
	{
		std::cout << "createFirstFrame_Scene8 Starting..." << std::endl;

		//Create Cube
		std::list<Triangle> readedTriangles;

		//Use r for a empty box
		for (int r = 0; r < 1; ++r)
		{
			float a = 1.0 - r*0.2;
			float b = 2.0 - r*0.2;
			float c = 3.0 - r*0.2;
			Point a1(-a, b, c);
			Point b1(-a, -b, c);
			Point c1(a, -b, c);
			Point d1(a, b, c);
			Point a2(-a, b, -c);
			Point b2(-a, -b, -c);
			Point c2(a, -b, -c);
			Point d2(a, b, -c);
			readedTriangles.push_back(Triangle(a1,d1,d2));
			readedTriangles.push_back(Triangle(d2,a2,a1));
			readedTriangles.push_back(Triangle(d1,c1,c2));
			readedTriangles.push_back(Triangle(c2,d2,d1));
			readedTriangles.push_back(Triangle(c1,b1,b2));
			readedTriangles.push_back(Triangle(b2,c2,c1));
			readedTriangles.push_back(Triangle(b1,a1,a2));
			readedTriangles.push_back(Triangle(a2,b2,b1));
			readedTriangles.push_back(Triangle(a1,b1,c1));
			readedTriangles.push_back(Triangle(c1,d1,a1));
			readedTriangles.push_back(Triangle(a2,d2,c2));
			readedTriangles.push_back(Triangle(c2,b2,a2));
		}
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		std::list<MeshData> shatterMeshesNoCentered;
		shatterMeshesNoCentered.push_back(MeshData(outTriangles, cutInfo));

		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.density = 10.0;
			so.breakCoefficient = 0.4;
			so.hardnessCoefficient = 1000.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			if (ConfigManager::use_simplified_meshes_for_bullet)
			{
				so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
			}
			//Normalize mass and inertia
			//so.mass *= 100.0;
			//so.inertia.x *= 100.0;
			//so.inertia.y *= 100.0;
			//so.inertia.z *= 100.0;

			//Prevent static object is mass and inertia is 0
			if (so.mass < 0.1)
				so.mass = 1.0;
			if (so.inertia.x < 0.00001)
				so.inertia.x = 0.001;
			if (so.inertia.y < 0.00001)
				so.inertia.y = 0.001;
			if (so.inertia.z < 0.00001)
				so.inertia.z = 0.001;

			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());
			btVector3 cmVector(cm_x, cm_y, cm_z);
			dobj.rotationAngle = 0.78539816339; // 0.52; //30 degrees
			btVector3 rotationAxis(1.0, 2.0, 3.0);
			dobj.rotationAxis = getLFromBtVector3(rotationAxis);

			//Position
			btTransform pt(btQuaternion(rotationAxis, dobj.rotationAngle), btVector3(0.0, 0.0, 0.0));
			btVector3 rotatedPosition = pt * cmVector;
			//init translation: Add a translation on Y axis of 12.0
			btVector3 initTranslation(0.0 + rotatedPosition.getX(), 12.0 + rotatedPosition.getY(), 0.0 + rotatedPosition.getZ());
			//Add a translation on Y axis of 12.0
			dobj.position = getLFromBtVector3(initTranslation);

			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;

			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Muro solido e sfera
	void createFirstFrame_Scene9()
	{
		std::cout << "createFirstFrame_Scene9 Starting..." << std::endl;

		//Create wall box
		std::list<Triangle> readedTriangles;

		float a = 2.0;
		float b = 10.0;
		float c = 20.0;
		Point a1(-a, b, c);
		Point b1(-a, -b, c);
		Point c1(a, -b, c);
		Point d1(a, b, c);
		Point a2(-a, b, -c);
		Point b2(-a, -b, -c);
		Point c2(a, -b, -c);
		Point d2(a, b, -c);
		readedTriangles.push_back(Triangle(a1,d1,d2));
		readedTriangles.push_back(Triangle(d2,a2,a1));
		readedTriangles.push_back(Triangle(d1,c1,c2));
		readedTriangles.push_back(Triangle(c2,d2,d1));
		readedTriangles.push_back(Triangle(c1,b1,b2));
		readedTriangles.push_back(Triangle(b2,c2,c1));
		readedTriangles.push_back(Triangle(b1,a1,a2));
		readedTriangles.push_back(Triangle(a2,b2,b1));
		readedTriangles.push_back(Triangle(a1,b1,c1));
		readedTriangles.push_back(Triangle(c1,d1,a1));
		readedTriangles.push_back(Triangle(a2,d2,c2));
		readedTriangles.push_back(Triangle(c2,b2,a2));

		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		MeshData cubeMeshData = MeshData(outTriangles, cutInfo);
		TranslatedMeshData cubeTranslatedMeshData = TranslatedMeshData(cubeMeshData, Point(0.0, 0.0, 0.0));

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		{
			//Add box wall
			frame.lastCreatedIdObj++;
			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;
			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = so.idObj;
			so.meshData = cubeTranslatedMeshData;
			so.density = 1000.0;
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 1000.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			//Mass and inertia
			//so.mass = 32000.0;
			//so.inertia.x = 0.0001;
			//so.inertia.y = 0.0001;
			//so.inertia.z = 0.0001;
			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;
			//Save staticObject
			LarmorPhysx::saveStaticObject(so);
			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = 1;
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(0.0, 10.0, 0.0);
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);
		}

		{
			//Add sphere
			std::list<Triangle> readedTrianglesSphere = readOffMeshToTriangles("mesh_sphere_05.off");
			std::cout << "readedTrianglesSphere size:" << readedTrianglesSphere.size() << std::endl;
			readedTrianglesSphere = scaleAndCenterMesh(readedTrianglesSphere, 2.0);
			//No voronoi
			TrianglesInfoList cutInfoSphere = createNewTriangleInfoList(readedTrianglesSphere);
			MeshData sphereMeshData = MeshData(readedTrianglesSphere, cutInfoSphere);
			TranslatedMeshData sphereTranslatedMeshData = TranslatedMeshData(sphereMeshData, Point(0.0, 0.0, 0.0));
			//Add to frame
			frame.lastCreatedIdObj++;
			std::cout << "***** Creating Object Sphere N." << frame.lastCreatedIdObj << std::endl;
			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = so.idObj;
			so.meshData = sphereTranslatedMeshData;
			so.density = 10.0;
			so.breakCoefficient = 5.0;
			so.hardnessCoefficient = 1000.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			//Mass and inertia
			//so.mass = 1000.0 * 100.0;
			//so.inertia.x = 50.0 * 100.0;
			//so.inertia.y = 50.0 * 100.0;
			//so.inertia.z = 50.0 * 100.0;
			//Save staticObject
			LarmorPhysx::saveStaticObject(so);
			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = 2;
			//Add a translation on Y axis of 12.0
			dobj.position = LVector3(30.0, 10.0, 0.0);
			//dobj.rotationAngle = 0.1 * r;
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(0.0, 1.0, 0.0);
			dobj.linearVelocity = LVector3(-100.0, 8.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);
		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//empty sphere on a plane for voronoi PhysicsCore shatter
	void createFirstFrame_Scene10()
	{
		std::cout << "createFirstFrame_Scene10 Starting..." << std::endl;

		std::list<Triangle> readedTriangles = readOffMeshToTriangles("mesh_sphere_05.off");
		//std::list<Triangle> readedTriangles = readObjMeshToTriangles("lattice.obj"); //Use Scale 0.03
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = scaleAndCenterMesh(readedTriangles, 2.0);
		//std::list<Triangle> outTriangles = readedTriangles;

		//Add empty sphere
		// Total volume: (4/3 * pi * (2)^3  - 4/3 * pi * (1.8)^3)  = 9.0812
		std::list<Triangle> outTrianglesInner = scaleAndCenterMesh(readedTriangles, 1.9); //1.8
		std::list<Triangle>::iterator triangleIter;
		for(triangleIter = outTrianglesInner.begin(); triangleIter != outTrianglesInner.end(); ++triangleIter)
		{
			Triangle t = *triangleIter;
			outTriangles.push_back(t);
		}

		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();

		//No voronoi
		TrianglesInfoList cutInfo = createNewTriangleInfoList(outTriangles);
		std::list<MeshData> shatterMeshesNoCentered;
		shatterMeshesNoCentered.push_back(MeshData(outTriangles, cutInfo));

		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.density = 10.0;
			so.breakCoefficient = 0.2;
			so.hardnessCoefficient = 500.0;
			so.simplifyMesh = TrianglesList();
			so.calculateVolumeMassInertia();
			if (ConfigManager::use_simplified_meshes_for_bullet)
			{
				so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
			}

			std::cout << "---> Object Mass:" << so.mass
					<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());
			btVector3 cmVector(cm_x, cm_y, cm_z);
			dobj.rotationAngle = 0.0;
			btVector3 rotationAxis(1.0, 0.0, 0.0);
			dobj.rotationAxis = getLFromBtVector3(rotationAxis);

			//Position
			btTransform pt(btQuaternion(rotationAxis, dobj.rotationAngle), btVector3(0.0, 0.0, 0.0));
			btVector3 rotatedPosition = pt * cmVector;
			//init translation: Add a translation on Y axis of 12.0
			btVector3 initTranslation(0.0 + rotatedPosition.getX(), 12.0 + rotatedPosition.getY(), 0.0 + rotatedPosition.getZ());
			//Add a translation on Y axis of 12.0
			dobj.position = getLFromBtVector3(initTranslation);

			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;

			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}


	//Scene to call with generateExplodingScene for demo exploding
	void createFirstFrame_Scene11()
	{
		std::cout << "createFirstFrame_Scene11 Starting..." << std::endl;


		//Create innested Cubes
		std::list<Triangle> readedTriangles;
		//Use r for a empty box
		for (int r = 0; r < 1; ++r)
		{
			float a = 6.0 - r*0.2;
			float b = 6.0 - r*0.2;
			float c = 6.0 - r*0.2;
			Point a1(-a, b, c);
			Point b1(-a, -b, c);
			Point c1(a, -b, c);
			Point d1(a, b, c);
			Point a2(-a, b, -c);
			Point b2(-a, -b, -c);
			Point c2(a, -b, -c);
			Point d2(a, b, -c);
			readedTriangles.push_back(Triangle(a1,d1,d2));
			readedTriangles.push_back(Triangle(d2,a2,a1));
			readedTriangles.push_back(Triangle(d1,c1,c2));
			readedTriangles.push_back(Triangle(c2,d2,d1));
			readedTriangles.push_back(Triangle(c1,b1,b2));
			readedTriangles.push_back(Triangle(b2,c2,c1));
			readedTriangles.push_back(Triangle(b1,a1,a2));
			readedTriangles.push_back(Triangle(a2,b2,b1));
			readedTriangles.push_back(Triangle(a1,b1,c1));
			readedTriangles.push_back(Triangle(c1,d1,a1));
			readedTriangles.push_back(Triangle(a2,d2,c2));
			readedTriangles.push_back(Triangle(c2,b2,a2));
		}
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;

		std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();


		/*
		//Empty sphere
		std::list<Triangle> readedTriangles = readOffMeshToTriangles("mesh_sphere_05.off");
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;
		std::list<Triangle> outTriangles = scaleAndCenterMesh(readedTriangles, 2.0);
		//std::list<Triangle> outTriangles = readedTriangles;
		//Add empty sphere
		// Total volume: (4/3 * pi * (2)^3  - 4/3 * pi * (1.8)^3)  = 9.0812
		std::list<Triangle> outTrianglesInner = scaleAndCenterMesh(readedTriangles, 1.9);
		std::list<Triangle>::iterator triangleIter;
		for(triangleIter = outTrianglesInner.begin(); triangleIter != outTrianglesInner.end(); ++triangleIter)
		{
			Triangle t = *triangleIter;
			outTriangles.push_back(t);
		}
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();
 	 	*/

		/*
		//Lattice mesh
		std::list<Triangle> readedTriangles = readObjMeshToTriangles("lattice.obj");
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;
		std::list<Triangle> outTriangles = scaleAndCenterMesh(readedTriangles, 0.03);
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();
		/*

		/*
		//Stanford Dragon mesh
		std::list<Triangle> readedTriangles = readPlyMeshToTriangles("dragon_recon\\dragon_vrip_res3.ply"); //dragon_vrip.ply
		std::cout << "readedTriangles size:" << readedTriangles.size() << std::endl;
		std::list<Triangle> outTriangles = scaleAndCenterMesh(readedTriangles, 50);
		//std::list<Triangle> outTriangles = readedTriangles;
		std::cout << "Readed Mesh" << std::endl;
		readedTriangles.clear();
		*/

		//Build Complete shattered pieces of mesh
		std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(outTriangles, ConfigManager::break_pieces_test, false);
		std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

		//Create Frame
		Frame frame;
		frame.idFrame = 0;
		frame.timePosition = 0.0;
		frame.lastCreatedIdObj = 0;

		std::list<TranslatedMeshData>::iterator meshDataInter;
		for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
		{
			TranslatedMeshData meshDataPartTranslated = *meshDataInter;
			frame.lastCreatedIdObj++;

			std::cout << "***** Creating Object N." << frame.lastCreatedIdObj << std::endl;

			//Create StaticObject
			StaticObject so;
			so.idObj = frame.lastCreatedIdObj;
			so.idParentObj = 0;
			so.meshData = meshDataPartTranslated;
			so.simplifyMesh = TrianglesList();

			//Save staticObject
			LarmorPhysx::saveStaticObject(so);

			//Create DynamicObject
			DynamicObject dobj;
			dobj.idObj = frame.lastCreatedIdObj;
			dobj.idGroup = dobj.idObj;
			//Calcolate position
			Point originObj = so.meshData.second;
			double cm_x = CGAL::to_double(originObj.x());
			double cm_y = CGAL::to_double(originObj.y());
			double cm_z = CGAL::to_double(originObj.z());

			dobj.position = LVector3(cm_x+0.0, cm_y+0.0, cm_z+0.0);
			dobj.rotationAngle = 0.0;
			dobj.rotationAxis = LVector3(1.0, 0.0, 0.0);
			dobj.linearVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);
			dobj.isDynamic = true;
			//Add in frame
			frame.dynamicObjects.push_back(dobj);

		}

		//Save Frame
		LarmorPhysx::saveFrame(frame);

	}
}

