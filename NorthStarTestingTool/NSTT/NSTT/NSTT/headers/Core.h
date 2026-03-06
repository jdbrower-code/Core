/*The Core Header Holds all the includes that the main cpp file needs to operate.It also holds all the
  All the Global variables*/
#pragma once
#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#include <stdint.h>         // intptr_t
#include <fstream>
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>       // PRId64/PRIu64, not avail in some MinGW headers.#endif
#endif

#include <string>
#include <windows.h>
#include <Lmcons.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers


#include <random> // For std::mt19937, std::uniform_int_distribution
#include <chrono> // For seeding with time


#include "WordWrap.h"
#include "Save.h"
#include "SecGroupFlags.h"
#include "Sql.h"
#include "CashierTest.h"
#include "AccountSpecTest_NB.h"
#include "AccountSpecTest.h"
#include "Opening.h"





 






//Program State
//Window Booleans, Used to change windows or launch user or testing related functions
bool showGetLogin = true;
bool ServerReq = true;
bool DBReq = false;
bool UNReq = false;
bool PWReq = false;

bool li_SeverNameEntered = false;
bool li_DatabaseEntered = false;
bool li_UserNameEntered = false;
bool li_PasswordEntered = false;

//static bool focus_Server_once = true;
//static bool focus_Database_now = false;
static bool focus_user_now = true;      // only focus the first field once
static bool focus_pass_now = false;      // set true when Enter on user

bool credsEntered = false;

bool showInitialDialog = true;
bool showInitialNegativeDialog = false;
bool showFeedbackWindow = true;
bool showGreetingDialog = false;
bool showTestingInstructions = false;
bool showTestingDeadlineDialog = false;
bool runCashTest = false;
bool runAcctSpecNBTest = false;
bool runAcctSpecTest = false;
bool accountNumberObtained = true;




//button booleans
bool ButtonLock = false;
bool Button1 = false;
bool Button2 = false;

std::string tempUsername;
std::string tempPassword;



//Account number vector used for ramdomization of account selection
//THIS WILL NEED TO BE SPLIT TO MULTIPLE VECTORS TO CONTROL TESTING
std::vector<int> vec_account_no;

//storage buffer
char Input_buf[4096];


//Flags
ImGuiWindowFlags InitialButtonflags = 	ImGuiWindowFlags_NoDecoration
									  | ImGuiWindowFlags_NoBackground   // transparent
									  | ImGuiWindowFlags_NoMove
									  | ImGuiWindowFlags_NoSavedSettings
									  | ImGuiWindowFlags_NoBringToFrontOnFocus
									  | ImGuiWindowFlags_NoFocusOnAppearing;



std::string TimeOfTest()
{
	std::string SaveFileName;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	tm t = *std::localtime(&currentTime);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &t);
	std::string dateString = buffer;

	return dateString;
}




//Members and bools for Cashiers Test


int Q1_Task_Req = 3;	//Number of account Question 1 must be done on
int Q1_Task = 0;		//Current Task