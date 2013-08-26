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
