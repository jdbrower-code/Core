
//#include "Core.h"
//
///*
//	Program steps
//		1. Get User Name of logged in User
//*/
//
//int main()
//{
//	//Save Filename and tester
//	//Creation
//	std::string SaveFileName = FileName();
//	//Tester (Delete at program design completion)
//	std::cout << SaveFileName << std::endl;
//
//	//Initiate GLFW
//	if (!glfwInit())
//		return 1;
//
//	//Set  OpenGL Version and shadder version
//	const char* glsl_version = "#version 330";
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
//
//	
//	//Create OpenGl Window
//	GLFWwindow* window = glfwCreateWindow(1280, 720, "NorthStar Testing Tool", nullptr, nullptr);
//	if (window == nullptr)
//		return 1;
//	glfwMakeContextCurrent(window);
//	glfwSwapInterval(1); // Enable vsync
//
//	// Setup Dear ImGui context
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGuiIO& io = ImGui::GetIO(); //(void)io;
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//	// Setup Dear ImGui style
//	ImGui::StyleColorsDark();
//
//	// Setup Platform/Renderer backends
//	ImGui_ImplGlfw_InitForOpenGL(window, true);
//#ifdef __EMSCRIPTEN__
//	ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
//#endif
//	ImGui_ImplOpenGL3_Init(glsl_version);
//
//	//Declare window color
//	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//
//
//	//Sql Connections, Need one for each query
//	//nanodbc::connection connName = SqlConnect();
//	//
//	//nanodbc::result AcctAndOccupantqry = AcctAndOccupant(connName);
//	//
//	//for (int i = 1; AcctAndOccupantqry.next(); ++i)
//	//{
//	//	int msg = AcctAndOccupantqry.get<int>(0);
//	//	vec_account_no.push_back(msg);
//	//	//std::cout << msg << std::endl;
//	//}
//	//
//	//
//	//// 1. Create a random number generator engine
//	//// Use std::chrono::system_clock::now().time_since_epoch().count()
//	//// to obtain a time-based seed for better randomness.
//	//std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
//	//
//	//// 2. Define a distribution for the random index
//	//// The range is [0, myVector.size() - 1]
//	//std::uniform_int_distribution<int> distribution(0, vec_account_no.size() - 1);
//	//
//	//// 3. Generate a random index
//	//int randomIndex = distribution(generator);
//	//
//	//// 4. Access the element at the random index
//	//int randomElement = vec_account_no[randomIndex];
//	//
//	//
//	//
//	////std::string UN = GetUserName();
//	//
//	////std::cout << UN << std::endl;
//	//
//	////Group flag class call
//	SecGroupFlags secGrpFlg;
//	//
//	////std::cout << std::boolalpha << secGrpFlg.getCashFlag() << std::endl;
//	//
//	cashierTest cashTest;
//
//	//secGrpFlg.SecGroupFlagSet();
//	//std::cout << secGrpFlg.getCashFlag() << std::endl;
//
//	launchPad LP;
//
//	// Somewhere persistent (e.g., file-scope or static in your render function):
//	static double approach_start = ImGui::GetTime();
//
//	// Each frame in your render loop:
//	//ImGuiIO& io = ImGui::GetIO();
//	const ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
//	const ImVec2 CenterTop(io.DisplaySize.x, 140.0f);
//
//	//Main Loop
//#ifdef __EMSCRIPTEN__
//// For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
//// You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
//	io.IniFilename = nullptr;
//	EMSCRIPTEN_MAINLOOP_BEGIN
//#else
//	while (!glfwWindowShouldClose(window))
//#endif
//	{
//		// Poll and handle events (inputs, window resize, etc.)
//		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
//		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
//		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
//		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
//		glfwPollEvents();
//
//		// Start the Dear ImGui frame
//		ImGui_ImplOpenGL3_NewFrame();
//		ImGui_ImplGlfw_NewFrame();
//		ImGui::NewFrame();
//
//		bool finished = LP.DrawApproachingText(
//			"Welcome to the Northstar Testing Tool",   // text
//			ImVec2(center.x + 640, center.y + 140),               // center of screen
//			60.0f,                // base text size (px)
//			1200.0f,              // z_start (far)
//			5.0f,                 // z_end (near)
//			350.0f,               // fov "strength"
//			approach_start,       // when animation began
//			10.0,                  // duration (seconds)
//			IM_COL32(24, 98, 217, 255), // color
//			0.0f                  // shadow px
//		);
//
//		// Get size of inner child region
//		ImVec2 childSize = ImGui::GetWindowSize();
//		float buttonWidth = 80.0f;
//		float buttonHeight = ImGui::GetFrameHeight(); // Typical button height
//		float padding = 640.0f;
//
//		
//		
//		if (finished) {
//			// Optionally: do something (restart, switch state, etc.)
//			//approach_start = ImGui::GetTime(); // to loop
//
//			
//			//ImGui::TextUnformatted(LaunchDialog.c_str());
//			if (ImGui::Button("Yes"))
//			{
//				if (showFeedbackWindow)
//				{
//					ImGui::SetNextWindowPos(ImVec2(775, 0));
//					ImGui::SetNextWindowSize(ImVec2(500, 360));
//
//					float box_w = ImGui::GetContentRegionAvail().x; // width the item will use
//					float inner_wrap_px = box_w - ImGui::GetStyle().FramePadding.x * 2.0f;
//					AutoWrapData wrap{ inner_wrap_px };
//
//					ImGui::Begin("Feedback", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse);
//
//					ImGui::InputTextMultiline("##Input",
//						Input_buf,
//						sizeof(Input_buf),
//						ImVec2(500.0f, 650.0f),
//						ImGuiInputTextFlags_CallbackEdit |
//						ImGuiInputTextFlags_NoHorizontalScroll,
//						AutoWrapCallback, &wrap);
//
//					if (ImGui::BeginMenuBar())
//					{
//						if (ImGui::BeginMenu("Menu"))
//						{
//							if (ImGui::MenuItem("Save"))
//							{
//								FILE* file = fopen(SaveFileName.c_str(), "w");
//								if (file)
//								{
//									fprintf(file, "%s", Input_buf);
//									fclose(file);
//								}
//							}
//							ImGui::EndMenu();
//						}
//						ImGui::EndMenuBar();
//					}
//
//
//
//
//					ImGui::End();
//				}
//
//
//				if (showInitialDialog)
//				{
//					ImGui::SetNextWindowPos(ImVec2(775, 360));
//					ImGui::SetNextWindowSize(ImVec2(500, 360));
//					ImGui::Begin("Welcome", nullptr, ImGuiWindowFlags_NoCollapse);
//
//					// Get the full size of the outer window's content area
//					ImVec2 outerSize = ImGui::GetContentRegionAvail();
//					{
//						ImGui::BeginChild("LogBox", ImVec2(outerSize.x, outerSize.y), true);
//
//						ImGui::PushTextWrapPos(0.0f);
//
//
//
//						//THis provides a random Account number to the user
//						//ImGui::Text("%d", vec_account_no[randomElement]);
//
//						//ImGui::TextUnformatted(LaunchDialog.c_str());
//
//						ImGui::PopTextWrapPos();
//
//						// Get size of inner child region
//						ImVec2 childSize = ImGui::GetWindowSize();
//						float buttonWidth = 80.0f;
//						float buttonHeight = ImGui::GetFrameHeight(); // Typical button height
//						float padding = 10.0f;
//
//						// Position cursor near bottom-right
//						ImGui::SetCursorPos(ImVec2(
//							childSize.x - buttonWidth - (padding + 90.0f),
//							childSize.y - buttonHeight - padding
//						));
//
//						if (ImGui::Button("Yes", ImVec2(buttonWidth, 0)))
//						{
//							showInitialDialog = false;
//							showGreetingDialog = true;
//							if (secGrpFlg.getFlag() == 0)
//								runCashTestOne = true;
//							else if (secGrpFlg.getFlag() == 1)
//								runAcctSpecNBTEstOne = true;
//
//
//						}
//
//						// Position cursor near bottom-right
//						ImGui::SetCursorPos(ImVec2(
//							childSize.x - buttonWidth - padding,
//							childSize.y - buttonHeight - padding
//						));
//
//						if (ImGui::Button("No", ImVec2(buttonWidth, 0)))
//						{
//							showInitialDialog = false;
//							showFeedbackWindow = false;
//							showTestingDeadlineDialog = true;
//						}
//
//						ImGui::EndChild();
//					}
//					ImGui::End();
//				}
//
//				if (showGreetingDialog)
//				{
//					ImGui::SetNextWindowPos(ImVec2(775, 360));
//					ImGui::SetNextWindowSize(ImVec2(500, 360));
//					ImGui::Begin("Instructions", nullptr, ImGuiWindowFlags_NoCollapse);
//
//					ImGui::End();
//				}
//
//				if (showTestingDeadlineDialog)
//				{
//					ImGui::SetNextWindowPos(ImVec2(0, 0));
//					ImGui::SetNextWindowSize(ImVec2(1280, 720));
//					ImGui::Begin("Directive", nullptr, ImGuiWindowFlags_NoCollapse);
//
//					ImGui::End();
//				}
//
//
//				if (runCashTestOne)
//					cashTest.test_1_S2();
//			}
//
//
//			
//		}
//
//		
//			
//			
//		
//
//		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
//		//ImGui::End();
//	
//
//		// Rendering
//		ImGui::Render();
//		int display_w, display_h;
//		glfwGetFramebufferSize(window, &display_w, &display_h);
//		glViewport(0, 0, display_w, display_h);
//		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
//		glClear(GL_COLOR_BUFFER_BIT);
//		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//
//		glfwSwapBuffers(window);
//
//	}	
//	
//#ifdef __EMSCRIPTEN__
//	EMSCRIPTEN_MAINLOOP_END;
//#endif
//
//	// Cleanup
//	ImGui_ImplOpenGL3_Shutdown();
//	ImGui_ImplGlfw_Shutdown();
//	ImGui::DestroyContext();
//
//	glfwDestroyWindow(window);
//	glfwTerminate();
//
//}
//
//
////std::cout << randomElement;
////ImGui::Text("%d", randomElement);