#include "AccountSpecTest.h"

NorthStarSql Acsp1;

AcctSpecTest::AcctSpecTest()
{
	SecGroupFlags secGRP;
	m_temp_acct_num = test_1_GetAccountNumber();
	m_temp_sec_group = secGRP.returnSecGroup();
	m_temp_user_id = Acsp1.GetShortLogin(Acsp1.NSTT_NsLive_SqlConnect());

	auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	m_date = std::format("{:%Y-%m-%d}", now);
}
AcctSpecTest::~AcctSpecTest() {};

int AcctSpecTest::randomElementInt(std::vector<int> a)
{
	// 1. Create a random number generator engine
	// Use std::chrono::system_clock::now().time_since_epoch().count()
	// to obtain a time-based seed for better randomness.
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

	// 2. Define a distribution for the random index
	// The range is [0, myVector.size() - 1]
	std::uniform_int_distribution<int> distribution(0, a.size() - 1);

	// 3. Generate a random index
	int randomIndex = distribution(generator);

	// 4. Access the element at the random index
	int randomElement = a[randomIndex];

	return randomElement;
}
int AcctSpecTest::randomElementStrIndex(std::vector<std::string> a)
{
	// 1. Create a random number generator engine
	// Use std::chrono::system_clock::now().time_since_epoch().count()
	// to obtain a time-based seed for better randomness.
	std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

	// 2. Define a distribution for the random index
	// The range is [0, myVector.size() - 1]
	std::uniform_int_distribution<int> distribution(0, a.size() - 1);

	// 3. Generate a random index
	int randomIndex = distribution(generator);

	// 4. Access the element at the random index
	//std::string randomElement = a[randomIndex];

	return randomIndex;
}

int AcctSpecTest::test_1_GetAccountNumber()
{
	SetAcctSpec_Test_Sql(Acsp1.NSTT_NsLive_SqlConnect());

	//Pulls account number that has a balance and has not been used
	try {
		nanodbc::result AcctSpecQuery = nanodbc::execute(AcctSpec_Test_Sql,
			NANODBC_TEXT(R"(
            SELECT acct.account_no
            FROM Northstar_live.harris_live.PU_ACCTV AS acct
            JOIN Northstar_live.harris_live.puaccbal AS bal
                ON acct.account_no = bal.account_no
			LEFT JOIN NorthStarTestingTool.dbo.Used_Accounts as nstt
				ON acct.account_no = nstt.acct_num
            WHERE acct.active_serv > 0
              AND bal.current_balance > 0  
			  AND nstt.acct_num IS NULL 
        )"));

		if (AcctSpecQuery.next())
		{
			for (int i = 1; AcctSpecQuery.next(); ++i)
			{
				int temp = AcctSpecQuery.get<int>(0);
				test1.push_back(temp);
			}
		}
		else
		{



			return 0;
		}

	}
	catch (const nanodbc::database_error& e) {
		std::cerr << "Database error: " << e.what() << std::endl;


	}

	return randomElementInt(test1);
}

void AcctSpecTest::fillTaskVectors()
{
	try
	{
		nanodbc::result cashTaskDescQuery = nanodbc::execute(
			AcctSpec_Test_Sql,
			NANODBC_TEXT(R"(
		            SELECT QD.task, QD.taskLevelDescription
		            FROM NorthStarTestingTool.dbo.Question_Directives as QD
		            WHERE QD.testId = 3
		        )"));



		while (cashTaskDescQuery.next())
		{
			std::string task = cashTaskDescQuery.get<std::string>(0);
			std::string description = cashTaskDescQuery.get<std::string>(1);

			AcctSpec_Tasks.push_back(task);
			AcctSpec_TaskDesc.push_back(description);
		}

		// Debug print
		//for (size_t i = 0; i < AcctSpecNB_Tasks.size(); ++i)
		//{
		//	std::cout << "\n\nTask: " << AcctSpecNB_Tasks[i]
		//		<< " | Description: " << AcctSpecNB_TaskDesc[i]
		//		<< std::endl;
		//}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}

	try
	{
		nanodbc::result callCodeQuery = nanodbc::execute(AcctSpec_Test_Sql,
			NANODBC_TEXT(R"(
					 SELECT cCode.callName, cCode.callType 
					 FROM NorthStarTestingTool.dbo.CashTest_CallType as cCode
	        )"));

		while (callCodeQuery.next())
		{
			std::string tempName = callCodeQuery.get<std::string>(0);
			std::string tempCCode = callCodeQuery.get<std::string>(1);

			callCodeName.push_back(tempName);
			callCode.push_back(tempCCode);
		}
		// Debug print
		//for (size_t i = 0; i < callCodeName.size(); i++)
		//{
		//	std::cout << "Name: " << callCodeName[i] 
		//		<< "\nCode: " << callCode[i] << std::endl;
		//}

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
}

void AcctSpecTest::SetAcctSpec_Test_Sql(nanodbc::connection conn)
{
	AcctSpec_Test_Sql = conn;
}

void AcctSpecTest::QuestDirInsert()
{
	if (TestPassed)
		m_test_passed = "Passed";
	else
		m_test_passed = "Failed";


	try
	{
		nanodbc::statement stmt(AcctSpec_Test_Sql);
		nanodbc::prepare(
			stmt,
			NANODBC_TEXT(
				R"(INSERT INTO NorthStarTestingTool.dbo.Used_Accounts 
				(acct_num, used_by, security_group, used_on, applied_test, test_description, 
				test_passed, user_comment) VALUES (?, ?, ?, ?, ?, ?, ?, ?))")
		);


		const std::string task = AcctSpecTest::AcctSpec_Tasks[QuestMark].c_str();
		const std::string desc = AcctSpecTest::AcctSpec_TaskDesc[QuestMark];
		const std::string uComment = Input_buf;

		stmt.bind(0, &m_temp_acct_num);
		stmt.bind(1, m_temp_user_id.c_str());
		stmt.bind(2, m_temp_sec_group.c_str());
		stmt.bind(3, m_date.c_str());
		stmt.bind(4, task.c_str());
		stmt.bind(5, desc.c_str());
		stmt.bind(6, m_test_passed.c_str());
		stmt.bind(7, uComment.c_str());

		nanodbc::execute(stmt);

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
}

void AcctSpecTest::SaveToDB()
{
	if (Acct_saveBtn1)
	{
		ImGui::SetCursorPos(ImVec2(580, 350));

		if (ImGui::Button("Save", ImVec2(80.0f, 20.0f)))
		{
			char buf[256];
			std::string Spacer = " ";
			std::string tempcharstring = AcctSpec_Tasks[QuestMark].c_str() + Spacer + std::to_string(QuestionLoopMarker + 1) +
				"\n\nUser: " + m_temp_user_id.c_str() + "\nAccount Number: " + std::to_string(m_temp_acct_num) + "\n";
			std::strcpy(buf, tempcharstring.c_str());

			AcctSpecDialog2 = false;
			AcctSpecDialog1 = true;



			QuestionLoopMarker++;
			ACT_nextBtnClicked = 0;
			std::cout << "QuestionLoopMarker = " << QuestionLoopMarker << "\nQuestMark = " << QuestMark << std::endl;
			//FILE* file = fopen(SaveFileName.c_str(), "a");
			std::ofstream out(SaveFileName, std::ios::binary | std::ios::app);
			if (!out)
			{
				std::cerr << "Open failed: " << SaveFileName
					<< " errno=" << errno << "\n";
				return; // or set status
			}
			out << tempcharstring << "\n\n" << Input_buf << "\n\n";
			if (!out)
			{
				std::cerr << "Write failed: " << SaveFileName
					<< " errno=" << errno << "\n";
			}
			else
			{

				//fprintf(file, "%s\n\n", buf);
				//fprintf(file, "%s\n\n", Input_buf);
				//fclose(file);
				getAcctNum = true;
				GetCallCode = true;

				QuestDirInsert();
				Input_buf[0] = '\0';
				Acct_saveBtn1 = false;
				//Acct_nextBtn = true;
				TestPassed = false;
			}

		}
	}
}

void AcctSpecTest::FeedBackWindow()
{
	//Section 2 Begin
	ImGui::SetNextWindowPos(ImVec2(0, 380));
	ImGui::SetNextWindowSize(ImVec2(680, 350));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.082f, 0.822f, 0.941f, 0.812f));




	ImGui::Begin("FeedbackWindow", nullptr, ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoTitleBar);

	ImGui::SetCursorPos(ImVec2(0, 20));

	ImGui::Text("Did the Test Pass?");

	ImGui::SetCursorPos(ImVec2(180, 20));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::Checkbox("Yes", &TestPassed);
	ImGui::PopStyleColor(2);
	ImGui::SetCursorPos(ImVec2(0, 50));
	ImGui::Text("Decribe the actions you took in the box below");

	ImGui::SetCursorPos(ImVec2(10, 75));
	float box_w = ImGui::GetContentRegionAvail().x - 3; // width the item will use
	float inner_wrap_px = box_w - ImGui::GetStyle().FramePadding.x * 2.0f;
	AutoWrapData wrap{ inner_wrap_px };



	ImGui::InputTextMultiline("##Input",
		Input_buf,
		sizeof(Input_buf),
		ImVec2(660, 255),
		ImGuiInputTextFlags_CallbackEdit |
		ImGuiInputTextFlags_NoHorizontalScroll,
		AutoWrapCallback, &wrap);


	ImGui::End();
	ImGui::PopStyleColor();
}

void AcctSpecTest::Close()
{
	std::string Q_Text_P2 =
		"\n\nThank you for taking the time to complete this test."
		"\nYour day-to-day, hands-on knowledge makes you the most"
		"\nqualified person to identify any issues this upgrade may"
		"\nhave on our customers experience and on your own productivity."
		"\n\nWe will carefully review your feedback and address any concerns"
		"\nyou have noted that could negatively impact our customers"
		"\nexperiance or your ability to perform your job efficiently."
		"\n\nOnce again, thank you for your hard work and valuable input. "
		"\n\nYou may now click the END TEST button to close the program"
		"\nand return to your desk."
		"\n\n\n\t\t\tHave a great day and GO TEAM!";


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(680, 720));
	ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::SetWindowFontScale(1.5f);

	ImGui::Text(Q_Text_P2.c_str());

	ImGui::SetWindowFontScale(1.0f);
	float buttonHeight = ImGui::GetFrameHeight();
	ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
	ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));


	ImGui::SetCursorPos(ImVec2(580, 670));
	if (ImGui::Button("End Test", ImVec2(80.0f, 20.0f)))
	{
		CloseProgram = true;
	}


	ImGui::PopStyleColor();
	ImGui::End();
}

void AcctSpecTest::T1_AcctSpecTest()
{
	std::string Directions =
		"\n\nDirections"
		"\n\nYou will see your testing instructions"
		"\nOn the window to the left"
		"\nThis box will guide you through your"
		"\nrequired testing"
		"\n\nIf you compelted the task with no errors"
		"\ncheck the 'Passed' box"
		"\n\nif you encountered issues leave it blank"
		"\n\nIn the text box (Bottom Left) give in"
		"\ndepth details of the actions you took"
		"\n\nWHat to Include"
		"\n--Customers Full Name"
		"\n--Detailed Explination of your actions"
		"\n--Detailed explination of the outcome"
		"\n--of your actions"
		"\n--!!ANY AND ALL ISSUES YOU ENCOUNTERED!!"
		"\n\nA 'Save' button will appear"
		"\nwhen the minumum amount of feedback"
		"\nis provided please continue to"
		"\nprovide feedback until"
		"\nALL DETAILS have been reported"
		"\nTHEN and ONLY THEN click the save button"
		"\n\nClicking the save button will"
		"\nlog your feedback and"
		"\nmove you to the next test";

	ImGui::SetNextWindowPos(ImVec2(680, 0));
	ImGui::SetNextWindowSize(ImVec2(300, 720));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.176f, 0.2f, 0.169f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	ImGui::Begin("##Instruction Window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text(Directions.c_str());
	ImGui::PopStyleColor(2);
	ImGui::End();

	switch (QuestMark)
	{
	case 0:
		ACTQ1();
		break;
	case 1:
		ACTQ2();
		break;
	case 2:
		ACTQ3();
		break;
	case 3:
		ACTQ4();
		break;
	case 4:
		ACTQ5();
		break;
	case 5:
		ACTQ6();
		break;
	case 6:
		ACTQ7();
		break;
	case 7:
		ACTQ8();
		break;
	case 8:
		ACTQ9();
		break;
	case 9:
		ACTQ10();
		break;
	case 10:
		ACTQ11();
		break;
	case 11:
		ACTQ12();
		break;
	case 12:
		ACTQ13();
		break;
	case 13:
		ACTQ14();
		break;
	case 14:
		ACTQ15();
		break;
	case 15:
		ACTQ16();
		break;
	case 16:
		ACTQ17();
		break;
	case 17:
		ACTQ18();
		break;
	case 18:
		ACTQ19();
		break;
	case 19:
		ACTQ20();
		break;
	case 20:
		ACTQ21();
		break;
	case 21:
		ACTQ22();
		break;
	case 22:
		ACTQ23();
		break;
	case 23:
		ACTQ24();
		break;
	case 24:
		ACTQ25();
		break;
	case 25:
		ACTQ26();
		break;
	case 26:
		ACTQ27();
		break;
	case 27:
		ACTQ28();
		break;
	case 28:
		ACTQ29();
		break;
	case 29:
		ACTQ30();
		break;
	case 30:
		ACTQ31();
		break;
	case 31:
		ACTQ32();
		break;
	case 32:
		ACTQ33();
		break;
	case 33:
		ACTQ34();
		break;
	case 34:
		ACTQ35();
		break;
	case 35:
		ACTQ36();
		break;
	case 36:
		ACTQ37();
		break;
	case 37:
		ACTQ38();
		break;
	case 38:
		ACTQ39();
		break;
	case 39:
		ACTQ40();
		break;
	case 40:
		ACTQ41();
		break;
	case 41:
		ACTQ42();
		break;
	case 42:
		ACTQ43();
		break;
	case 43:
		ACTQ44();
		break;
	case 44:
		ACTQ45();
		break;
	case 45:
		ACTQ46();
		break;
	default:
		Close();
		break;
	}
}

void AcctSpecTest::ACTQ1()
{

	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 1: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nIn Northstar search for account " + std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";
			"\n\nDocument each step taken to get to this point"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nNow In Northstar "
			+ AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\nSave Journal Print and provide file name in the _DB folder"
			"\nof your i: drive"
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ2()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 2: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nIn Northstar search for account " + std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";
			"\n\nDocument each step taken to get to this point"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nNow In Northstar "
			+ AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\nSave Journal Print and provide file name in the _DB folder"
			"\nof your i: drive"
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ3()
{

	//AcctSpecNBTest::Acct_saveBtn1 = true;
	if (GetCallCode)
	{
		CallCodeIndex = randomElementStrIndex(callCodeName);
		m_Call_Code_Name = callCodeName[CallCodeIndex];
		m_Call_Code = callCode[CallCodeIndex];
		GetCallCode = false;
	}




	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 3: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 3 times"
			"\nwith 3 Account Numbers"
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nIn the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar "
			"\nAdd different miscellaneous billing charges to accounts and verify they process correctly"
			"\nEnsure accounts are included in the billing journal below so they can be verified."
			"\nDocument each step you take in the box below"
			"\nProvide account numbers and charges/credits"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ4()
{
	if (GetCallCode)
	{
		CallCodeIndex = randomElementStrIndex(callCodeName);
		m_Call_Code_Name = callCodeName[CallCodeIndex];
		m_Call_Code = callCode[CallCodeIndex];
		GetCallCode = false;
	}

	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 4: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to see the current flat rate billing codes"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar "
			"\nVerify number of units, start and end dates, and rate code; bill account"
			"\nDocument each step you take in the box below"
			"\nProvide account numbers and charges/credits"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ5()
{
	if (GetCallCode)
	{
		CallCodeIndex = randomElementStrIndex(callCodeName);
		m_Call_Code_Name = callCodeName[CallCodeIndex];
		m_Call_Code = callCode[CallCodeIndex];
		GetCallCode = false;
	}

	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 5: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nUse the same account you used in the previous test"
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\n\nNow In Northstar "
			"\nVerify The type of account and amount added/subtracted"
			"\nDocument each step you take in the box below"
			"\nProvide account numbers and charges/credits"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ6()
{
	if (GetCallCode)
	{
		CallCodeIndex = randomElementStrIndex(callCodeName);
		m_Call_Code_Name = callCodeName[CallCodeIndex];
		m_Call_Code = callCode[CallCodeIndex];
		GetCallCode = false;
	}

	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 6: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nUse the same account you used in the previous test"
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ7()
{

	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 7: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nUse the same account you used in the previous test"
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ8()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
		//	"\n Call Code: " << m_Call_Code << std::endl;

		//if (getAcctNum)
		//{
		//	m_temp_acct_num = test_1_GetAccountNumber();
		//	getAcctNum = false;
		//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 8: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ9()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
		//	"\n Call Code: " << m_Call_Code << std::endl;

		//if (getAcctNum)
		//{
		//	m_temp_acct_num = test_1_GetAccountNumber();
		//	getAcctNum = false;
		//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 10: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nAt least one account in the journal must have a calculated dollar amount" 
			"\nGreater than the variance percentage when compared to last bill."
			"\nSave Verification Listing and provide file name"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ10()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
		//	"\n Call Code: " << m_Call_Code << std::endl;

		//if (getAcctNum)
		//{
		//	m_temp_acct_num = test_1_GetAccountNumber();
		//	getAcctNum = false;
		//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 10: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ11()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 11: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nPrint bills to PDF, save, and provide file name Flat File:"
			"\nSave bill print file and provide file name;" 
			"\nsave email approval from bill print provider and provide file name"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ12()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;
	
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 12: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nSave Journal Print and provide file name"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ13()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;
	
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 13: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nLoad an account into billing manually; include different account types and configurations"
			"\nsuch as different combinations of services, different categories bill codes, unique accounts"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ14()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 14: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ15()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 15: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ16()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 16: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ17()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 16: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nSave report to the _DB folder in your i Drive"
			"\nprovide file name in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ18()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 16: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nSave report to the _DB folder in your i Drive"
			"\nprovide file name in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ19()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 16: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nSave report to the _DB folder in your i Drive"
			"\nprovide file name in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ20()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;

	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 20: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\nSave report to the _DB folder in your i Drive"
			"\nprovide file name in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ21()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
//	"\n Call Code: " << m_Call_Code << std::endl;

//if (getAcctNum)
//{
//	m_temp_acct_num = test_1_GetAccountNumber();
//	getAcctNum = false;
//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 21: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ22()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
//	"\n Call Code: " << m_Call_Code << std::endl;

//if (getAcctNum)
//{
//	m_temp_acct_num = test_1_GetAccountNumber();
//	getAcctNum = false;
//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 22: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ23()
{
	//std::cout << "Call Code Name: " << m_Call_Code_Name <<
	//	"\n Call Code: " << m_Call_Code << std::endl;
	
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 23: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ24()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 24: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ25()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 25: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ26()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 26: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ27()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 27: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ28()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 28: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nSearch by order type; "
			"\nprint order and select Yes to print all retrieved orders"
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ29()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 29: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ30()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 30: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ31()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 31: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ32()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 32: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ33()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 33: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ34()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 34: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ35()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 35: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ36()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 36: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ37()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 37: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ38() 
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 38: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ39()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 39: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ40()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 40: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ41()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 41: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ42()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 42: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ43()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 43: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ44()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 44: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\nVerify that data was or was not updated as per response."
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ45()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 45: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\nVerify that data was or was not updated as per response."
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
void AcctSpecTest::ACTQ46()
{
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 46: " + AcctSpecTest::AcctSpec_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecTest::AcctSpec_TaskDesc[QuestMark] +
			"\nGo to first record and confirm sort."
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"\nDocument each step you take in the box below"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\nAnd any and all actions you took at this step."
			"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
			"\n\tPretend your explination will be used to train future employees"
			"\n\n\n Click Save";


		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 8 && ACT_nextBtnClicked < 1)
			Acct_nextBtn = true;
		if (FeedbackLen > 200 && ACT_nextBtnClicked > 0)
			Acct_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (Acct_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				AcctSpecDialog1 = false;
				AcctSpecDialog2 = true;
				Acct_nextBtn = false;
				ACT_nextBtnClicked++;
			}
		}

		ImGui::PopStyleColor();
		ImGui::End();

		FeedBackWindow();

	}
	else
	{

		QuestMark++;
		QuestionLoopMarker = 0;
		AcctSpecDialog1 = true;
		AcctSpecDialog2 = false;

	}
}
