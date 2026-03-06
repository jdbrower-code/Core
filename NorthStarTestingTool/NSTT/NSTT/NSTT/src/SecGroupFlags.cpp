#include "SecGroupFlags.h"
#include "Sql.h"

NorthStarSql SecSql2;

void SecGroupFlags::SecGroupFlagSet()
{

	nanodbc::connection connSecGroup = SecSql2.SqlConnect();
	nanodbc::result result;



	try {
		result = SecSql2.UserSecurityQuery(connSecGroup);

		std::vector<std::string> Groups;

		while (result.next()) {
			std::string label = result.get<std::string>(0);
			//std::cout << "User Group: " << label << std::endl;
			Groups.push_back(label);
		}

		for (int j = 0; j < Groups.size(); j++)
		{
			if (Groups[j].find("Sec_UBCashiers") != std::string::npos)
				m_NSTT_CashiersSecGroup = true;
			//add other groups here as you create them
			else if(Groups[j].find("Sec_UBAccountSpec_NoBill") != std::string::npos)
				m_NSTT_AcctSpecSecNoBillGroup = true;
			else if (Groups[j].find("Sec_UBAccountSpec") != std::string::npos)
				m_NSTT_AcctSpecSecGroup = true;
			else if (Groups[j].find("Sec_UBSupervisor") != std::string::npos || Groups[j].find("Sec_MtrSupevisor") != std::string::npos)
				m_NSTT_SuperSecGroup = true;
			else if (Groups[j].find("Sec_UBQA") != std::string::npos)
				m_NSTT_QualityAdminSecGroup = true;
			else if (Groups[j].find("Sec_Metering") != std::string::npos)
				m_NSTT_MeterSecGroup = true;
			else if (Groups[j].find("Sec_Finance") != std::string::npos)
				m_NSTT_FinanceSecGroup = true;
			else if (Groups[j].find("Sec_Administration") != std::string::npos)
				m_NSTT_AdminSecGroup = true;
		}
		//std::cout << std::boolalpha << m_NSTT_CashiersSecGroup << std::endl;
	}
	catch (const nanodbc::database_error& e) {
		std::cerr << "Database error: " << e.what() << std::endl;
	}


	
}

int SecGroupFlags::getFlag()
{
	SecGroupFlagSet();
	if (m_NSTT_CashiersSecGroup)
		return 0;
	else if (m_NSTT_AcctSpecSecNoBillGroup)
		return 1;
	else if (m_NSTT_AcctSpecSecGroup)
		return 2;
	else if (m_NSTT_QualityAdminSecGroup)
		return 3;
	else if (m_NSTT_SuperSecGroup)
		return 4;
	else if (m_NSTT_MeterSecGroup)
		return 5;
	else if (m_NSTT_FinanceSecGroup)
		return 6;
	else if (m_NSTT_AdminSecGroup)
		return 7;
}

std::string SecGroupFlags::returnSecGroup()
{
	//SecGroupFlagSet();
	if (getFlag() == 0)
		return "Cashier";
	else if (getFlag() == 1 || getFlag() == 2)
		return "Account Specialist";
	else if (getFlag() == 3)
		return "Quality Administrator";
	else if (getFlag() == 4)
		return "Supervisor";
	else if (getFlag() == 5)
		return "Meter Tech";
	else if (getFlag() == 6)
		return "Finance Agent";
	else if (getFlag() == 7)
		return "System Administrator";
		
}

/*Activate these as Security Groups are set up
bool SecGroupFlags::getAcctSpecFlag()
{

}
bool SecGroupFlags::getMeterFlag()
{

}
bool SecGroupFlags::getFinanceFlag()
{

}
bool SecGroupFlags::getSuperFlag()
{

}
bool SecGroupFlags::getQAFlag()
{

}
bool SecGroupFlags::getAdminFlag()
{

}
*/

