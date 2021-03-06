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

#ifndef CHECK_VERSION_H_
#define CHECK_VERSION_H_

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

//For getMACaddress, used to generate a unique id
#if defined(WIN32)
    #include <Iphlpapi.h>
	#pragma comment(lib, "iphlpapi.lib")
#elif defined(__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
	#include <IOKit/IOKitLib.h>
	#include <IOKit/network/IOEthernetInterface.h>
	#include <IOKit/network/IONetworkInterface.h>
	#include <IOKit/network/IOEthernetController.h>
#elif defined(LINUX) || defined(linux)
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <arpa/inet.h>
#endif

#define SERVER_CHECK_VERSION ""
#define PATH_CHECK_VERSION ""

using boost::asio::ip::tcp;

void LarmorCheckProductVersion(const char* productName, const char* productVersionBuild, std::string productInfo);

int LarmorCheckProductVersion_getCode();

std::string LarmorCheckProductVersion_getMessage();

#endif /* CHECK_VERSION_H_ */
