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

#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <vector>
#include <list>
#include <exception>
#include <boost/program_options.hpp>

#define LARMORPHYSX_DEFAULT_CONFIG_FILE_NAME "larmorphysx_config.ini"
namespace po = boost::program_options;

namespace LarmorPhysx
{

	class ConfigManager {

		public:

			static bool initialize;

			//command line options
			static std::string action;
			static std::string config;
			static std::string input;

			//config file options
			static std::string name_test;
			static int break_pieces_test;
			static std::string action_command;
			static double facet_distance_coef;
			static std::string scene_output_directory;
			static bool not_use_cgal_collision;
			static bool use_simplified_meshes_for_bullet;
			static bool disable_deactivation;
			static bool voronoi_shatter;
			static bool use_memory_for_meshes;
			static bool rib_motion_blur;
			static int rib_frames_per_second;
			static int rib_frames_per_motion_blur;
			static int steps_per_second;
			static int internal_sub_steps;
			static int total_anim_steps;
			static double gravity_force;
			static bool is_ground_present;
			static int start_load_frame;

			static void init(int argc, char** argv);

	};

}


#endif /* CONFIG_MANAGER_H_ */
