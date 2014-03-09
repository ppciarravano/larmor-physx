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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>

#include "shatter/MeshViewer.h"
#include "shatter/CutterMesher.h"
#include "shatter/Custom.h"
#include "shatter/util.h"
#include "shatter/DelaunayVoronoi.h"

#include "core/core_classes.h"
#include "core/PhysicsCore.h"
#include "core/scenes_creation.h"
#include "core/animation_viewer.h"
#include "core/RibGenerator.h"

//using namespace std;
using namespace LarmorPhysx;

int main(int argc, char** argv) {
	std::cout << "Revano VX - LarmorPhysx Vers. 1.1beta Feb. 2014\nAuthor: Pier Paolo Ciarravano\nhttp://www.larmor.com" << std::endl;

	std::cout << "Start at: " << currentDateTime() << std::endl;
	CGAL::Timer timer;
	timer.start();

	ConfigManager::init(argc, argv);


	if (ConfigManager::action == "generaterib")
	{
		std::cout << "Go generaterib:" << std::endl;
		generateRIB();
	}

	if (ConfigManager::action == "simulation")
	{
		std::cout << "Go simulation:" << std::endl;
		PhysicsCore physicsCore;
	}

	if (ConfigManager::action == "animationviewer")
	{
		std::cout << "run_animation_viewer:" << std::endl;
		LarmorPhysxViewer::run_animation_viewer();
	}

	//Demo scenes:

	if (ConfigManager::action == "createscene1")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene1:" << std::endl;
		createFirstFrame_Scene1();
	}
	if (ConfigManager::action == "createscene2")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene2:" << std::endl;
		createFirstFrame_Scene2();
	}
	if (ConfigManager::action == "createscene3")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene3:" << std::endl;
		createFirstFrame_Scene3();
	}
	if (ConfigManager::action == "createscene4")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene4:" << std::endl;
		createFirstFrame_Scene4();
	}
	if (ConfigManager::action == "createscene5")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene5:" << std::endl;
		createFirstFrame_Scene5();
	}
	if (ConfigManager::action == "createscene6")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene6:" << std::endl;
		createFirstFrame_Scene6();
	}
	if (ConfigManager::action == "createscene7")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene7:" << std::endl;
		createFirstFrame_Scene7();
	}
	if (ConfigManager::action == "createscene8")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene8:" << std::endl;
		createFirstFrame_Scene8();
	}
	if (ConfigManager::action == "createscene9")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene9:" << std::endl;
		createFirstFrame_Scene9();
	}
	if (ConfigManager::action == "createscene10")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene10:" << std::endl;
		createFirstFrame_Scene10();
	}
	if (ConfigManager::action == "createscene11")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene11:" << std::endl;
		createFirstFrame_Scene11();
	}
	if (ConfigManager::action == "createscene12")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene12:" << std::endl;
		createFirstFrame_Scene12();
	}
	if (ConfigManager::action == "createscene13")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene13:" << std::endl;
		createFirstFrame_Scene13();
	}
	if (ConfigManager::action == "createscene14")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene14:" << std::endl;
		createFirstFrame_Scene14();
	}
	if (ConfigManager::action == "createscene15")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene15:" << std::endl;
		createFirstFrame_Scene15();
	}
	if (ConfigManager::action == "createscene16")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene16:" << std::endl;
		createFirstFrame_Scene16();
	}
	if (ConfigManager::action == "createscene17")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene17:" << std::endl;
		createFirstFrame_Scene17();
	}
	if (ConfigManager::action == "createscene18")
	{
		createNotExistsDir(ConfigManager::scene_output_directory.c_str());

		std::cout << "createFirstFrame_Scene18:" << std::endl;
		createFirstFrame_Scene18();
	}
	if (ConfigManager::action == "explodescene")
	{
		std::cout << "generateExplodingScene():" << std::endl;
		generateExplodingScene();
	}
	if (ConfigManager::action == "rotatescene")
	{
		std::cout << "generateRotatingScene():" << std::endl;
		generateRotatingScene();
	}
	if (ConfigManager::action == "rotexpscene")
	{
		std::cout << "generateRotatingExplodingScene():" << std::endl;
		generateRotatingExplodingScene();
	}


	// old action
	if (ConfigManager::action == "mesh")
	{
		std::list<Triangle> readedTriangles = readObjMeshToTriangles(ConfigManager::input.c_str());

		VectListCSegment3d vettorelisteseg3d;
		VectListCTriangle3d listMeshes;
		VectCPoint3d pointsTranslation;
		TrianglesInfoList cutInfo = createNewTriangleInfoList(readedTriangles);
		ListCTriangle3d cutMeshConverted = fromTriangles(readedTriangles, cutInfo);
		listMeshes.push_back(cutMeshConverted);
		vettorelisteseg3d.push_back(ListCSegment3d());
		CPoint3d cpoint3d;
		cpoint3d.x = CGAL::to_double(0);
		cpoint3d.y = CGAL::to_double(0);
		cpoint3d.z = CGAL::to_double(0);
		pointsTranslation.push_back(cpoint3d);
		MeshViewerAnim::openMeshViewerAnim(listMeshes, pointsTranslation);
	}


	// old action
	if (ConfigManager::action == "view")
	{

		VectListCSegment3d vettorelisteseg3d;
		VectListCTriangle3d listMeshes;
		VectCPoint3d pointsTranslation;
		double totalVolume = 0.0;

		for (int i = 1; i <= ConfigManager::break_pieces_test; ++i)
		{
			std::stringstream fileName;
			fileName << ConfigManager::scene_output_directory << FILE_SEPARATOR_CHAR << "static_obj_" << i << ".bin";
			StaticObject so = LarmorPhysx::loadStaticObject( fileName.str().c_str() );
			totalVolume += so.volume;

			TranslatedMeshData meshDataPartTranslated = so.meshData;
			MeshData meshDataPart = meshDataPartTranslated.first;
			Point tp = meshDataPartTranslated.second;
			std::cout << "meshDataPart size: " << meshDataPart.first.size() << std::endl;

			ListCTriangle3d cutMeshConverted = fromTriangles(meshDataPart.first, meshDataPart.second);
			//ListCTriangle3d cutMeshConverted = fromTriangles(so.simplifyMesh, createNewTriangleInfoList(so.simplifyMesh));
			listMeshes.push_back(cutMeshConverted);
			vettorelisteseg3d.push_back(ListCSegment3d());

			CPoint3d cpoint3d;
			cpoint3d.x = CGAL::to_double(tp.x());
			cpoint3d.y = CGAL::to_double(tp.y());
			cpoint3d.z = CGAL::to_double(tp.z());
			pointsTranslation.push_back(cpoint3d);

		}

		std::cout << "totalVolume: " << totalVolume << std::endl;

		//openMeshViewer(listMeshes, vettorelisteseg3d);
		MeshViewerAnim::openMeshViewerAnim(listMeshes, pointsTranslation);
	}



	std::cout << "End at: " << currentDateTime() << std::endl;
	timer.stop();
	std::cout << "Total time: " << timer.time() << " seconds." << std::endl;

	return 0;
}

