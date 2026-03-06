#include "Core.h"

#include "imgui_stdlib.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>

SecGroupFlags dialogHelp;
NorthStarSql SecSql;



std::string user = SecSql.GetUserName();

//std::string TestingGroup = dialogHelp.returnSecGroup();
std::string NSTT_ICON =
"   #########        ########    ######     ############  ###############	############### \n"
"  ###              ###   ###     ###      ###                 ####               ####      \n"
" ##               ###    ###    ###      ###                 ####               ####       \n"
"###              ###     ###   ###      #############       ####               ####        \n"
" ##             ###      ###  ###                 ###      ####               ####         \n"
" ###           ###       #######                 ###      ####               ####          \n"
"  #########  #####       ######       #############      ####               ####           \n"
"																							\n"
"		    COSM		NORTHSTAR			TESTING			TOOL							\n";

std::string LaunchDialog =
"Hello " + user.substr(user.find('_') + 1) + ", \nWelcome to the NorthStar Testing Tool.\n\n"
"This tool will walk you through the various \ntests that you have been tasked with.\n\n"
"My Records show that you are employed \nas a " + dialogHelp.returnSecGroup() + ", is this correct? \n\n"
"If so please click Yes below; \nif not please click No.";

std::string IncorrectPostionAssignment =
"It appears you have been set up \nwrong in NorthStar\n\n"
"Please reach out to the System Administrator \nso they can get this corrected\n\n"
"Then relaunch this tool \nand begin your testing of the software\n";

static std::string last_error() {
	char* msg = nullptr;
	DWORD err = GetLastError();
	if (!err) return "0";
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, 0, (LPSTR)&msg, 0, nullptr);
	std::string s = msg ? msg : "";
	if (msg) LocalFree(msg);
	return s;
}

static std::filesystem::path exe_dir() {
	char buf[MAX_PATH];
	GetModuleFileNameA(nullptr, buf, MAX_PATH);
	std::filesystem::path p(buf);
	return p.parent_path();
}

static std::filesystem::path safe_appdata_dir() {
	char* appdata = std::getenv("LOCALAPPDATA");
	if (!appdata) appdata = (char*)"C:\\Temp"; // fallback
	return std::filesystem::path(appdata) / "YourApp";
}

void startup_self_test() {
	using std::cout;
	using namespace std::filesystem;

	cout << "Current working dir: " << current_path().string() << "\n";
	cout << "EXE dir: " << exe_dir().string() << "\n";

	// Try to write a probe file to your intended save location
	path targetDir = safe_appdata_dir();
	create_directories(targetDir);

	path probe = targetDir / "write_probe.txt";
	std::ofstream f(probe, std::ios::binary);
	if (!f) {
		cout << "WRITE PROBE FAILED at " << probe.string()
			<< "\nerrno=" << errno << " WinErr=" << last_error() << "\n";
	}
	else {
		f << "ok";
		f.close();
		cout << "WRITE PROBE OK: " << probe.string() << "\n";
	}
}

std::string Temp;

int main()
{
	HWND hw = GetConsoleWindow();
	if (hw) ShowWindow(hw, SW_HIDE);   // hide it (SW_SHOW to re-show)

	//startup_self_test();

	std::string SaveFileName = FileName();

	if (!glfwInit())
		return 1;

	//Set  OpenGL Version and shadder version
	const char* glsl_version = "#version 330";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	//Create OpenGl Window

	GLFWwindow* window = glfwCreateWindow(980, 720, "NorthStar Testing Tool", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync



	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImFont* MyFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\courbd.ttf", 13.0f);
	io.FontDefault = MyFont;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);

#ifdef __EMSCRIPTEN__
	ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
	ImGui_ImplOpenGL3_Init(glsl_version);

	//Declare window color
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Class Calls
	launchPad LP;
	SecGroupFlags SGF;
	cashierTest CT;
	AcctSpecNBTest  ACNB;
	AcctSpecTest ACT;

	// Somewhere persistent (e.g., file-scope or static in your render function)
	static double approach_start = ImGui::GetTime();

	// Each frame in your render loop:
	//ImGuiIO& io = ImGui::GetIO();
	const ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
	const ImVec2 CenterTop(io.DisplaySize.x, 140.0f);


#ifdef __EMSCRIPTEN__
	// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
	// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
	io.IniFilename = nullptr;
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!glfwWindowShouldClose(window))
#endif
	{
		glfwPollEvents();


		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		bool finished = LP.DrawApproachingText(
			NSTT_ICON.c_str(),   // text
			ImVec2(center.x + 500, center.y + 300),               // center of screen
			15.0f,                // base text size (px)
			3200.0f,              // z_start (far)
			0.2f,                 // z_end (near)
			350.0f,               // fov "strength"
			approach_start,       // when animation began
			4.0,                  // duration (seconds)
			IM_COL32(24, 98, 217, 255), // color
			0.0f                  // shadow px
			);
			if (finished)
			{


				if (showInitialDialog)
				{
					ImGui::SetNextWindowPos(ImVec2(250.0f, 240.0f));
					ImGui::Begin("##overlay_InitialText", nullptr, InitialButtonflags);

					ImGui::SetWindowFontScale(1.5f);

					ImGui::TextUnformatted(LaunchDialog.c_str());

					ImGui::SetWindowFontScale(1.0f);

					ImGui::End();
				}
				if (!ButtonLock)
				{
					Button1 = true;
					Button2 = true;
				}

			}
		//}





		



		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(150.0f, 640.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));
		
		ImGui::Begin("##overlay_yes", nullptr, InitialButtonflags);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::SetWindowFontScale(2.0f);
		
		if (Button1)
		{
			if (ImGui::Button("YES", ImVec2(200.0f, 60.0f)))
			{
				std::cout << "Yes button clicked" << std::endl;
				
				showInitialDialog = false;
				//clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
				Button1 = false;
				Button2 = false;
				ButtonLock = true;

				switch (SGF.getFlag())
				{
				case 0:
				{
					CT.fillTaskVectors();
					if (CT.test_1_GetAccountNumber() > 0)
						runCashTest = true;
					break;
				}
				case 1:
				{
					
					ACNB.fillTaskVectors();
					if (ACNB.test_1_GetAccountNumber() > 0)
						runAcctSpecNBTest = true;
					break;
				}
				case 2:
				{
					ACT.fillTaskVectors();
					if (ACT.test_1_GetAccountNumber() > 0)
						runAcctSpecTest = true;
					break;
				}
				default:
					accountNumberObtained = false;
				}
				//if (SGF.getFlag() == 0)
				//{
				//	CT.fillTaskVectors();
				//	if (CT.test_1_GetAccountNumber() > 0)
				//		runCashTest = true;
				//	
				//}
				//else if (SGF.getFlag() == 1)
				//{
				//	CT.fillTaskVectors();
				//	ACNB.fillTaskVectors();
				//	if(ACNB.test_1_GetAccountNumber() > 0)
				//		runAcctSpecNBTest = true;
				//}
				//else
				//	accountNumberObtained = false;
					
			}
		}
		if (runCashTest)
		{
			//Need to work out loop for cashier testing here
			// Question Number
			int QuestNum = 0;
			CT.T1_CashTest();
				
		}
		else if (runAcctSpecNBTest)
		{
			int QuestNum = 0;
			ACNB.T1_AcctSpecNBTest();
		}
		else if (runAcctSpecTest)
		{
			int QuestNum = 0;
			ACT.T1_AcctSpecTest();
		}
		else if ((!accountNumberObtained))
		{
			showInitialDialog;
			ImGui::SetNextWindowPos(ImVec2(450.0f, 300.0f));
			ImGui::Begin("##overlay_noacct", nullptr, ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_NoBackground   // transparent
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoBringToFrontOnFocus
				| ImGuiWindowFlags_NoFocusOnAppearing
			);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::SetWindowFontScale(2.0f);

			ImGui::TextUnformatted(
				"No Account Available\n"
				"Please Reach out to Your System Admin");
			ImGui::SetWindowFontScale(1.0f);
			ImGui::PopStyleColor();
			ImGui::End();
		}
		
		ImGui::SetWindowFontScale(1.0f);
		ImGui::PopStyleColor();

		
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(480.0f, 640.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		ImGui::Begin("##overlay_no", nullptr, InitialButtonflags);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::SetWindowFontScale(2.0f);

		if (Button2)
		{
			if (ImGui::Button("No", ImVec2(200.0f, 60.0f)))
			{
				showInitialDialog = false;
				std::cout << "No button clicked" << std::endl;
				Button1 = false;
				Button2 = false;
				ButtonLock = true;
				showInitialNegativeDialog = true;

			}
			


			
		}

		if (showInitialNegativeDialog)
		{
			ImGui::SetNextWindowPos(ImVec2(270.0f, 240.0f));
			ImGui::Begin("##overlay_InCorrectPosAssign", nullptr, InitialButtonflags);

			ImGui::SetWindowFontScale(1.5f);

			ImGui::TextUnformatted(IncorrectPostionAssignment.c_str());

			ImGui::SetWindowFontScale(1.0f);

			ImGui::End();
		}


		ImGui::SetWindowFontScale(1.0f);
		ImGui::PopStyleColor();
		if 
			(CT.CloseProgram   || 
			 ACNB.CloseProgram ||
			 ACT.CloseProgram)
		{
			glfwSetWindowShouldClose(window, true);
		}

		ImGui::End();



		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

	}
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}