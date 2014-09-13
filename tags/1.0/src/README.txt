/*****************************************************************************
 * Larmor-Physx Version 1.0 2014
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

Build instructions for LarmorPhysx RevanoVX (standalone) Revision: r30 13 Sept. 2014.


Build third-party libraries:

1) Build Boost Libraries vers. boost_1_53_0
	cd boost_1_53_0
	./bootstrap.sh
	./b2 cxxflags=-fPIC

2) Build Gmp library vers. gmp-5.1.3
	cd gmp-5.1.3
	./configure --enable-cxx  --with-pic
	make

3) Build Mpfr library vers. mpfr-3.1.2
	cd mpfr-3.1.2
	./configure --with-gmp-build=/path_of_third-party_libs/gmp-5.1.3/ --with-pic
	make

4) Build CGAL library vers. CGAL-4.2
	Configure using CMake/CMake-gui and setup paths for Gmp, Mpfr and Boost libraries
	use CGAL_CXX_FLAGS:STRING= -frounding-math -fPIC
	use CMAKE_CXX_FLAGS:STRING= -frounding-math -fPIC
	make

5) Build Bullet Physics Vers.2.81-rev2613
	Apply the patches following the instructions in larmor-physx/BulletPhysics_patches/README.txt
	Configure using CMake/CMake-gui with double_precision, multithreading and build-type: Release
	use CMakeCache.txt:CMAKE_CXX_FLAGS:STRING=-fPIC
	make

6) Download Intel Threading Building Blocks TBB Vers. tbb41_20121003oss if you want to use Voronoi multithreading (sperimental)


Build LarmorPhysx RevanoVX (standalone):

	edit larmor-physx/src/CMakeList.txt and update INCLUDE_DIRECTORIES and LINK_DIRECTORIES with the correct paths of your third-party libraries
	edit larmor-physx/src/CMakeList.txt and update PRJ_LIBRARIES with your third-party library names
	edit larmor-physx/src/CMakeList.txt and if you don't want to use the -DBULLET_USE_CGAL_PATCH=1 comment the ADD_DEFINITIONS
	cd larmor-physx/src/
	cmake .
	make

Follow the instructions in larmor-physx/bin/README.txt to run RevanoVX. 


