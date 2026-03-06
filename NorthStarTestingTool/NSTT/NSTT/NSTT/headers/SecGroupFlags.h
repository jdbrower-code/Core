#pragma once
//#include "Core.h"

#include <iostream>
#include <vector>



class SecGroupFlags
{
private:
	bool m_NSTT_CashiersSecGroup = false;
	bool m_NSTT_AcctSpecSecNoBillGroup = false;
	bool m_NSTT_AcctSpecSecGroup = false;
	bool m_NSTT_MeterSecGroup = false;
	bool m_NSTT_FinanceSecGroup = false;
	bool m_NSTT_SuperSecGroup = false;
	bool m_NSTT_QualityAdminSecGroup = false;
	bool m_NSTT_AdminSecGroup = false;
public:
	void SecGroupFlagSet();
	int getFlag();
	/*Activate these as Security Groups are set up
	bool getAcctSpecFlag();
	bool getAcctSpecFlag_NoBill();
	bool getMeterFlag();
	bool getFinanceFlag();
	bool getSuperFlag();
	bool getQAFlag();
	bool getAdminFlag();
	*/
	std::string returnSecGroup();
};

