#pragma once
#include <random>													// For std::mt19937, std::uniform_int_distribution
#include <chrono>													// For seeding with time
#include "Sql.h"
#include <iostream>
#include "WordWrap.h"
#include "Save.h"
#include "SecGroupFlags.h"



class AcctSpecTest
{
private:
	bool Acct_saveBtn1 = false;
	bool Acct_nextBtn = false;
	//nanodbc::connection AcctSpecTest_Sql = SqlConnect();
	nanodbc::connection AcctSpec_Test_Sql;						//DB Connection
	std::vector<int> test1;											//to store available account numbers
	std::vector<std::string> AcctSpec_Tasks;						//Vector to hold all test 1 Tasks
	std::vector<std::string> AcctSpec_TaskDesc;					//Vector to hold all test 1 Tasks Descriptions
	std::vector<std::string> callCodeName;							//Vector to hold the readable name of the call code
	std::vector<std::string> callCode;

	bool getAcctNum = true;											//to prevent account number pull loop
	int QuestMark = 0;

	bool AcctSpecDialog1 = true;
	bool AcctSpecDialog2 = false;
	bool TestPassed = false;
	bool GetCallCode = true;

	int QuestionLoopMarker = 0;										//Repeat Question Loop Marker
	int m_temp_acct_num = 0;										//To hold selected account number
	int CallCodeIndex = 0;
	int ACT_nextBtnClicked = 0;
	std::string m_temp_user_id = "";								//Holds short login of tester for Table input
	std::string m_temp_sec_group = "";								//Holds security group of tester for Table input
	std::string m_date = "";										//Holds Date of test for Table input
	std::string m_Call_Code_Name = "";								//Holds selected Human Readable Call Code Name
	std::string m_Call_Code = "";									//Holds selected Call Code
	std::string m_test_passed = "";

	std::string DescriptionPreface = "\n\nFor this task you will: \n--";

	int taskCounter = 1;
	char Input_buf[4096];

	std::string SaveFileName = FileName();
	std::string AcctSpecOneTestQ1;
public:
	AcctSpecTest();
	~AcctSpecTest();

	int randomElementInt(std::vector<int> a);
	int randomElementStrIndex(std::vector<std::string> a);
	int test_1_GetAccountNumber();

	void fillTaskVectors();
	void SetAcctSpec_Test_Sql(nanodbc::connection conn);

	std::string task(int questNum);
	std::string taskDesc(int questNum);
	bool CloseProgram = false;

	void T1_AcctSpecTest();
	//
	void QuestDirInsert();
	void SaveToDB();
	void FeedBackWindow();
	void Close();

	void ACTQ1();
	void ACTQ2();
	void ACTQ3();
	void ACTQ4();
	void ACTQ5();
	void ACTQ6();
	void ACTQ7();
	void ACTQ8();
	void ACTQ9();
	void ACTQ10();
	//
	void ACTQ11();
	void ACTQ12();
	void ACTQ13();
	void ACTQ14();
	void ACTQ15();
	void ACTQ16();
	void ACTQ17();
	void ACTQ18();
	void ACTQ19();
	void ACTQ20();
	//
	void ACTQ21();
	void ACTQ22();
	void ACTQ23();
	void ACTQ24();
	void ACTQ25();
	void ACTQ26();
	void ACTQ27();
	void ACTQ28();
	void ACTQ29();
	void ACTQ30();
	//
	void ACTQ31();
	void ACTQ32();
	void ACTQ33();
	void ACTQ34();
	void ACTQ35();
	void ACTQ36();
	void ACTQ37();
	void ACTQ38();
	void ACTQ39();
	void ACTQ40();
	//
	void ACTQ41();
	void ACTQ42();
	void ACTQ43();
	void ACTQ44();
	void ACTQ45();
	void ACTQ46();


};