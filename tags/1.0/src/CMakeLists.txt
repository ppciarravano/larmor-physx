##*****************************************************************************
## Larmor-Physx Version 1.0 2014
## Copyright (c) 2013-2014 Pier Paolo Ciarravano - http://www.larmor.com
## All rights reserved.
##
## This file is part of LarmorVoronoi Maya Plugin (http://code.google.com/p/larmor-physx/).
##
## LarmorVoronoi Maya Plugin is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## LarmorVoronoi Maya Plugin is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with LarmorVoronoi Maya Plugin. If not, see <http://www.gnu.org/licenses/>.
##
## Licensees holding a valid commercial license may use this file in
## accordance with the commercial license agreement provided with the
## software.
##
## Author: Pier Paolo Ciarravano
## $Id$
## Version 27/08/14
##
## ****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

Project(RevanoVX)

#SET(CMAKE_CXX_WARNING_LEVEL 4)
SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#SET(CMAKE_CXX_COMPILER /usr/bin/g++)
SET(CMAKE_BUILD_TYPE Release)
#SET(PROJECT_SOURCE_DIR /home/larmor/DEVELOP/LarmorPhysx/workspace/larmor-physx/src)
#SET(PROJECT_BINARY_DIR /home/larmor/DEVELOP/LarmorPhysx/workspace/build)

#ADD_DEFINITIONS( -DFILE_SEPARATOR_CHAR="/" )
ADD_DEFINITIONS( -DBT_USE_DOUBLE_PRECISION=1 )
ADD_DEFINITIONS( -DBULLET_USE_CGAL_PATCH=1 )

SET( WINDOWS FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
	SET( WINDOWS TRUE )
ENDIF()

SET( DARWIN FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
	SET( DARWIN TRUE )
ENDIF()

SET( LINUX FALSE )
IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux" )
	SET( LINUX TRUE )
ENDIF()

IF( ${WINDOWS} )
	ADD_DEFINITIONS( -DPLATFORM_WINDOWS -DPLATFORM=WINDOWS )
ELSEIF( ${DARWIN} )
	ADD_DEFINITIONS( -DPLATFORM_DARWIN -DPLATFORM=DARWIN )
ELSE()
	ADD_DEFINITIONS( -DPLATFORM_LINUX -DPLATFORM=LINUX )
ENDIF()


IF ( NOT WINDOWS )
  IF ( NOT DARWIN )
	# LINUX SECTION
	SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords" )
	SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic" )
  ELSE()
	# DARWIN SECTION
	# NOT TESTED: TODO
	SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -DCC_GNU_ -DOSMac_ -DOSMacOSX_ -DOSMac_MachO_ -DREQUIRE_IOSTREAM -fPIC -fno-gnu-keywords -ftemplate-depth=1024 -D_LANGUAGE_C_PLUS_PLUS" )
	SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -pthread -pipe -framework System  -framework SystemConfiguration -framework CoreServices -framework Carbon -framework Cocoa -framework ApplicationServices -framework IOKit -bundle -fPIC -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic" )
  ENDIF()
ELSE()
	# WINDOWS SECTION
	SET( PRJ_COMPILE_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s -Os -Wall -c -fmessage-length=0 --param ggc-min-heapsize=2008192" )
	SET( PRJ_LINK_FLAGS   "-DNDEBUG=1 -UDEBUG -O3 -s" )
ENDIF()

MESSAGE(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}" )
MESSAGE("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
MESSAGE("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
MESSAGE("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})


IF ( WINDOWS )

	# Include directories
	INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/bullet-2.81/bullet-2.81-rev2613_CGAL41/bullet-2.81-rev2613/src)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/CGAL-4.1/include)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/CGAL-4.1/BUILD/include)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/boost_1_47_0)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/freeglut/include)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/gmplib/gmp-4.3.2)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/core-2.1_lib/core-2.1.1/builds/mpfr-3.0.0)
	INCLUDE_DIRECTORIES(C:/LARMOR/DEVELOP/tbb/src/tbb41_20121003oss/include)

	# Linker libraries directories
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/bullet-2.81/install_CGAL41)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/bullet-2.81/bullet-2.81-rev2613_CGAL41/bullet-2.81-rev2613/Glut)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/CGAL-4.1/BUILD/lib)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/boost_1_47_0_64/lib)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/boost_1_47_0/lib)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/freeglut/lib)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/gmplib/gmp-4.3.2/.libs)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/core-2.1_lib/core-2.1.1/builds/mpfr-3.0.0/.libs)
	LINK_DIRECTORIES(C:/LARMOR/DEVELOP/tbb/src/tbb41_20121003oss/build/windows_ia32_gcc_mingw4.5.2_release)

	# Linker libraries
	SET(PRJ_LIBRARIES
		BulletDynamics
		BulletCollision
		LinearMath
		CGAL
		boost_thread-mgw45-mt-1_47
		boost_serialization-mgw45-mt-1_47
		boost_program_options-mgw45-mt-1_47
		opengl32
		glu32
		freeglut_static
		freeglut
		mpfr
		gmp
		tbb
	)

ELSE()

	# Include directories
	INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/bullet-2.81-rev2613/src)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/CGAL-4.2/include)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/CGAL_bin/include)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/boost_1_53_0)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/gmp-5.1.3)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/mpfr-3.1.2/src)
	INCLUDE_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/tbb41_20121003oss/include)

	# Linker libraries directories
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/bullet_bin/libs)
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/CGAL_bin/lib)
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/boost_1_53_0/stage/lib)
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/gmp-5.1.3/.libs)
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/mpfr-3.1.2/src/.libs)
	LINK_DIRECTORIES(/home/larmor/DEVELOP/LarmorPhysx/third-party/tbb41_20121003oss/lib/intel64/cc4.1.0_libc2.4_kernel2.6.16.21)

	# Linker libraries
	SET(PRJ_LIBRARIES
		BulletDynamics
		BulletCollision
		LinearMath
		CGAL
		boost_thread
		boost_serialization
		boost_program_options
		boost_system
		glut
		GLU
		GL	
		mpfr
		gmp
		tbb
	)

ENDIF()


# Source header files
SET(H_FILES 
	shatter/Custom.h
	shatter/CutterMesher.h
	shatter/DelaunayVoronoi.h
	shatter/LinesEditor.h
	shatter/LinesViewer.h
	shatter/MeshViewer.h
	shatter/SegmentsArrangement.h
	shatter/SphereMesher.h
	shatter/util.h
	core/animation_viewer.h
	core/ConfigManager.h
	core/core_classes.h
	core/CubicSpline.h
	core/PhysicsCore.h
	core/RibGenerator.h
	core/scenes_creation.h
)

# Source cpp files
SET(CXX_FILES 
	RevanoVX.cpp
	shatter/CutterMesher.cpp
	shatter/DelaunayVoronoi.cpp
	shatter/DisjointMesh.cpp
	shatter/LinesEditor.cpp
	shatter/LinesViewer.cpp
	shatter/MeshCheck.cpp
	shatter/MeshMassProperties.cpp
	shatter/MeshSimplification.cpp
	shatter/MeshViewer.cpp
	shatter/MeshViewerAnim.cpp
	shatter/RejointMeshes.cpp
	shatter/SegmentsArrangement.cpp
	shatter/SphereMesher.cpp
	shatter/util.cpp
	shatter/VolumeMesh.cpp
	core/animation_viewer.cpp
	core/ConfigManager.cpp
	core/core_classes.cpp
	core/CubicSpline.cpp
	core/PhysicsCore.cpp
	core/PhysicsCore_shatter.cpp
	core/RibGenerator.cpp
	core/scenes_creation.cpp
)

SET( SOURCE_FILES ${CXX_FILES} ${H_FILES} )

ADD_EXECUTABLE(RevanoVX ${SOURCE_FILES})

SET_TARGET_PROPERTIES( RevanoVX
	PROPERTIES
	COMPILE_FLAGS ${PRJ_COMPILE_FLAGS}
	LINK_FLAGS ${PRJ_LINK_FLAGS}
	PREFIX "" )
	
TARGET_LINK_LIBRARIES(RevanoVX ${PRJ_LIBRARIES})



