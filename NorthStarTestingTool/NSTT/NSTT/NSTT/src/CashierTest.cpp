#include "CashierTest.h"
#include <chrono>
#include <format>
#include <imgui.h>


NorthStarSql CS;



cashierTest::cashierTest()
{
	SecGroupFlags secGRP;
	m_temp_acct_num = test_1_GetAccountNumber();
	m_temp_sec_group = secGRP.returnSecGroup();
	m_temp_user_id = CS.GetShortLogin(CS.NSTT_NsLive_SqlConnect());

	auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	m_date = std::format("{:%Y-%m-%d}", now);

}

cashierTest::~cashierTest()
{

};

void cashierTest::SetCashierTest_Sql(nanodbc::connection conn)
{
	CashierTest_Sql = conn;
}

int cashierTest::randomElementInt(std::vector<int> a)
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

int cashierTest::randomElementStrIndex(std::vector<std::string> a)
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

int cashierTest::test_1_GetAccountNumber()
{

	SetCashierTest_Sql(CS.NSTT_NsLive_SqlConnect());

	//Pulls account number that has a balance and has not been used
	try {
		nanodbc::result cashierT1Query = nanodbc::execute(CashierTest_Sql,
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

		if (cashierT1Query.next())
		{
			for (int i = 1; cashierT1Query.next(); ++i)
			{
				int temp = cashierT1Query.get<int>(0);
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

void cashierTest::T1Q1_SelectCallcode()
{

}

void cashierTest::QuestDirInsert()
{
	if (TestPassed)
		m_test_passed = "Passed";
	else
		m_test_passed = "Failed";
	

	try
	{
		nanodbc::statement stmt(CashierTest_Sql);
		nanodbc::prepare(
			stmt,
			NANODBC_TEXT(
				R"(INSERT INTO NorthStarTestingTool.dbo.Used_Accounts 
				(acct_num, used_by, security_group, used_on, applied_test, test_description, 
				test_passed, user_comment) VALUES (?, ?, ?, ?, ?, ?, ?, ?))")
		);


		const std::string task = cashierTest::cashTasks[QuestMark].c_str();
		const std::string desc = cashierTest::cashTaskDesc[QuestMark];
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

void cashierTest::fillTaskVectors()
{
	try
	{
		nanodbc::result cashTaskDescQuery = nanodbc::execute(
			CashierTest_Sql,
			NANODBC_TEXT(R"(
                SELECT QD.task, QD.taskLevelDescription
                FROM NorthStarTestingTool.dbo.Question_Directives as QD
                WHERE QD.testId = 1
            )"));



		while (cashTaskDescQuery.next())
		{
			std::string task = cashTaskDescQuery.get<std::string>(0);
			std::string description = cashTaskDescQuery.get<std::string>(1);

			cashTasks.push_back(task);
			cashTaskDesc.push_back(description);
		}

		// Debug print
		//for (size_t i = 0; i < cashTasks.size(); ++i)
		//{
		//	std::cout << "\n\nTask: " << cashTasks[i]
		//		<< " | Description: " << cashTaskDesc[i]
		//		<< std::endl;
		//}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}

	try
	{
		nanodbc::result callCodeQuery = nanodbc::execute(CashierTest_Sql,
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

std::string cashierTest::task(int questNum)
{
	return cashTasks[questNum];
}

std::string cashierTest::taskDesc(int questNum)
{
	return cashTaskDesc[questNum];
}

void cashierTest::SaveToDB()
{
	if (CT_saveBtn1)
	{
		ImGui::SetCursorPos(ImVec2(580, 350));

		if (ImGui::Button("Save", ImVec2(80.0f, 20.0f)))
		{
			char buf[256];
			std::string Spacer = " ";
			std::string tempcharstring = cashTasks[QuestMark].c_str() + Spacer + std::to_string(QuestionLoopMarker + 1) +
				"\n\nUser: " + m_temp_user_id.c_str() + "\nAccount Number: " + std::to_string(m_temp_acct_num) + "\n";
			std::strcpy(buf, tempcharstring.c_str());

			ShowCashDialog2 = false;
			ShowCashDialog1 = true;



			QuestionLoopMarker++;
			CT_nextBtnClicked = 0;
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
				CT_saveBtn1 = false;
				//CT_nextBtn = true;
				TestPassed = false;
			}

		}
	}
}

void cashierTest::FeedBackWindow()
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

void cashierTest::T1_CashTest()
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
		"\n  of your actions"
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
			T1Q1();
			break;
		case 1:
			T1Q2();
			break;
		case 2:
			T1Q3();
			break;
		case 3:
			T1Q4();
			break;
		case 4:
			T1Q5();
			break;
		case 5:
			T1Q6();
			break;
		case 6:
			T1Q7();
			break;
		case 7:
			T1Q8();
			break;
		case 8:
			T1Q9();
			break;
		case 9:
			T1Q10();
			break;
		case 10:
			T1Q11();
			break;
		case 11:
			//T1Q12();
			QuestMark++;
			break;
		case 12:
			T1Q13();
			break;
		case 13:
			//T1Q14();
			QuestMark++;
			break;
		case 14:
			//T1Q15();
			QuestMark++;
			break;
		case 15:
			T1Q16();
			break;
		case 16:
			T1Q17();
			break;
		case 17:
			T1Q18();
			break;
		case 18:
			T1Q19();
			break;
		case 19:
			T1Q20();
			break;
		case 20:
			T1Q21();
			break;
		case 21:
			T1Q22();
			break;
		case 22:
			T1Q23();
			break;
		case 23:
			//T1Q24();
			QuestMark++;
			break;
		case 24:
			T1Q25();
			break;
		case 25:
			/*Build a function to close out test and place it here*/
			Close();
			break;
	}


}

void cashierTest::T1Q1()
{

	//cashierTest::CT_saveBtn1 = true;
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

	//ShowCashDialog1 = true;
	//ShowCashDialog2 = false;

	if (QuestionLoopMarker < 3)
	{
		int FeedbackLen = strlen(Input_buf);
		std::string Q_Text =
			"Question 1: " + cashierTest::cashTasks[QuestMark] +
			DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
			"Question : " + cashierTest::cashTasks[QuestMark] +
			DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
			"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
			"\n\nNow In Northstar"
			"\nCreate a " + m_Call_Code_Name + " Logged Call"
			"\nDocument each step you take in the box below"
			"\nMake sure to notate the type of logged call you were assigned"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;


		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog1 = true;
		ShowCashDialog2 = false;

	}
}

void cashierTest::T1Q2()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 2: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar create a call that is Utility specific"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q3()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 3: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nYou will need to complete this task 3 times"
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar create a No Charge Change Order"
		"\nDocument each step you take in the box below"
		"\n\nHit the 'Enter' at the end of each steps documentation"
		"\n\tto move to a new line"
		"\n\nMake sure you document the tab you are working in"
		"\n\t[Tab: <current tab>]"
		"\nAnd any and all actions you took at this step."
		"\n\nPROVIDE AS MUCH DETAIL AS POSSIBLE!!!!"
		"\n\tAs Before Pretend your explination will be used to \ntrain future employees"
		"\n\n\n Click Save";


	if (QuestionLoopMarker < 3)
	{

		int FeedbackLen = strlen(Input_buf);

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(680, 380));
		ImGui::Begin("##Cashiers Test", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q4()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 4: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;

	}
}

void cashierTest::T1Q5()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 5: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && (CT_nextBtnClicked > 0 || QuestionLoopMarker > 0))
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog1 = true;
		ShowCashDialog2 = false;
		//ShowCashDialog11= true;
	}
}

void cashierTest::T1Q6()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 6: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog1 = true;
		ShowCashDialog2 = false;
		//ShowCashDialog11= true;
	}
}

void cashierTest::T1Q7()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 7: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog1 = true;
		ShowCashDialog2 = false;
		//ShowCashDialog11= true;
	}
}

void cashierTest::T1Q8()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}

	std::string Q_Text =
		"Question 8: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog1 = true;
		ShowCashDialog2 = false;
	}
}

void cashierTest::T1Q9()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 9: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q10()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 10: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar Update a status From"
		"\nScheduled to Void"
		"\n\nIf there is no Scheduled call to update"
		"\nCreate one, and then update it to Void"
		"\nMake sure you notate everything in the feedback Window"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q11()
{
	
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 11: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for an account with an Overdue Balance"
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--Add a payment arrangement to an active account with an overdue balance"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q12()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 12: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for an account with a standing payment arrangement"
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will Update a Payment Arrangement"
		"\n--Edit date and or dollar amount of an installment"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q13()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 13: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please Run report for all arrangements in system "
		"\n\n Save report to the _DB folder in you i drive"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q14() 
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 14: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for an account that has a standing balance "
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will Set up a PAP with no Budget Plan"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q15()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}
	std::string Q_Text =
		"Question 15: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for an account that has a balance"
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will Set and Alternative Due Date (Extension)"
		"\nSet account to a specific due date"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q16()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 16: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--Add a payment to an account using the Gateway Payment icon in Account Gateway"
		"\nVerify the payment is added and any change is correctly reflected"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q17()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 17: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 3 time"
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--Add different types of payments to various accounts."
		"\nYou will do this for 3 Diffrent Accounts"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && (CT_nextBtnClicked > 0 || QuestionLoopMarker > 0))
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q18()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 18: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please Run Verification and validate payment details "
		"\n\nDocument the steps you took to Run the Verification"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\nSave Verification Listing in the '_DB' folder of your 'i' drive"
		"\nand provide file name in you feedback below"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q19()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 19: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for \n account "
		"\n\nrun Verification and validate payment details"
		"\n\nDocument the steps you took to Run the Verification"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--Run Verification and validate payment details"
		"\nSave Verification Listing in the '_DB' folder of your 'i' drive"
		"\nand provide file name in you feedback below"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q20()
{
	//if (getAcctNum)
	//{
	//	m_temp_acct_num = test_1_GetAccountNumber();
	//	getAcctNum = false;
	//}

	std::string tempString;
	if (QuestionLoopMarker == 0)
	{
		tempString = " One Batch";
	}
	else
	{
		tempString = " Many Batches";
	}

	std::string Q_Text =
		"Question 20: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 2 times"
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nRound " + std::to_string(QuestionLoopMarker + 1) +
		"\n\nNow in NorthStar"
		"\n--Add" + tempString + " of payments" 
		"\nand transfer to main cash batch"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q21()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 21: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for a pending account "
		"\nIf you can not find one create one"
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--Deposit Payment"
		"\nIf applicable, add a customer deposit for a pending account"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q22()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 22: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will process Miscellaneous Cash Transactions"
		"\nIf applicable, enter Miscellaneous Cash Transaction for appropriate fees"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q23()
{

	std::string Q_Text =
		"Question 23: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\tClick the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will Identify five accounts in a lockbox file and note down payment amounts."
		"\nProvide lockbox file name in your feedback below"
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q24()
{
	if (getAcctNum)
	{
		m_temp_acct_num = test_1_GetAccountNumber();
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 24: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
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
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will process a Manual Payment - Cash Only accounts."
		"\nMust have an account with an active CASH ONLY alert."
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::T1Q25()
{
	if (getAcctNum)
	{
		m_temp_acct_num = NULL;
		getAcctNum = false;
	}
	std::string Q_Text =
		"Question 25: " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\n You will need to complete this task 1 time"
		"\n\n In the Northstar CIS please search for an account with a payment arrangement "
		"\n\nOnce the Account populates Enter the Customer Name"
		"\nand the occupant code in the box below"
		"\n\n\t Enter[Cust: <Firstname Lastname>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Enter the occupant code [OC: <Occupant Code>]"
		"\n\t\thit the 'Enter' key to go to a new line"
		"\n\tThen Click the 'Next Button";

	std::string Q_Text_P2 =
		"Question : " + cashierTest::cashTasks[QuestMark] +
		DescriptionPreface + cashierTest::cashTaskDesc[QuestMark] +
		"\n\nNow in NorthStar"
		"\n--You will process a Basic Payment Arrangement"
		"\nTest processing of normal payments against payment arrangements."
		"\n\nDocument each step you take in the box below"
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

		if (ShowCashDialog1)
			ImGui::Text(Q_Text.c_str());
		if (ShowCashDialog2)
			ImGui::Text(Q_Text_P2.c_str());

		float buttonHeight = ImGui::GetFrameHeight();
		ImGui::SetNextWindowPos(ImVec2(300.0f, 300.0f));
		ImGui::SetNextWindowSize(ImVec2(190.0f, 190.0f));

		if (FeedbackLen > 20 && CT_nextBtnClicked < 1)
			CT_nextBtn = true;
		if (FeedbackLen > 200 && CT_nextBtnClicked > 0)
			CT_saveBtn1 = true;

		SaveToDB();

		//Next Button
		if (CT_nextBtn)
		{
			ImGui::SetCursorPos(ImVec2(580, 350));
			if (ImGui::Button("Next", ImVec2(80.0f, 20.0f)))
			{
				ShowCashDialog1 = false;
				ShowCashDialog2 = true;
				CT_nextBtn = false;
				CT_nextBtnClicked++;
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
		ShowCashDialog2 = false;
		ShowCashDialog1 = true;
	}
}

void cashierTest::Close()
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