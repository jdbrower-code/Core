#include "Sql.h"
#include "imgui.h"

nanodbc::connection NorthStarSql::SqlConnect()
{

	
	//Connect to Database
	nanodbc::connection conn(NANODBC_TEXT(
		"Driver={ODBC Driver 17 for SQL Server};"
		"Server=NS-Test2016\\NorthStar;"
		"DATABASE=Northstar_test;"
		"Trusted_Connection=Yes;"));

	//Test Connection
	if (conn.connected())
		return conn;
	else
		std::cerr << "Failed to Connect" << std::endl;
}

nanodbc::connection NorthStarSql::NSTT_SqlConnect()
{
	//Connect to Database
	nanodbc::connection nstt_conn(NANODBC_TEXT(
		"Driver={ODBC Driver 17 for SQL Server};"
		"Server=TestSQL2022\\sandbox;"
		"DATABASE=NorthStarTestingTool;"
		"Trusted_Connection=Yes;"));

	//Test Connection
	if (nstt_conn.connected())
		return nstt_conn;
	else
		std::cerr << "Failed to Connect" << std::endl;
}

nanodbc::connection NorthStarSql::NSTT_NsLive_SqlConnect()
{
	//Connect to Database
	nanodbc::connection nstt_NsLive_conn(NANODBC_TEXT(
		"Driver={ODBC Driver 17 for SQL Server};"
		"Server=TestSQL2022\\sandbox;"
		"DATABASE=Northstar_live;"
		"Trusted_Connection=Yes;"));

	//Test Connection
	if (nstt_NsLive_conn.connected())
	{
		//std::cout << "Connection Established" << std::endl;
		return nstt_NsLive_conn;
	}
	else
		std::cerr << "Failed to Connect" << std::endl;
}

nanodbc::result NorthStarSql::AcctAndOccupant(nanodbc::connection conn)
{
	//Query
	nanodbc::result Query = nanodbc::execute(conn, NANODBC_TEXT("SELECT account_no, "
		"occupant_code FROM harris_test.PU_ACCOUNT ORDER BY account_no"));

	return Query;
}





std::string NorthStarSql::GetShortLogin(nanodbc::connection conn)
{
	std::string username = GetUserName();
	std::vector<std::string> sLogin;
	

	nanodbc::statement stmt(conn);

	prepare(stmt, NANODBC_TEXT(R"(
    SELECT lsl.shortLogin 
    FROM NorthStarTestingTool.dbo.Login_srt_lng as lsl
    WHERE lsl.longLogin = ?
)"));

	stmt.bind(0, username.c_str());

	nanodbc::result shrLoginQuery = execute(stmt);
	
	for (int i = 1; shrLoginQuery.next(); ++i)
	{
		 std::string temp = shrLoginQuery.get<std::string>(0);
		 sLogin.push_back(temp);
		
	}


	
	return sLogin[0];

}

	



nanodbc::result NorthStarSql::UserSecurityQuery(nanodbc::connection conn)
{

	std::string username = GetUserName();
	
	nanodbc::statement stmt(conn);

	nanodbc::prepare(stmt,
		"SELECT UG.displaylabel "
		"FROM Northstar_test.harris_test.NSUSER AS NSU "
		"JOIN Northstar_test.harris_test.USERTOUSERGROUP AS UTUG ON NSU.id = UTUG.nsuserid "
		"JOIN Northstar_test.harris_test.USERGROUP AS UG ON UG.id = UTUG.usergroupid "
		"WHERE NSU.userlogin = ?");

		stmt.bind(0, username.c_str());

		auto result = nanodbc::execute(stmt);
		
		return result;
}

std::string NorthStarSql::GetUserName()
{
	//Get logged in users email
	char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;

	GetUserNameA(username, &username_len);

	return username;
}


