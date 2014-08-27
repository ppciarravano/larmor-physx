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

#include "PhysicsCore.h"

namespace LarmorPhysx
{

	/* OK
	//Shatter action algorithm
	std::list<TranslatedMeshData> PhysicsCore::shatterObject(DynamicObject* dynamicObject, CollisionPoint& cpMax)
	{

		//Shatter condition
		if ((cpMax.forceModule >= dynamicObject->staticObject.hardnessCoefficient) && (dynamicObject->shatterLevel < 3))
		{
			std::cout << "Doing Voronoi Shatter: objId: " << dynamicObject->idObj << " collision impulse: " << cpMax.forceModule <<
					" point: ("<< cpMax.contactPoint.x << ", " << cpMax.contactPoint.y << ", " << cpMax.contactPoint.z << ")" << std::endl;

			//Build Complete shattered pieces of mesh
			int numShatterPieces = dynamicObject->staticObject.volume / dynamicObject->staticObject.breakCoefficient;
			numShatterPieces = 3;
			if (numShatterPieces >= 2)
			{
				std::cout << " numShatterPieces: " << numShatterPieces << std::endl;
				//Point parentOriginObj = staticObject.meshData.second;
				//Call shatter function
				std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(dynamicObject->staticObject.meshData.first, numShatterPieces, false);
				std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

				return shatterMeshes;
			}
			else
			{
				std::cout << " ******* NOT SHATTERED numShatterPieces < 2 " << std::endl;
			}
		}

		std::list<TranslatedMeshData> shatterMeshes;
		return shatterMeshes;
	}
	*/


	/*
	 unsigned int idObj1;
	unsigned int idObj2;
	LVector3 contactPoint;
	LReal forceModule;
	LVector3 collisionDirection;

	typedef std::list<Triangle>      TrianglesList;     //Non utilizzato in CutterMesher.cpp
typedef std::list<TriangleInfo>  TrianglesInfoList; //Non utilizzato in CutterMesher.cpp
typedef std::pair<TrianglesList, TrianglesInfoList> MeshData;
typedef std::pair<MeshData,Point> TranslatedMeshData;

LVector3 p = cpMax.contactPoint;
	 */

	/*
	//Shatter action algorithm
	std::list<TranslatedMeshData> PhysicsCore::shatterObject(DynamicObject* dynamicObject, CollisionPoint& cpMax)
	{

		//Shatter condition
		if ((cpMax.forceModule >= dynamicObject->staticObject.hardnessCoefficient) && (dynamicObject->shatterLevel < 1))
		{
			std::cout << "Doing Voronoi Shatter: objId: " << dynamicObject->idObj << " collision impulse: " << cpMax.forceModule <<
					" point: ("<< cpMax.contactPoint.x << ", " << cpMax.contactPoint.y << ", " << cpMax.contactPoint.z << ")" << std::endl;

			//Collision point
			LVector3 p = cpMax.contactPoint;

			//Call shatter function
			std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(dynamicObject->staticObject.meshData.first, 100, false);
			//std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_sphereDistributionOnPoint(dynamicObject->staticObject.meshData.first, 100, KDPoint(p.x, p.y, p.z), 1.0, false);
			std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

			return shatterMeshes;
		}

		std::list<TranslatedMeshData> shatterMeshes;
		return shatterMeshes;
	}
	 */


	//Shatter action algorithm: wood
	std::list<TranslatedMeshData> PhysicsCore::shatterObject(DynamicObject* dynamicObject, CollisionPoint& cpMax)
	{

		//Shatter condition
		if ((cpMax.forceModule >= dynamicObject->staticObject.hardnessCoefficient) && (dynamicObject->shatterLevel < 1))
		{
			std::cout << "Doing Voronoi Shatter: objId: " << dynamicObject->idObj << " collision impulse: " << cpMax.forceModule <<
					" point: ("<< cpMax.contactPoint.x << ", " << cpMax.contactPoint.y << ", " << cpMax.contactPoint.z << ")" << std::endl;

			//Collision point
			LVector3 p = cpMax.contactPoint;

			//std::list<KDPoint> points;
			//for (int y = -10; y <= 10; ++ y) {
			//	double number = (rand() % 10) * 0.02;
			//	KDPoint vp(0.0, y, p.z + number);
			//	points.push_back(vp);
			//}

			std::list<KDPoint> points;
			for (int y = -10; y <= 10; y += 4) {
				double number = (rand() % 10) * 0.05 / 20.0;
				KDPoint vp(0.0, y / 20.0 , p.z + number);
				points.push_back(vp);
			}

			//Call shatter function
			//std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(dynamicObject->staticObject.meshData.first, 10, false);
			//std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_sphereDistributionOnPoint(dynamicObject->staticObject.meshData.first, 100, KDPoint(p.x, p.y, p.z), 10.0, false);
			std::list<MeshData> shatterMeshesNoCentered = voronoiShatter(dynamicObject->staticObject.meshData.first, points);
			std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

			return shatterMeshes;
		}
		/*
		else if ( (cpMax.forceModule >= 3.0) &&
				((dynamicObject->shatterLevel == 1) || (dynamicObject->shatterLevel == 2)) )
		{
			std::cout << "Doing Voronoi Shatter: objId: " << dynamicObject->idObj << " collision impulse: " << cpMax.forceModule <<
					" point: ("<< cpMax.contactPoint.x << ", " << cpMax.contactPoint.y << ", " << cpMax.contactPoint.z << ")" << std::endl;

			//Call shatter function
			std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(dynamicObject->staticObject.meshData.first, 2, false);
			std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

			return shatterMeshes;
		}
		 */
		std::list<TranslatedMeshData> shatterMeshes;
		return shatterMeshes;
	}


}



