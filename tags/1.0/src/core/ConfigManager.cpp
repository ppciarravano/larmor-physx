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

#include "ConfigManager.h"


namespace LarmorPhysx
{

	bool ConfigManager::initialize = false;

	std::string ConfigManager::action = "";
	std::string ConfigManager::config = LARMORPHYSX_DEFAULT_CONFIG_FILE_NAME;
	std::string ConfigManager::input = "";

	std::string ConfigManager::name_test = "";
	int ConfigManager::break_pieces_test = 10;
	std::string ConfigManager::action_command = "";
	double ConfigManager::facet_distance_coef = 10.0;
	std::string ConfigManager::scene_output_directory = "";
	bool ConfigManager::not_use_cgal_collision = false;
	bool ConfigManager::use_simplified_meshes_for_bullet = false;
	bool ConfigManager::disable_deactivation = false;
	bool ConfigManager::voronoi_shatter = false;
	bool ConfigManager::use_memory_for_meshes = false;
	bool ConfigManager::rib_motion_blur = false;
	int ConfigManager::rib_frames_per_second = 30;
	int ConfigManager::rib_frames_per_motion_blur = 10;
	int ConfigManager::frames_per_step_animationviewer = 1;
	int ConfigManager::steps_per_second = 60;
	int ConfigManager::internal_sub_steps = 4;
	int ConfigManager::total_anim_steps = 120;
	double ConfigManager::gravity_force = -10.0;
	bool ConfigManager::is_ground_present = true;
	int ConfigManager::start_load_frame = 0;

	void ConfigManager::init(int argc, char** argv)
	{
		if(!initialize)
		{
			try
			{
				std::cout << "Reading the config items from command line..." << std::endl;

				po::options_description descCL("LarmorPhysx command line options");
				descCL.add_options()
					("help", "produce help message")
					("action", po::value<std::string>( &action ), "main action")
					("config", po::value<std::string>( &config )->implicit_value(LARMORPHYSX_DEFAULT_CONFIG_FILE_NAME), "input config file");
					("input", po::value<std::string>( &input )->implicit_value(""), "input file");
					//("not_use_cgal_collision", po::value<bool>( &not_use_cgal_collision )->implicit_value(1), "not use CGAL for triangle collision algorithm");

				po::variables_map vmCL;
				po::store(po::parse_command_line(argc, argv, descCL), vmCL);
				po::notify(vmCL);

				if (vmCL.count("help")) {
					std::cout << descCL << "\n";
					exit(0);
				}

				if (!vmCL.count("action")) {
					std::cout << "Action was not set: run with --help\n";
					exit(0);
				}

				std::cout << "COMMAND LINE PARAM: action = " << action << std::endl;
				std::cout << "COMMAND LINE PARAM: config = " << config << std::endl;
				std::cout << "COMMAND LINE PARAM: input = " << input << std::endl;


				std::cout << "Reading the config items from config file..." << std::endl;

				//Write on file
				//std::ofstream settings_file( LARMORPHYSX_DEFAULT_CONFIG_FILE_NAME );
				//settings_file << "name = " << value << "\n";
				//settings_file.close();

				po::options_description desc("LarmorPhysx Configs");
				desc.add_options()
					("name_test", po::value< std::string >( &name_test ), "name_test" );
				desc.add_options()
					("break_pieces_test", po::value<int>( &break_pieces_test ), "break_pieces_test" );
				desc.add_options()
					("facet_distance_coef", po::value<double>( &facet_distance_coef ), "facet_distance_coef" );
				desc.add_options()
					("scene_output_directory", po::value<std::string>( &scene_output_directory ), "scene_output_directory" );
				desc.add_options()
					("action_command", po::value<std::string>( &action_command ), "action_command" );
				desc.add_options()
					("not_use_cgal_collision", po::value<bool>( &not_use_cgal_collision ), "not use CGAL for triangle collision algorithm");
				desc.add_options()
					("use_simplified_meshes_for_bullet", po::value<bool>( &use_simplified_meshes_for_bullet ), "use_simplified_meshes_for_bullet");
				desc.add_options()
					("disable_deactivation", po::value<bool>( &disable_deactivation ), "disable_deactivation");
				desc.add_options()
					("voronoi_shatter", po::value<bool>( &voronoi_shatter ), "voronoi_shatter");
				desc.add_options()
					("use_memory_for_meshes", po::value<bool>( &use_memory_for_meshes ), "use_memory_for_meshes");
				desc.add_options()
					("rib_motion_blur", po::value<bool>( &rib_motion_blur ), "rib_motion_blur");
				desc.add_options()
					("rib_frames_per_second", po::value<int>( &rib_frames_per_second ), "rib_frames_per_second" );
				desc.add_options()
					("rib_frames_per_motion_blur", po::value<int>( &rib_frames_per_motion_blur ), "rib_frames_per_motion_blur" );
				desc.add_options()
					("frames_per_step_animationviewer", po::value<int>( &frames_per_step_animationviewer ), "frames_per_step_animationviewer" );
				desc.add_options()
					("steps_per_second", po::value<int>( &steps_per_second ), "steps_per_second" );
				desc.add_options()
					("internal_sub_steps", po::value<int>( &internal_sub_steps ), "internal_sub_steps" );
				desc.add_options()
					("total_anim_steps", po::value<int>( &total_anim_steps ), "total_anim_steps" );
				desc.add_options()
					("gravity_force", po::value<double>( &gravity_force ), "gravity_force" );
				desc.add_options()
					("is_ground_present", po::value<bool>( &is_ground_present ), "is_ground_present");
				desc.add_options()
					("start_load_frame", po::value<int>( &start_load_frame ), "start_load_frame" );


				std::ifstream settings_file( config.c_str() );
				po::variables_map vm = po::variables_map();
				po::store( po::parse_config_file( settings_file , desc ), vm );
				settings_file.close();
				po::notify( vm );

				std::cout << "CONFIG PARAM: name_test = " << name_test << std::endl;
				std::cout << "CONFIG PARAM: break_pieces_test = " << break_pieces_test << std::endl;
				std::cout << "CONFIG PARAM: action_command = " << action_command << std::endl;
				std::cout << "CONFIG PARAM: facet_distance_coef = " << facet_distance_coef << std::endl;
				std::cout << "CONFIG PARAM: scene_output_directory = " << scene_output_directory << std::endl;
				std::cout << "CONFIG PARAM: not_use_cgal_collision = " << not_use_cgal_collision << std::endl;
				std::cout << "CONFIG PARAM: use_simplified_meshes_for_bullet = " << use_simplified_meshes_for_bullet << std::endl;
				std::cout << "CONFIG PARAM: disable_deactivation = " << disable_deactivation << std::endl;
				std::cout << "CONFIG PARAM: voronoi_shatter = " << voronoi_shatter << std::endl;
				std::cout << "CONFIG PARAM: use_memory_for_meshes = " << use_memory_for_meshes << std::endl;
				std::cout << "CONFIG PARAM: rib_motion_blur = " << rib_motion_blur << std::endl;
				std::cout << "CONFIG PARAM: rib_frames_per_second = " << rib_frames_per_second << std::endl;
				std::cout << "CONFIG PARAM: rib_frames_per_motion_blur = " << rib_frames_per_motion_blur << std::endl;
				std::cout << "CONFIG PARAM: frames_per_step_animationviewer = " << frames_per_step_animationviewer << std::endl;
				std::cout << "CONFIG PARAM: steps_per_second = " << steps_per_second << std::endl;
				std::cout << "CONFIG PARAM: internal_sub_steps = " << internal_sub_steps << std::endl;
				std::cout << "CONFIG PARAM: total_anim_steps = " << total_anim_steps << std::endl;
				std::cout << "CONFIG PARAM: gravity_force = " << gravity_force << std::endl;
				std::cout << "CONFIG PARAM: is_ground_present = " << is_ground_present << std::endl;
				std::cout << "CONFIG PARAM: start_load_frame = " << start_load_frame << std::endl;
				std::cout << std::endl;

				initialize = true;

			}
			catch (std::exception& e)
			{
				std::cout << "Exception init command line and config parameters: run with --help" << std::endl;
				std::cout << "Exception message: " << e.what() << std::endl;
				exit(0);
			}
		}
	}


}
