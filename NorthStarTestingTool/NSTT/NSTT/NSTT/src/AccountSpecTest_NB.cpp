#include "AccountSpecTest_NB.h"

NorthStarSql Acsp;
AcctSpecNBTest CT;

AcctSpecNBTest::AcctSpecNBTest()
{
	SecGroupFlags secGRP;
	m_temp_acct_num = test_1_GetAccountNumber();
	m_temp_sec_group = secGRP.returnSecGroup();
	m_temp_user_id = Acsp.GetShortLogin(Acsp.NSTT_NsLive_SqlConnect());

	auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	m_date = std::format("{:%Y-%m-%d}", now);
}

AcctSpecNBTest::~AcctSpecNBTest()
{

}

int AcctSpecNBTest::test_1_GetAccountNumber()
{
	SetAcctSpecNB_Test_Sql(Acsp.NSTT_NsLive_SqlConnect());

	//Pulls account number that has a balance and has not been used
	try {
		nanodbc::result AcctSpecNBQuery = nanodbc::execute(AcctSpecNB_Test_Sql,
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

		if (AcctSpecNBQuery.next())
		{
			for (int i = 1; AcctSpecNBQuery.next(); ++i)
			{
				int temp = AcctSpecNBQuery.get<int>(0);
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

void AcctSpecNBTest::SetAcctSpecNB_Test_Sql(nanodbc::connection conn)
{
	AcctSpecNB_Test_Sql = conn;
}

int AcctSpecNBTest::randomElementInt(std::vector<int> a)
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

int AcctSpecNBTest::randomElementStrIndex(std::vector<std::string> a)
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

void AcctSpecNBTest::fillTaskVectors()
{
	try
	{
		nanodbc::result cashTaskDescQuery = nanodbc::execute(
			AcctSpecNB_Test_Sql,
			NANODBC_TEXT(R"(
		            SELECT QD.task, QD.taskLevelDescription
		            FROM NorthStarTestingTool.dbo.Question_Directives as QD
		            WHERE QD.testId = 2
		        )"));



		while (cashTaskDescQuery.next())
		{
			std::string task = cashTaskDescQuery.get<std::string>(0);
			std::string description = cashTaskDescQuery.get<std::string>(1);

			AcctSpecNB_Tasks.push_back(task);
			AcctSpecNB_TaskDesc.push_back(description);
		}

		// Debug print
		for (size_t i = 0; i < AcctSpecNB_Tasks.size(); ++i)
		{
			std::cout << "\n\nTask: " << AcctSpecNB_Tasks[i]
				<< " | Description: " << AcctSpecNB_TaskDesc[i]
				<< std::endl;
		}
}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
	
	try
	{
		nanodbc::result callCodeQuery = nanodbc::execute(AcctSpecNB_Test_Sql,
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

void AcctSpecNBTest::QuestDirInsert()
{
	if (TestPassed)
		m_test_passed = "Passed";
	else
		m_test_passed = "Failed";


	try
	{
		nanodbc::statement stmt(AcctSpecNB_Test_Sql);
		nanodbc::prepare(
			stmt,
			NANODBC_TEXT(
				R"(INSERT INTO NorthStarTestingTool.dbo.Used_Accounts 
				(acct_num, used_by, security_group, used_on, applied_test, test_description, 
				test_passed, user_comment) VALUES (?, ?, ?, ?, ?, ?, ?, ?))")
		);


		const std::string task = AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark].c_str();
		const std::string desc = AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark];
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

void AcctSpecNBTest::SaveToDB()
{
	if (Acct_saveBtn1)
	{
		ImGui::SetCursorPos(ImVec2(580, 350));

		if (ImGui::Button("Save", ImVec2(80.0f, 20.0f)))
		{
			char buf[256];
			std::string Spacer = " ";
			std::string tempcharstring = AcctSpecNB_Tasks[QuestMark].c_str() + Spacer + std::to_string(QuestionLoopMarker + 1) +
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

void AcctSpecNBTest::FeedBackWindow()
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

void AcctSpecNBTest::T1_AcctSpecNBTest()
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
		ACNBQ1();
		break;
	case 1:
		ACNBQ2();
		break;
	case 2:
		//ACNBQ3();
		QuestMark++;
		break;
	case 3:
		ACNBQ4();
		break;
	case 4:
		ACNBQ5();
		break;
	case 5:
		ACNBQ6();
		break;
	case 6:
		//ACNBQ7();
		break;
	case 7:
		ACNBQ8();
		break;
	case 8:
		ACNBQ9();
		break;
	case 9:
		ACNBQ10();
		break;
	case 10:
		ACNBQ11();
		break;
	case 11:
		//ACNBQ12();
		break;
	case 12:
		ACNBQ13();
		break;
	case 13:
		ACNBQ14();
		break;
	case 14:
		ACNBQ15();
		break;
	case 15:
		ACNBQ16();
		break;
	case 16:
		ACNBQ17();
		break;
	case 17:
		//ACNBQ18();
		break;
	case 18:
		ACNBQ19();
		break;
	case 19:
		ACNBQ20();
		break;
	case 20:
		ACNBQ21();
		break;
	case 21:
		ACNBQ22();
		break;
	case 22:
		//ACNBQ23();
		break;
	case 23:
		ACNBQ24();
		break;
	case 24:
		//ACNBQ25();
		break;
	case 25:
		ACNBQ26();
		break;
	case 26:
		ACNBQ27();
		break;
	case 27:
		ACNBQ28();
		break;
	case 28:
		ACNBQ29();
		break;
	case 29:
		ACNBQ30();
		break;
	case 30:
		ACNBQ31();
		break;
	case 31:
		ACNBQ32();
		break;
	case 32:
		ACNBQ33();
		break;
	case 33:
		ACNBQ34();
		break;
	case 34:
		ACNBQ35();
		break;
	case 35:
		//ACNBQ36();
		break;
	case 36:
		//ACNBQ37();
		break;
	case 37:
		//ACNBQ38();
		break;
	case 38:
		ACNBQ39();
		break;
	case 39:
		ACNBQ40();
		break;
	case 40:
		ACNBQ41();
		break;
	case 41:
		//ACNBQ42();
		break;
	case 42:
		//ACNBQ43();
		break;
	case 43:
		//ACNBQ44();
		break;
	case 44:
		//ACNBQ45();
		break;
	case 45:
		//ACNBQ46();
		break;
	case 46:
		//ACNBQ47();
		break;
	case 47:
		ACNBQ48();
		break;
	case 48:
		ACNBQ49();
		break;
	case 49:
		ACNBQ50();
		break;
	case 50:
		ACNBQ51();
		break;
	case 51:
		ACNBQ52();
		break;
	case 52:
		ACNBQ53();
		break;
	case 53:
		//ACNBQ54();
		break;
	case 54:
		//ACNBQ55();
		break;
	case 55:
		//ACNBQ56();
		break;
	case 56:
		//ACNBQ57();
		break;
	case 57:
		//ACNBQ58();
		break;
	case 58:
		//ACNBQ59();
		break;
	case 59:
		ACNBQ60();
		break;
	case 60:
		ACNBQ61();
		break;
	case 61:
		ACNBQ62();
		break;
	case 62:
		ACNBQ63();
		break;
	case 63:
		ACNBQ64();
		break;
	case 64:
		ACNBQ65();
		break;
	case 65:
		ACNBQ66();
		break;
	case 66:
		ACNBQ67();
		break;
	case 67:
		ACNBQ68();
		break;
	case 68:
		ACNBQ69();
		break;
	case 69:
		ACNBQ70();
		break;
	case 70:
		ACNBQ71();
		break;
	case 71:
		ACNBQ72();
		break;
	case 72:
		ACNBQ73();
		break;
	case 73:
		ACNBQ74();
		break;
	case 74:
		ACNBQ75();
		break;
	case 75:
		ACNBQ76();
		break;
	case 76:
		ACNBQ77();
		break;
	case 77:
		ACNBQ78();
		break;
	case 78:
		ACNBQ79();
		break;
	case 79:
		ACNBQ80();
		break;
	case 80:
		ACNBQ81();
		break;
	case 81:
		//ACNBQ82();
		break;
	case 82:
		//ACNBQ83();
		break;
	case 83:
		//ACNBQ84();
		break;
	case 84:
		//ACNBQ85();
		break;
	case 85:
		ACNBQ86();
		break;
	case 86:
		//ACNBQ87();
		break;
	case 87:
		ACNBQ88();
		break;
	case 88:
		//ACNBQ89();
		break;
	case 89:
		ACNBQ90();
		break;
	case 90:
		//ACNBQ91();
		break;
	case 91:
		ACNBQ92();
		break;
	default:
		Close();
		break;
	}
	
}

void AcctSpecNBTest::ACNBQ1()
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
			"Question 1: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
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
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ2()
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
			"Question 2: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to see the current flat rate billing codes"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ3()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 3: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to see the current flat rate billing codes"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nNow In Northstar"
			"\nBegin Proofing meter readings"
			"\nDocument each step you take in the box below"
			"\nProvide account numbers and charges/credits"
			"\n\nHit the 'Enter' at the end of each steps documentation"
			"\n\tto move to a new line"
			"\n\nMake sure you document the tab you are working in"
			"\n\t[Tab: <current tab>]"
			"\n\t\tAnd any and all actions you took at this step."
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ4()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 4: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps will you have to take to prepare to print the Journal"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nNow In Northstar"
			"\nRun Journal print; save to _DB folder located in your i drive"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ5()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}

	

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 5: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 time for each post"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nNow In Northstar"
			"\nPost the billing journal of an account that has been Finaled"
			"\nsave to the _DB folder located in your i drive"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked  < 1)
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
void AcctSpecNBTest::ACNBQ6()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 6: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nSearch for an account with a move out service order"
			"\nDocument the account number and customer name and address below"
			"\n\n\t Notate the steps you take to start this process"
			"\n\t\tWhat Tab you will be working in"
			"\nWhat actions to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nManually load accounts with move out service orders into billing and attach appropriate service orders"
			"\nload accounts from mCare into billing and review attached service orders"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ7()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 7: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nDocument the steps you would need to do to reprint the Billing Journal Report"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nReprint the billing Journal Report"
			"\nSave the file in the _DB folder of your i drive"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ8()
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
			"Question 8: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\n In the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\nDocument the steps you would need to do to reprint the Balance History Report"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nReprint the Balance History Report"
			"\n\tACCT GATEWAY > MAIN SELECTIONS > ACCOUNT BALANCE HISTORY > TOOLBAR BUTTONS"
			"\n\tSelect billing item in balance history and click Reprint button on toolbar"
			"\nSave the file in the _DB folder of your i drive"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ9()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 9: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\n In the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\nDocument the steps you would need to do to reprint a bill"
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nReprint the bill"
			"\n\tACCT GATEWAY > MAIN SELECTIONS > ACCOUNT BALANCE HISTORY > TOOLBAR BUTTONS"
			"\n\tSelect billing item in balance history and click Reprint Bill  button on toolbar."
			"\n\tCopy of bill should display with all printer's tags"
			"\nSave the file in the _DB folder of your i drive"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ10()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 10: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\nDocument each step taken to get to this point"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\n\tACCT GATEWAY > MAIN SELECTIONS > ACCOUNT BALANCE HISTORY > STATISTIC"
			"\n\tSelect billing item in balance history and click Statistic tab"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ11()
{
	if (GetCallCode)
	{
		CallCodeIndex = randomElementStrIndex(callCodeName);
		m_Call_Code_Name = callCodeName[CallCodeIndex];
		m_Call_Code = callCode[CallCodeIndex];
		GetCallCode = false;
	}

	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 11: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\t\tWhat Tab you will be working in"
			"\n\t\tWhat steps are needed to prepare to post a batch"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nCreate a " + m_Call_Code_Name + " Logged Call"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked  < 1)
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
void AcctSpecNBTest::ACNBQ12()
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
			"Question 12: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\n In the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nNow In Northstar"
			"\n\nNow in NorthStar create a call that is Utility specific"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ13()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 13: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 3 times"
			"\nwith 3 Account Numbers"
			"\n\n In the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\n\nNow in NorthStar Add one or more service orders with no attached charges."
			"\nExamples might include meter change, read meter, trim trees, etc "
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ14()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 14: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 3 times"
			"\nwith 3 Account Numbers"
			"\n\n In the Northstar CIS please search for \n account " +
			std::to_string(m_temp_acct_num) +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\n\nNow in NorthStar Add a Field order which includes a charge"
			"\nMake sure to confirm and document the pending CARe charges "
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ15()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}

	if (QuestionLoopMarker < 1)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 15: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nYou will need to complete this task 1 times"
			"\nIn Northstar"
			"\n\tClick on Advanced Search"
			"\n\tDouble click on --Active(Y/N)--"
			"\n\tClick on the --Equals-- bubble"
			"\n\tType a 'Y' into the --Equals-- textbox"
			"\n\tClick Ok"
			"\n\nIn the Advanced Search box scroll down and double click on --Serv street--"
			"\n\tClick on the --Equals-- bubble"
			"\n\tEnter any street name that exists in San Marcos"
			"\n\tClick Ok"
			"\n\tClick Ok"
			"\n\nIN the box below Enter the Your Name, Your Short Login, and the street that you choose"
			"\nHit enter to place each entry on a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nNow In Northstar"
			"\n\nClick on the --Generate Service Order Button-- on the tollbar"
			"\n\tIn the pop up Window click --Yes--"
			"\n\tIn The --Generate Orders for ALL Accounts Found?-- Window click Yes"
			"\n\tDouble Click on --Customer Request Log--"
			"\n\tSelect --O. Tree Triming-- and click Ok"
			"\n\tSet the scheduled date to any day of your choosing"
			"\n\tEnter Call notes and click ok"
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ16()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 16: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 times"
		"\nwith 3 Account Numbers"
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar create a Move Out Order"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used \nto train future employees"
		"\n\n\n Click Save";

	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ17()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 17: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 times"
		"\nwith 3 Account Numbers"
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar create a Move In Order"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used \nto train future employees"
		"\n\n\n Click Save";

	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ18()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 18: " +AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface +AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 times"
		"\nwith 3 Account Numbers"
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " +AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface +AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar move the Customer out of"
		"\nthe address they currently reside in and into"
		"\nanother address of your choosing"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";

	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		//ShowCashDialog11= true;
	}
}
void AcctSpecNBTest::ACNBQ19()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 19: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 times"
		"\nwith 3 Account Numbers"
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar Generate a Disconnect Order"
		"\nand confirm soft alert shows a pending disconnect"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";

	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		//ShowCashDialog11= true;
	}
}
void AcctSpecNBTest::ACNBQ20()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 20: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 times"
		"\nwith 3 Account of your choosing"
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\n In the Northstar CIS please search for an account that is diconnected"
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar Generate a Reconnect Order"
		"\nand confirm soft alert shows a pending reconnect"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used \nto train future employees"
		"\n\n\n Click Save";

	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ21()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 21: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar Edit a logged call"
		"\nUpdate Notes and Scheduled Notes"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ22()
{
	std::string Q_Text =
		"Question 21: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please click on the seach button on the tool bar"
		"\n\tExpand the CARe menu"
		"\n\tClick on 'Call Maintenace'"
		"\n\tIn the Call Maintenace window set the Status (bottom right) to 'S'"
		"\n\tSelect One of the Generated accounts"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tClick the eidt button on the toolbar (looks like a pencil)"
		"\n\tChange any field except the status field"
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail the change to the service order you made"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ23()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string temp = "";
	if (QuestionLoopMarker == 0)
	{
		temp =
			"\n\tExpand 'CUSTOMER REQUESTED RE-READ'"
			"\n\tSelect a service the customer has (Electric or Water)"
			"\n\tDocument Data in the Add Call window"
			"\n\tClick OK";
	}
	else
	{
		temp =
			"\n\tCreate a 'Field Order' of you choosing"
			"\n\tAs Long as it has a charge assigned to it"
			"\n\tDocument Data in the Add Call window"
			"\n\tClick OK";
	}
		 

	std::string Q_Text =
		"Question 23: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tGo to the 'Calls' Tab" +
		temp +
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail the change to the service order you made"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 2)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ24()
{
	std::string Q_Text =
		"Question 24: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please click on the Advanced seach button on the tool bar"
		"\n\tDouble Click on 'Active Y/N'"
		"\n\tSelect 'Equals' and insert a 'Y' into the textbox"
		"\n\tDouble Click on 'Name'"
		"\n\tSelect 'Equals' and insert 'VACANT' into the textbox"
		"\n\tClick OK"
		"\n\tSelect One of the Generated accounts"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tComplete a Move in on the account"
		"\n\tAfter you have it scheduled manually complete the order"
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail the change to the service order you made"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ25()
{
	std::string temp = "";
	if (QuestionLoopMarker == 0)
	{
		temp =
			"\n\tClick on the Advance Search in the tool bad"
			"\n\tDouble click on 'Active (Y/N)'"
			"\n\tSelect Equals and input 'Y' in the text box"
			"\n\tClick OK"
			"\n\tSelect one of the Generated Accounts"
			"\n\tComplete a Manual Disconnect of one of the accounts services";
	}
	else
	{
		temp =
			"\n\tManually Reconnect the service you disconnected in the last step";

	}


	std::string Q_Text =
		"Question 23: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"+
		temp +
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail the change to the service order you made"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 2)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ26()
{
	std::string Q_Text =
		"Question 26: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please click on the seach button on the tool bar"
		"\n\tExpand the CARe menu"
		"\n\tClick on 'Call Maintenace'"
		"\n\tIn the Call Maintenace window set the Status (bottom right) to 'S'"
		"\n\tSelect One of the Generated accounts"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tIn the Call Maintenance window click on the Browse button"
		"\n\tDouble click on a call that has not been completed"
		"\n\tIn the Call Maintenace window click on the Status Change Tab"
		"\n\tClcik the Edit button (looks like a pencil)"
		"\n\t\Change the 'Status' from 'S' to 'V'"
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail the change to the service order you made"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ27()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nSearch for an order Using a Single Criteria on Screen (Normal Search)"
			"\n\tEx: Status, Createdby, cycle, ect.";
	}
	else if (QuestionLoopMarker > 0 && QuestionLoopMarker < 3)
	{
		temp =
			"\nSearch for an order Using a Single Criteria Diffrent from" 
			"\nPrevious used on Screen (Normal Search)"
			"\n\tEx: Status, Createdby, cycle, ect.";
	}
	else if (QuestionLoopMarker >= 3)
	{
		temp =
			"\nSearch for an order using the Multiple Criteria"
			"\n\tEx: Status, Createdby, cycle, ect.";
	}

	std::string Q_Text =
		"Question 27: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nYou will need to complete this task multiple times"
		"\n\tfor both single critera and multiple criteria"
		"\n\nIn the Northstar CIS please click on the seach button on the tool bar" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + temp +
		"\n\tMake sure you documnent evey aspect of the process"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 6)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ28()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nSearch for an order Using a Single Criteria in the Advanced Find"
			"\n\tEx: scheduled between, cycle less than x, ect.";
	}
	else if (QuestionLoopMarker > 0 && QuestionLoopMarker < 3)
	{
		temp =
			"\nIn the Advanced Find Search for an order Using a Single Criteria Diffrent from"
			"\nPrevious used on Screen (Normal Search)"
			"\n\tEx: scheduled between, cycle less than x, ect.";
	}
	else if (QuestionLoopMarker >= 3)
	{
		temp =
			"\nSearch for an order using the Multiple Criteria in the Advanced Find"
			"\n\tEx: scheduled between, cycle less than x, ect.";
	}

	std::string Q_Text =
		"Question 28: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nYou will need to complete this task multiple times"
		"\n\tfor both single critera and multiple criteria"
		"\n\nIn the Northstar CIS please click on the Advanced Find button on the tool bar" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + temp +
		"\n\tMake sure you documnent evey aspect of the process"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 6)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ29()
{
	std::string Q_Text =
		"Question 29: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS Account Gateway Tab"
		"\n\tExpand the CARe menu"
		"\n\tClick on 'Call Maintenace'"
		"\n\tIn the search window click the drop down next to the word 'Type'"
		"\n\tIts Located in the 'Call Information' box on the right hand side"
		"\n\tSelect One of the types and click 'OK'"
		"\n\tSelect one of the accounts that populated"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"\nIn the 'Call Maintenace' window click on the 'Browse' tab"
		"\n\tFind the call type you searched for and double click on it"
		"\n\nPrint The document"
		"\n\nClick the search icon on the toolbar"
		"\n\nSelct another option from the drop down nest to Type"
		"\n\tComplete the same steps as above"
		"\n\tPrint this one as well"
		"\n\tDo this for 5 diffrent Call Types"
		"\n\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ30()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 30: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\t" + std::to_string(m_temp_acct_num) + " and " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		"\n\nDocument the Account"  
		"\n\tEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nDocument each step you take in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ31()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 31: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for account "
		"\n\t" + std::to_string(m_temp_acct_num) +
		"\n\tEnter the Customer Name"
		"\n\tthe occupant code"
		"\n\tand the customer number in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the customer number [CN: <Customer Number>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow click the search button on the tool bar"
		"\n\tsearch by the customer number you documented in the last step" 
		"\n\tConfirm that you get the same account by macthing the account name and occupant code"
		"\n\twith the name and occupant code you retrived when you searched by the acct Number"
		"\n\tDocument each step you take in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ32()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 32: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for account "
		"\n\t" + std::to_string(m_temp_acct_num) +
		"\n\tEnter the Customer Name"
		"\n\tthe occupant code"
		"\n\tand the Service Address in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the customer number [CN: <Customer Number>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow click the search button on the tool bar"
		"\n\tsearch by the Service Address you documented in the last step"
		"\n\tFind the Account in the drop down of Accounts"
		"\n\tConfirm that you get the same account by macthing the account name and occupant code"
		"\n\twith the name and occupant code you retrived when you searched by the acct Number"
		"\n\tDocument each step you take in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ33()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 33: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for account "
		"\n\t" + std::to_string(m_temp_acct_num) +
		"\n\tEnter the Customer Name"
		"\n\tthe occupant code"
		"\n\tand the Phone Number in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the customer number [CN: <Customer Number>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow click the search button on the tool bar"
		"\n\tsearch by the Phone Number you documented in the last step"
		"\n\tConfirm that you get the same account by macthing the account name and occupant code"
		"\n\twith the name and occupant code you retrived when you searched by the acct Number"
		"\n\tDocument each step you take in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ34()
{

	std::string Q_Text =
		"Question 34: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS Click on the search Button"
		"\n\tThen click on the Account Details tab in the search window"
		"\n\tPlace 1,2, 3, or 4 into the cycle box and click ok"
		"\n\tSelect any account from the ones found"
		"\n\tEnter the Customer Name and occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the customer number [CN: <Customer Number>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tDocument each step you took in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ35()
{
	std::string Q_Text =
		"Question 35: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS Click on the search Button"
		"\n\tThen click on the Account Details tab in the search window"
		"\n\tclick the drop down next for Class and select one of your choosing"
		"\n\t!!!--IF YOU CHOOSE RESIDENTIAL PUT 1, 2, 3, OR 4 IN THE CYCLE BOX--!!!"
		"\n\tand click ok"
		"\n\tSelect any account from the ones found"
		"\n\tEnter the Customer Name and occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the customer number [CN: <Customer Number>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tDocument each step you took in the box below"
		"\n\tMake sure to detail all aspects of the account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ36()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\n\tSearch for Service Details Using a Single Criteria";
	}


	std::string Q_Text =
		"Question 36: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ37()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\n\tSearch for Service Details Using a Single Criteria";
	}


	std::string Q_Text =
		"Question 37: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar"
		"\n\tUsing the options under Service Address " +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ38()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\n\tSearch for Service Details Using a Single Criteria";
	}


	std::string Q_Text =
		"Question 38: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar"
		"\n\tUsing the options under Service Address " +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ39()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nSearch for an Account Using a Multiple Criteria";
	}


	std::string Q_Text =
		"Question 39: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar" 
		"\nClick on the 'Account Details' tab and " +
		temp +
		"\n\tEx: SSN, Cycle, Class, Addresss"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ40()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nSearch for an Account Using a Multiple Criteria";
	}


	std::string Q_Text =
		"Question 40: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar"
		"\nClick on the 'Account Details' tab and " +
		temp +
		"\n\tEx: SSN, Cycle, Class, Addresss"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ41()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nSearch for an Account Using a Multiple Criteria";
	}


	std::string Q_Text =
		"Question 41: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please click on the search button on the tool bar"
		"\nClick on the 'Account Details' tab and " +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ42()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'Transaction Amount'"
			"\nClick on 'Greater Than' and put any number over 1000 into the box";
	}


	std::string Q_Text =
		"Question 42: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'" 
		"\nIn the 'Account Balance History' window click on the Advanced Search button"+
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ43()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'GL Year'"
			"\nClick on 'Equals' and enter any year up to the current one";
	}


	std::string Q_Text =
		"Question 43: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'"
		"\nIn the 'Account Balance History' window click on the Advanced Search button" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ44()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'Jour Code'"
			"\nClick on 'Equals' and enter either 'WO' or 'EC' then click ok";
	}


	std::string Q_Text =
		"Question 44: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'"
		"\nIn the 'Account Balance History' window click on the Advanced Search button" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ45()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'GL Year'"
			"\nClick on 'Equals' and enter any year up to the current one"
			"\nDouble click on 'Jour Code' and enter either 'CR' or 'LP'"
			"\nClick Ok";
	}


	std::string Q_Text =
		"Question 45: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'"
		"\nIn the 'Account Balance History' window click on the Advanced Search button" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ46()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'Operator'"
			"\nClick on 'Equals' and enter Your short login"
			"\nDouble click on 'Previous Balance' and direct it to search between 500 and 1000"
			"\nClick Ok";
	}


	std::string Q_Text =
		"Question 46: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'"
		"\nIn the 'Account Balance History' window click on the Advanced Search button" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ47()
{
	std::string temp = "";

	if (QuestionLoopMarker < 1)
	{
		temp =
			"\nDouble click on 'GL Month'"
			"\nClick on 'Equals' and enter a number between 1 and 12"
			"\nDouble click on 'GL Year' and enter any year up till this one"
			"\nClick Ok";
	}


	std::string Q_Text =
		"Question 47: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Main Selections' menu"
		"\nDouble Click on 'Balance History'"
		"\nIn the 'Account Balance History' window click on the Advanced Search button" +
		temp +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ48()
{
	std::string Q_Text =
		"Question 48: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Additional Information' menu"
		"\nDouble Click on 'Alerts'"
		"\nIn the 'Alert Code' Drop down select an option of your choosing and hit OK"
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ49()
{
	std::string Q_Text =
		"Question 49: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Additional Information' menu"
		"\nDouble Click on 'Alerts'"
		"\nIn the 'Alert Code' Drop down select an option of your choosing and hit OK"
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ50()
{
	std::string Q_Text =
		"Question 50: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please Expand the 'Additional Information' menu"
		"\nDouble Click on 'Alerts'"
		"\nIn the 'Alert Code' Drop down select an option of your choosing and hit OK"
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ51()
{
	std::string Q_Text =
		"Question 51: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ52()
{ 
	std::string Q_Text =
		"Question 52: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ53()
{ 
	std::string Q_Text =
		"Question 53: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ54()
{ 
	std::string Q_Text =
		"Question 54: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ55()
{ 
	std::string Q_Text =
		"Question 55: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ56()
{ 
	std::string Q_Text =
		"Question 56: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\tEspecially the Alerts that are shown from the search"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ57()
{ 
	std::string Q_Text =
		"Question 57: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ58()
{ 
	std::string Q_Text =
		"Question 58: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ59()
{ 
	std::string Q_Text =
		"Question 59: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ60()
{ 
	std::string Q_Text =
		"Question 60: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ61()
{ 
	std::string Q_Text =
		"Question 61: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSelect one of the accounts that is returned"
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ62()
{ 
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 64: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ63()
{ 
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 64: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ64()
{ 
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 64: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ65()
{ 
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 65: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ66()
{ 
	std::string Q_Text =
		"Question 66: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nDocument the Account you selected"
		"\nEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ67()
{ 
	std::string Q_Text =
		"Question 67: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nDocument the Account you selected"
		"\nEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ68()
{ 
	std::string Q_Text =
		"Question 68: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nDocument the Account you selected"
		"\nEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tMake sure you documnent all major aspects of the account"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ69()
{ 

	std::string temp = "";

	if (QuestionLoopMarker == 0)
		temp = "an account with an active service";
	else if (QuestionLoopMarker == 1)
		temp = "an account with a service finaled in the last 89 days";
	else
		temp = "an account with a service finaled more than 91 days ago";

	std::string subQuestion = std::to_string(QuestionLoopMarker + 1);

	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 69." + subQuestion + ":" + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			"\n\nYou will need to complete this task 3 times"
			"\nwith 3 Account Numbers"
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nIn the Northstar CIS please search for " + temp +
			"\n\nOnce the Account populates Enter the Customer Name"
			"\nand the occupant code in the box below"
			"\n\n\t Enter[Cust: <Firstname Lastname>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
			"\n\t\thit the 'Enter' key to go to a new line"
			"\n\tThen Click the 'Next Button;";

		std::string Q_Text_P2 =
			"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
			DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar search for " +
			temp +
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

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
void AcctSpecNBTest::ACNBQ70()
{
	std::string Q_Text =
		"Question 68: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSearch by Name in Acccount Summary but only use 2 letters followed by the wildcard symbol"
		"\nRepeat until you get more than 1000 accounts"
		"\n\nDocument the Top Account"
		"\nEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tClick in the white box that show the number of accounts"
		"\n\t\tSort by name Ascending"
		"\n\t\tDocument the top account"
		"\n\tClick in the white box that show the number of accounts"
		"\n\t\tSort by Street Number Descending"
		"\n\t\tDocument the top account"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ71()
{
	std::string Q_Text =
		"Question 71: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nSearch by Name in Acccount Summary but only use 2 letters followed by the wildcard symbol"
		"\nRepeat until you get more than 500 accounts"
		"\n\nDocument the Top Account"
		"\nEnter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tClick in the white box that show the number of accounts"
		"\n\tAt the bottom of the window click the button with a single arrow pointing down"
		"\n\t\tIf there are more than a 1000 rows click yes on the popup"
		"\n\tSelect Excel from the next window and click OK"
		"\n\nRepeat this process but this time for the button with 2 overlapping arrows"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ72()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 72: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tMake sure you documnent At laest 3 Activities you find on the Calander"
		"\n\tAim to provide 6 to 9 Activites if possible"
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ73()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 73: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tDocument all ativity on the account in the last 60 days" 
		"\n\t\tProvide any detail that you think would help others recreate your search"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ74()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 74: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tClick on the Decrypt button on the tool bad"
		"\n\t\tDocumnet what fields provided additional information"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ75()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 75: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for \n account " +
		std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar " + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\tClick on the Scan Old Accounts button on the tool bad"
		"\n\t\tDid you find any old accounts?"
		"\n\t\tIf Yes do any of them have a balance?"
		"\n\t\t\t\If Yes Select one (with balance if possible)"
		"\n\t\t\tDocument the full address of the account and the blance if one exists"
		"\n\t\tIf No Document that they didnt and the full address of current customer"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ76()
{
	std::string Q_Text =
		"Question 76: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nEnter the Your Name and Your short login"
		"\n(initials plus empoyee id - ex jb12345) in the box below"
		"\n\n\t Enter[Name: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the Short Login [Login: <Short Login>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tClose any and all tabs you have open in the bottom window"
		"\n\tNow double click on"
		"\n\t\tBalance History"
		"\n\t\tService Details"
		"\n\t\tService Summary"
		"\n\t\tand Call Maintenance"
		"\n\nClick the save icon next on the profile toolbar"
		"\n\tName the profile (<your initials>-NS68-Test"
		"\n\t\tEx: JB-NS68-Test"
		"\nDocument all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ77()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 77: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tUse your new profile to populate account info"
		"\nDocument the tabs the profile produced"
		"\nand the info populated in them"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ78()
{
	std::string Q_Text =
		"Question 78: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nWithout clearing the previous account Enter your Name in the box below"
		"\n\n\t Enter[Name: <Your Name>]"
		"\n\t\tAnd Your Shory login"
		"\n\t [Login: <short login> Ex:JB12345]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tSwitch to and load a diffrent profile"
		"\nDefault is fine if it is the only other option"
		"\nDocument the tabs the changes produced"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ79()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 79: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tOpen the Service Summary and \n" +
		AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the tabs the screens produced"
		"\nand the info populated in them"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}

void AcctSpecNBTest::ACNBQ80()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 80: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tOpen the Additional Customer Info menu and \n" +
		AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument each addition you made"
		"\nand the info populated in them"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ81()
{

	std::string Q_Text =
		"Question 81: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for a Commercial account " 
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\tOpen the Additional Customer Info menu and \n" +
		AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the tabs the screens produced"
		"\nand the info populated in them"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ82()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 82: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ83()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 83: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ84()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 84: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ85()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 85: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ86()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 86: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ87()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 87: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ88()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 88: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ89()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 89: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}

void AcctSpecNBTest::ACNBQ90()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 90: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ91()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 91: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::ACNBQ92()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 92: " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nIn the Northstar CIS please search for account " + std::to_string(m_temp_acct_num) +
		"\n\nOnce the Account has been selected Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + AcctSpecNBTest::AcctSpecNB_Tasks[QuestMark] +
		DescriptionPreface + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n\t" + AcctSpecNBTest::AcctSpecNB_TaskDesc[QuestMark] +
		"\nDocument the Actions you take"
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used"
		"\nto train future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 1)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (AcctSpecDialog1)
			ImGui::Text(Q_Text.c_str());
		if (AcctSpecDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && ACT_nextBtnClicked < 1)
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
		AcctSpecDialog2 = false;
		AcctSpecDialog1 = true;
	}
}
void AcctSpecNBTest::Close()
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

