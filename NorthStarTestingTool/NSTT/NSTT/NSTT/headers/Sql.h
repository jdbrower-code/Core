#pragma once
#define SECURITY_WIN32          // needed before including Sspi.h in some setups
//Sql Connection headers
#include <nanodbc.h>
#include "example_unicode_utils.h"

#include <windows.h>
#include <lmcons.h>
#include <security.h>
#include <Sspi.h>  

#include <iostream>
#include <stdexcept>
#include <string>
#include <stdio.h>

#pragma comment(lib, "Secur32.lib")

class NorthStarSql
{
private:

public:
	//Connections
	nanodbc::connection SqlConnect();

	nanodbc::connection NSTT_SqlConnect();

	nanodbc::connection NSTT_NsLive_SqlConnect();

		//Queries
	nanodbc::result AcctAndOccupant(nanodbc::connection conn);

	std::string GetShortLogin(nanodbc::connection conn);

	nanodbc::result UserSecurityQuery(nanodbc::connection conn);

	std::string GetUserName();

};

	////Connections
	//nanodbc::connection SqlConnect(
	//	std::string Server,
	//	std::string User, 
	//	std::string Password);
	//
	//nanodbc::connection NSTT_SqlConnect(
	//	/*std::string Server,
	//	std::string User,
	//	std::string Password*/);
	//
	//nanodbc::connection NSTT_NsLive_SqlConnect(
	//	/*std::string Server,
	//	std::string User,
	//	std::string Password*/);
	//
	////Queries
	//nanodbc::result AcctAndOccupant(nanodbc::connection conn);
	//
	//std::string GetShortLogin(nanodbc::connection conn);
	//
	//nanodbc::result UserSecurityQuery(nanodbc::connection conn);
	//
	//std::string GetUserName();









