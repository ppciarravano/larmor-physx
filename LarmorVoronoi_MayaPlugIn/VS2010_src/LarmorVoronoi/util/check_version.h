/*
 * Project name: Larmor-Physx
 * Released: 28 July 2013
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

#ifndef CHECK_VERSION_H_
#define CHECK_VERSION_H_

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

//For getMACaddress, used to generate a unique id 
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

#define SERVER_CHECK_VERSION ""
#define PATH_CHECK_VERSION ""

using boost::asio::ip::tcp;

void LarmorCheckProductVersion(const char* productName, const char* productVersionBuild, std::string productInfo);

int LarmorCheckProductVersion_getCode();

std::string LarmorCheckProductVersion_getMessage();

#endif /* CHECK_VERSION_H_ */
