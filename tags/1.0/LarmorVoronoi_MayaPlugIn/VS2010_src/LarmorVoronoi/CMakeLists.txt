##*****************************************************************************
## Larmor-Physx Version 1.0 2013
## Copyright (c) 2013 Pier Paolo Ciarravano - http://www.larmor.com
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
## Version 1.1.1Beta Build 78 (08/03/14)
##
## ****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

Project(LarmorVoronoi)


#version variables
SET(MAYA_VERSION "2012")
#SET(MAYA_VERSION "2013")
#SET(MAYA_VERSION "2014")
SET(LIB_DEPENDENCIES_DIR "/Users/user/thirdpartylibs")
#SET(LIB_DEPENDENCIES_DIR "/home/user/thirdpartylibs")


#SET(CMAKE_CXX_WARNING_LEVEL 4)
SET(CMAKE_VERBOSE_MAKEFILE TRUE)
SET(CMAKE_CXX_COMPILER /usr/bin/g++)
SET(CMAKE_BUILD_TYPE Release)


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


IF ( "${CMAKE_BUILD_TYPE}" MATCHES "Debug" )
  ADD_DEFINITIONS( -DDEBUG=1 -UNDEBUG )
  IF ( NOT WINDOWS )
    ADD_DEFINITIONS( -Wall -Werror -Wextra -Wno-unused-parameter)
  ENDIF()
ELSEIF ( "${CMAKE_BUILD_TYPE}" MATCHES "Release" )
  ADD_DEFINITIONS( -DNDEBUG=1 -UDEBUG )
  IF ( NOT WINDOWS )
	# use -s
    ADD_DEFINITIONS( -O3 -s )
  ENDIF()
ENDIF()


IF( ${WINDOWS} )
  ADD_DEFINITIONS ( -DOPENEXR_DLL -DHALF_EXPORTS )
  ADD_DEFINITIONS( -DPLATFORM_WINDOWS -DPLATFORM=WINDOWS )
  FIND_PATH(MAYA_BASE_DIR devkit/include/maya/MFn.h PATH "C:/Program Files/Autodesk/Maya${MAYA_VERSION}" )
ELSEIF( ${DARWIN} )
  #ADD_DEFINITIONS( -DPLATFORM_DARWIN -DPLATFORM=DARWIN "-arch x86_64" )
  ADD_DEFINITIONS( -DPLATFORM_DARWIN -DPLATFORM=DARWIN )
  FIND_PATH(MAYA_BASE_DIR devkit/include/maya/MFn.h PATH "/Applications/Autodesk/maya${MAYA_VERSION}" )
ELSE()
  ADD_DEFINITIONS( -DPLATFORM_LINUX -DPLATFORM=LINUX )
  FIND_PATH(MAYA_BASE_DIR include/maya/MFn.h PATH "/usr/autodesk/maya${MAYA_VERSION}-x64" )
ENDIF()

MESSAGE(STATUS "PROJECT_SOURCE_DIR is ${PROJECT_SOURCE_DIR}" )
MESSAGE(STATUS "MAYA_BASE_DIR is ${MAYA_BASE_DIR}" )
MESSAGE(STATUS "MAYA_VERSION is ${MAYA_VERSION}" )
	
IF (MAYA_BASE_DIR)

	message ("Maya found at location " ${MAYA_BASE_DIR})

	# Maya include and lib
	IF( ${WINDOWS} )
		SET(MAYA_INC_DIR ${MAYA_BASE_DIR}/include)
		SET(MAYA_LIB_DIR ${MAYA_BASE_DIR}/lib)
	ELSEIF( ${DARWIN} )
		SET(MAYA_INC_DIR ${MAYA_BASE_DIR}/devkit/include)
		SET(MAYA_LIB_DIR ${MAYA_BASE_DIR}/Maya.app/Contents/MacOS)
	ELSE()
		SET(MAYA_INC_DIR ${MAYA_BASE_DIR}/include)
		SET(MAYA_LIB_DIR ${MAYA_BASE_DIR}/lib)
	ENDIF()
	SET(MAYA_LIBRARIES Foundation OpenMaya)

	message ("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
	message ("PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})

	
	IF ( NOT WINDOWS )
	  IF ( NOT DARWIN )
		# LINUX SECTION
		SET ( MAYA_EXTENSION ".so" )
		SET( MAYA_COMPILE_FLAGS
		  "-m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords" )

		#/export:initializePlugin /export:uninitializePlugin
		SET( MAYA_LINK_FLAGS
		  "-shared -m64 -g -pthread -pipe -D_BOOL -DLINUX -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords -Wl,-Bsymbolic" )
	  ELSE()
		# DARWIN SECTION
		#SET( MAYA_COMPILE_FLAGS
		#  "-DAW_NEW_IOSTREAMS -DCC_GNU_ -DOSMac_ -DOSMacOSX_ -DBits32_ \
		#  -DOSMac_MachO_ -DREQUIRE_IOSTREAM -fno-gnu-keywords -fpascal-strings \
		#-arch i386 -D_LANGUAGE_C_PLUS_PLUS -include ${MAYA_INCLUDE_PATH}/maya/OpenMayaMac.h" ) \
		SET ( MAYA_EXTENSION ".bundle" )
		SET( MAYA_COMPILE_FLAGS
		  "-DAW_NEW_IOSTREAMS -DCC_GNU_ -DOSMac_ -DOSMacOSX_ -DOSMac_MachO_ -DREQUIRE_IOSTREAM -fno-gnu-keywords -fpascal-strings -ftemplate-depth=1024 -D_LANGUAGE_C_PLUS_PLUS -include /Applications/Autodesk/maya${MAYA_VERSION}/Maya.app/Contents/../../devkit/include/maya/OpenMayaMac.h" )

		SET( MAYA_LINK_FLAGS
		  "-fno-gnu-keywords -fpascal-strings -framework System  -framework SystemConfiguration -framework CoreServices -framework Carbon -framework Cocoa -framework ApplicationServices -framework IOKit -bundle -fPIC -Wl,-exported_symbol,__Z16initializePlugin7MObject -Wl,-exported_symbol,__Z18uninitializePlugin7MObject " )
	  ENDIF()
	ELSE()
		# WINDOWS SECTION
	  SET( MAYA_EXTENSION ".mll" )
	  SET( MAYA_COMPILE_FLAGS "/MD /D \"NT_PLUGIN\" /D \"REQUIRE_IOSTREAM\" /D \"_BOOL\"" )
	  SET( MAYA_LINK_FLAGS " /export:initializePlugin /export:uninitializePlugin " )
	ENDIF()

	# Linker flags
	# -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk 
	# -arch x86_64 
	# -L"/Applications/Autodesk/maya2014/Maya.app/Contents/MacOS" 
	# -Wl,-executable_path,"/Applications/Autodesk/maya2014/Maya.app/Contents/MacOS"   
 	
	
	# Include directories
	INCLUDE_DIRECTORIES(${MAYA_INC_DIR})
	INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
	INCLUDE_DIRECTORIES(${LIB_DEPENDENCIES_DIR}/CGAL-4.2/include)
	INCLUDE_DIRECTORIES(${LIB_DEPENDENCIES_DIR}/CGAL-bin/include)
	INCLUDE_DIRECTORIES(${LIB_DEPENDENCIES_DIR}/boost_1_53_0)
	INCLUDE_DIRECTORIES(${LIB_DEPENDENCIES_DIR}/mpfr-3.1.2/src)
	INCLUDE_DIRECTORIES(${LIB_DEPENDENCIES_DIR}/gmp)


	# Linker libraries directories
	LINK_DIRECTORIES(${MAYA_LIB_DIR})
	#LINK_DIRECTORIES(/Users/user/thirdpartylibs/gmp/.libs)
	#LINK_DIRECTORIES(/Users/user/thirdpartylibs/mpfr-3.1.2/src/.libs)
	#LINK_DIRECTORIES(/Users/user/thirdpartylibs/boost_1_53_0/stage/lib)
	#LINK_DIRECTORIES(/Users/user/thirdpartylibs/CGAL-bin/lib)


	#ADD_DEFINITIONS(-DCC_GNU_)
	#ADD_DEFINITIONS(-DOSMac_)
	#ADD_DEFINITIONS(-DOSMacOSX_)
	#ADD_DEFINITIONS(-DREQUIRE_IOSTREAM)
	#ADD_DEFINITIONS(-DOSMac_MachO_)
	#ADD_DEFINITIONS(-O3)
	##ADD_DEFINITIONS(-arch x86_64)
	#ADD_DEFINITIONS(-D_LANGUAGE_C_PLUS_PLUS)
	#ADD_DEFINITIONS(-include "/Applications/Autodesk/maya2014/Maya.app/Contents/../../devkit/include/maya/OpenMayaMac.h")
	#ADD_DEFINITIONS(-fno-gnu-keywords)
	#ADD_DEFINITIONS(-fpascal-strings)
	##ADD_DEFINITIONS(-D_BOOL)
	
	
	# Source header files
	SET(H_FILES 
		LarmorVoronoi.h
		shatter/Custom.h
		shatter/CutterMesher.h
		shatter/DelaunayVoronoi.h
		shatter/SegmentsArrangement.h
		shatter/util.h
		util/check_version.h
	)
	
	# Source cpp files
	SET(CXX_FILES 
		LarmorVoronoiCmd.cpp
		shatter/CutterMesher.cpp
		shatter/DelaunayVoronoi.cpp
		shatter/DisjointMesh.cpp
		shatter/MeshCheck.cpp
		shatter/MeshMassProperties.cpp
		shatter/MeshSimplification.cpp
		shatter/RejointMeshes.cpp
		shatter/SegmentsArrangement.cpp
		shatter/util.cpp
		shatter/VolumeMesh.cpp
		util/check_version.cpp
	)

	SET( SOURCE_FILES ${CXX_FILES} ${H_FILES} )
	
	
	#ADD_LIBRARY(LarmorVoronoi SHARED ${SOURCE_FILES})
	ADD_LIBRARY(LarmorVoronoi MODULE ${SOURCE_FILES})
	SET_TARGET_PROPERTIES( LarmorVoronoi
		PROPERTIES
		COMPILE_FLAGS ${MAYA_COMPILE_FLAGS}
		LINK_FLAGS ${MAYA_LINK_FLAGS}
		PREFIX ""
		SUFFIX ${MAYA_EXTENSION} )

		
	#add external static libraries
	ADD_LIBRARY(CGAL_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(CGAL_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/CGAL-bin/lib/libCGAL.a)

	ADD_LIBRARY(CGAL_Core_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(CGAL_Core_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/CGAL-bin/lib/libCGAL_Core.a)

	ADD_LIBRARY(gmp_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(gmp_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/gmp/.libs/libgmp.a)

	ADD_LIBRARY(mpfr_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(mpfr_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/mpfr-3.1.2/src/.libs/libmpfr.a)

	ADD_LIBRARY(boost_system_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(boost_system_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/boost_1_53_0/stage/lib/libboost_system.a)

	ADD_LIBRARY(boost_thread_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(boost_thread_st PROPERTIES	
		IMPORTED_LOCATION /Users/user/thirdpartylibs/boost_1_53_0/stage/lib/libboost_thread.a)	

	ADD_LIBRARY(boost_serialization_st STATIC IMPORTED)
	SET_TARGET_PROPERTIES(boost_serialization_st PROPERTIES
		IMPORTED_LOCATION /Users/user/thirdpartylibs/boost_1_53_0/stage/lib/libboost_serialization.a)


	# Linker Libraries
	TARGET_LINK_LIBRARIES(LarmorVoronoi ${MAYA_LIBRARIES} CGAL_st CGAL_Core_st boost_thread_st boost_system_st boost_serialization_st mpfr_st gmp_st)


ELSE()
	MESSAGE("Maya not found!!")
ENDIF()


