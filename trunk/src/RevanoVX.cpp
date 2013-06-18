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
	std::cout << "Revano VX" << std::endl;

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
	if (ConfigManager::action == "explodescene")
	{
		std::cout << "generateExplodingScene():" << std::endl;
		generateExplodingScene();
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

