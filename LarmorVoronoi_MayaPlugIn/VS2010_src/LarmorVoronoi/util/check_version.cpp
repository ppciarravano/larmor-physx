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

#include "check_version.h"

std::string urlencode(const std::string &s)
{
	//RFC 3986 section 2.3 Unreserved Characters
	const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	std::string escaped="";
	for(size_t i=0; i<s.length(); i++)
	{
		if (unreserved.find_first_of(s[i]) != std::string::npos)
		{
			escaped.push_back(s[i]);
		}
		else
		{
			escaped.append("%");
			char buf[3];
			sprintf(buf, "%.2X", s[i]);
			escaped.append(buf);
		}
	}
	return escaped;
}


//Used to generate a unique id
std::string getMACaddress()
{
	std::string macAddrs;
	IP_ADAPTER_INFO adapterInfo[16];
	DWORD dwBufLen = sizeof(adapterInfo);
	DWORD dwStatus = GetAdaptersInfo(adapterInfo, &dwBufLen);
	if (dwStatus == ERROR_SUCCESS)
	{
		PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
		bool isFirst = true;
		do
		{
			//Add comma
			if (!isFirst)
			{
				macAddrs.append(",");				
			}
			isFirst = false;

			BYTE* MACData = pAdapterInfo->Address;
			char macAddrChars[17];
			sprintf(macAddrChars, "%02X-%02X-%02X-%02X-%02X-%02X",
				MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
			macAddrs.append(macAddrChars);
			pAdapterInfo = pAdapterInfo->Next;
		}
		while(pAdapterInfo);
	}
	else
	{
		macAddrs.append("NO_MAC_ADDR");
	}
	return macAddrs;
}


class LarmorCheckProductVersionClient
{
	public:	 
		LarmorCheckProductVersionClient(boost::asio::io_service& io_service,
			const std::string& server, const std::string& path,
			const char* productName, const char* productVersionBuild, std::string productInfo,
			std::string systemInfo, std::string uniqueId)
			: resolver_(io_service), socket_(io_service)
		{
			return_code = -1;
			return_message = std::string("");
		
			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			std::ostream request_stream(&request_);
			request_stream << "GET " << path; 
			request_stream << "?PRODUCT_NAME=" << urlencode(productName);
			request_stream << "&PRODUCT_VERSION_BUILD=" << urlencode(productVersionBuild);
			request_stream << "&PRODUCT_INFO=" << urlencode(productInfo);
			request_stream << "&SYSTEM_INFO=" << urlencode(systemInfo);
			request_stream << "&UNIQUE_ID=" << urlencode(uniqueId);
			request_stream << " HTTP/1.0\r\n";
			request_stream << "Host: " << server << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Connection: close\r\n\r\n";

			// Start an asynchronous resolve to translate the server and service names
			// into a list of endpoints.
			tcp::resolver::query query(server, "http");
			resolver_.async_resolve(query,
			boost::bind(&LarmorCheckProductVersionClient::handle_resolve, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));

		}

	private:
		tcp::resolver resolver_;
		tcp::socket socket_;
		boost::asio::streambuf request_;
		boost::asio::streambuf response_;
	  
		std::stringstream content_stream;
		static std::string return_message;
		static int return_code;
  
		void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
		{
			if (!err)
			{
				// Attempt a connection to each endpoint in the list until we
				// successfully establish a connection.
				boost::asio::async_connect(socket_, endpoint_iterator,
				boost::bind(&LarmorCheckProductVersionClient::handle_connect, this,
				boost::asio::placeholders::error));
			}
			else
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err.message() << "\n";
			}
		}

		void handle_connect(const boost::system::error_code& err)
		{
			if (!err)
			{
				// The connection was successful. Send the request.
				boost::asio::async_write(socket_, request_,
				boost::bind(&LarmorCheckProductVersionClient::handle_write_request, this,
				boost::asio::placeholders::error));
			}
			else
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err.message() << "\n";
			}
		}

		void handle_write_request(const boost::system::error_code& err)
		{
			if (!err)
			{
				// Read the response status line. The response_ streambuf will
				// automatically grow to accommodate the entire line. The growth may be
				// limited by passing a maximum size to the streambuf constructor.
				boost::asio::async_read_until(socket_, response_, "\r\n",
				boost::bind(&LarmorCheckProductVersionClient::handle_read_status_line, this,
				boost::asio::placeholders::error));
			}
			else
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err.message() << "\n";
			}
		}

		void handle_read_status_line(const boost::system::error_code& err)
		{
			if (!err)
			{
				// Check that response is OK.
				std::istream response_stream(&response_);
				std::string http_version;
				response_stream >> http_version;
				unsigned int status_code;
				response_stream >> status_code;
				std::string status_message;
				std::getline(response_stream, status_message);
				if (!response_stream || http_version.substr(0, 5) != "HTTP/")
				{
					//std::cout << "LarmorCheckProductVersion Invalid response\n";
					return;
				}
				if (status_code != 200)
				{
					//std::cout << "LarmorCheckProductVersion Response returned with status code ";
					//std::cout << status_code << "\n";
					return;
				}

				// Read the response headers, which are terminated by a blank line.
				boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
				boost::bind(&LarmorCheckProductVersionClient::handle_read_headers, this,
				boost::asio::placeholders::error));
			}
			else
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err << "\n";
			}
		}

		void handle_read_headers(const boost::system::error_code& err)
		{
			if (!err)
			{
				// Process the response headers.
				std::istream response_stream(&response_);
				std::string header;
				while (std::getline(response_stream, header) && header != "\r")
				{
					//std::cout << header << "\n";
				}
				//std::cout << "\n";

				// Write whatever content we already have to output.
				if (response_.size() > 0)
				{
					//std::cout << &response_;
				}

				// Start reading remaining data until EOF.
				boost::asio::async_read(socket_, response_,
				boost::asio::transfer_at_least(1),
				boost::bind(&LarmorCheckProductVersionClient::handle_read_content, this,
				boost::asio::placeholders::error));
			}
			else
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err << "\n";
			}
		}

		void handle_read_content(const boost::system::error_code& err)
		{
			// Write all of the data that has been read so far.
			//std::cout << &response_;
			//std::cout.flush();
			content_stream << &response_;

			if (!err)
			{
		  		// Continue reading remaining data until EOF.
				boost::asio::async_read(socket_, response_,
				boost::asio::transfer_at_least(1),
				boost::bind(&LarmorCheckProductVersionClient::handle_read_content, this,
				boost::asio::placeholders::error));
			}
			else if (err != boost::asio::error::eof)
			{
				//std::cout << "LarmorCheckProductVersion Error: " << err << "\n";
			}
			else if (err == boost::asio::error::eof)
			{
				//std::cout << "LarmorCheckProductVersion EOF: " << err << "\n";
				content_stream.flush();

				if ((content_stream.str().find_first_of("|") == 1) && (content_stream.str().length() > 2))
				{
					//parse content_stream
					return_message = content_stream.str().substr(2);
					return_code = atoi(content_stream.str().substr(0, 1).c_str());
					//std::cout << return_code << std::endl;
					std::cout << return_message << std::endl;
					std::cout.flush();
				}
			}
		}

	public:
		static int get_return_code()
		{
			return return_code;
		}

		static std::string get_return_message()
		{
			return return_message;
		}

};

std::string LarmorCheckProductVersionClient::return_message("");
int LarmorCheckProductVersionClient::return_code = -1;

void LarmorCheckProductVersion(const char* productName, const char* productVersionBuild, std::string productInfo)
{
	//std::cout << "Sending LarmorCheckProductVersion..." << "\n";
	try
	{
		std::string macAddrs = getMACaddress(); //MAC addresses used as unique id
		boost::asio::io_service io_service;
		LarmorCheckProductVersionClient http_client(io_service, SERVER_CHECK_VERSION, PATH_CHECK_VERSION,
		productName, productVersionBuild, productInfo, "no system info", macAddrs);
		io_service.run();
	}
	catch (std::exception& e)
	{
		//std::cout << "LarmorCheckProductVersion Exception: " << e.what() << "\n";
	}
}

int LarmorCheckProductVersion_getCode()
{
	return LarmorCheckProductVersionClient::get_return_code();
}

std::string LarmorCheckProductVersion_getMessage()
{
	return LarmorCheckProductVersionClient::get_return_message();
}
