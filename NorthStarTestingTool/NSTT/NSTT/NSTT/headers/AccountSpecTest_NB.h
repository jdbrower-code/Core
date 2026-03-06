#pragma once
#include <random>													// For std::mt19937, std::uniform_int_distribution
#include <chrono>													// For seeding with time
#include "Sql.h"
#include <iostream>
#include "WordWrap.h"
#include "Save.h"
#include "SecGroupFlags.h"
#include "CashierTest.h"



class AcctSpecNBTest
{
private:
	bool Acct_saveBtn1 = false;
	bool Acct_nextBtn = false;
	//nanodbc::connection AcctSpecTest_Sql = SqlConnect();
	nanodbc::connection AcctSpecNB_Test_Sql;						//DB Connection
	std::vector<int> test1;											//to store available account numbers
	std::vector<std::string> AcctSpecNB_Tasks;						//Vector to hold all test 1 Tasks
	std::vector<std::string> AcctSpecNB_TaskDesc;					//Vector to hold all test 1 Tasks Descriptions
	std::vector<std::string> callCodeName;							//Vector to hold the readable name of the call code
	std::vector<std::string> callCode;								//Vector to hold the actual call codes

	bool getAcctNum = true;											//to prevent account number pull loop
	int QuestMark = 0;

	bool AcctSpecDialog1 = true;
	bool AcctSpecDialog2 = false;
	bool TestPassed = false;
	bool GetCallCode = true;
	bool CallCashTest = true;

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

	int taskCounter = 0;
	char Input_buf[4096];											//storage buffer

	std::string SaveFileName = FileName();
	std::string AcctSpecNBOneTestQ1;								// Instruction Dialog
public:
	AcctSpecNBTest();
	~AcctSpecNBTest();
	int randomElementInt(std::vector<int> a);
	int randomElementStrIndex(std::vector<std::string> a);
	int test_1_GetAccountNumber();
	
	void fillTaskVectors();
	void SetAcctSpecNB_Test_Sql(nanodbc::connection conn);

	std::string task(int questNum);
	std::string taskDesc(int questNum);
	bool CloseProgram = false;

	void T1_AcctSpecNBTest();

	void ACNBQ1();
	void ACNBQ2();
	void ACNBQ3();
	void ACNBQ4();
	void ACNBQ5();
	void ACNBQ6();
	void ACNBQ7();
	void ACNBQ8();
	void ACNBQ9();
	void ACNBQ10();
	void ACNBQ11();
	void ACNBQ12();
	void ACNBQ13();
	void ACNBQ14();
	void ACNBQ15();
	void ACNBQ16();
	void ACNBQ17();
	void ACNBQ18();
	void ACNBQ19();
	void ACNBQ20();
	void ACNBQ21();
	void ACNBQ22();
	void ACNBQ23();
	void ACNBQ24();
	void ACNBQ25();
	void ACNBQ26();
	void ACNBQ27();
	void ACNBQ28();
	void ACNBQ29();
	void ACNBQ30();
	void ACNBQ31();
	void ACNBQ32();
	void ACNBQ33();
	void ACNBQ34();
	void ACNBQ35();
	void ACNBQ36();
	void ACNBQ37();
	void ACNBQ38();
	void ACNBQ39();
	void ACNBQ40();
	void ACNBQ41();
	void ACNBQ42();
	void ACNBQ43();
	void ACNBQ44();
	void ACNBQ45();
	void ACNBQ46();
	void ACNBQ47();
	void ACNBQ48();
	void ACNBQ49();
	void ACNBQ50();
	void ACNBQ51();
	void ACNBQ52();
	void ACNBQ53();
	void ACNBQ54();
	void ACNBQ55();
	void ACNBQ56();
	void ACNBQ57();
	void ACNBQ58();
	void ACNBQ59();
	void ACNBQ60();
	void ACNBQ61();
	void ACNBQ62();
	void ACNBQ63();
	void ACNBQ64();
	void ACNBQ65();
	void ACNBQ66();
	void ACNBQ67();
	void ACNBQ68();
	void ACNBQ69();
	void ACNBQ70();
	void ACNBQ71();
	void ACNBQ72();
	void ACNBQ73();
	void ACNBQ74();
	void ACNBQ75();
	void ACNBQ76();
	void ACNBQ77();
	void ACNBQ78();
	void ACNBQ79();
	void ACNBQ80();
	void ACNBQ81();
	void ACNBQ82();
	void ACNBQ83();
	void ACNBQ84();
	void ACNBQ85();
	void ACNBQ86();
	void ACNBQ87();
	void ACNBQ88();
	void ACNBQ89();

	void ACNBQ90();
	void ACNBQ91();
	void ACNBQ92();
	//void ACNBQ93();
	//void ACNBQ94();
	//void ACNBQ95();

	void QuestDirInsert();
	void SaveToDB();
	void FeedBackWindow();
	void Close();
};
