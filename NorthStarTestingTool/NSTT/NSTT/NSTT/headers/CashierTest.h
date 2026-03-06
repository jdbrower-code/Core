#pragma once
#include <random>													// For std::mt19937, std::uniform_int_distribution
#include <chrono>													// For seeding with time
#include "Sql.h"
#include <iostream>
#include "WordWrap.h"
#include "Save.h"
#include "SecGroupFlags.h"



class cashierTest
{
private:
	nanodbc::connection CashierTest_Sql;							//DB Connection
	std::vector<int> test1;											//to store available account numbers
	std::vector<std::string> cashTasks;								//Vector to hold all test 1 Tasks
	std::vector<std::string> cashTaskDesc;							//Vector to hold all test 1 Tasks Descriptions
	std::vector<std::string> callCodeName;							//Vector to hold the readable name of the call code
	std::vector<std::string> callCode;								//Vector to hold the actual call codes
	

	bool getAcctNum = true;											//to prevent account number pull loop
	//bool Q1_Text_Flag = true;
	int QuestMark = 0;

	//CashTest ButtonFlags
	bool CT_saveBtn1 = false;
	bool CT_nextBtn = false;
	
	bool ShowCashDialog1 = true;
	bool ShowCashDialog2 = false;
	bool TestPassed = false;
	bool GetCallCode = true;

	int QuestionLoopMarker = 0;										//Repeat Question Loop Marker
	int m_temp_acct_num = 0;										//To hold selected account number
	int CallCodeIndex = 0;
	int CT_nextBtnClicked = 0;
	std::string m_temp_user_id = "";								//Holds short login of tester for Table input
	std::string m_temp_sec_group = "";								//Holds security group of tester for Table input
	std::string m_date = "";										//Holds Date of test for Table input
	std::string m_Call_Code_Name = "";								//Holds selected Human Readable Call Code Name
	std::string m_Call_Code = "";									//Holds selected Call Code
	std::string m_test_passed = "";

	std::string DescriptionPreface = "\n\nFor this task you will: \n--";

	int taskCounter = 3;
	char Input_buf[4096];											//storage buffer
	
	std::string SaveFileName = FileName();
	std::string CashierTaskOneTestQ1;								// Instruction Dialog
	
public:
	cashierTest();
	~cashierTest();
	int randomElementInt(std::vector<int> a);						//Selects a random entry from a Vector of account numbers
	int randomElementStrIndex(std::vector<std::string> a);
	int test_1_GetAccountNumber();									//Returns a Random account number

	void SetCashierTest_Sql(nanodbc::connection conn);
	
	
	void fillTaskVectors();
	std::string task(int questNum);
	std::string taskDesc(int questNum);
	bool CloseProgram = false;
	
	
	//void test_1_Test();											//Runs all aspects of the first test
	void T1_CashTest();

	void T1Q1();
	void T1Q2();
	void T1Q3();
	void T1Q4();
	void T1Q5();
	void T1Q6();
	void T1Q7();
	void T1Q8();
	void T1Q9();
	void T1Q10();
	void T1Q11();
	void T1Q12();
	void T1Q13();
	void T1Q14();
	void T1Q15();
	void T1Q16();
	void T1Q17();
	void T1Q18();
	void T1Q19();
	void T1Q20();
	void T1Q21();
	void T1Q22();
	void T1Q23();
	void T1Q24();
	void T1Q25();

	void T1Q1_SelectCallcode();

	void QuestDirInsert();
	void SaveToDB();
	void FeedBackWindow();
	void Close();
};