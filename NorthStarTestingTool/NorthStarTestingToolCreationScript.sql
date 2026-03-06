CREATE DATABASE NorthStarTestingTool
GO

USE NorthStarTestingTool
GO

CREATE TABLE Sec_Group (
	id				int PRIMARY KEY,
	name			NVARCHAR(50),
	description		NVARCHAR(150)
);
GO

INSERT INTO dbo.Sec_Group (id, name, description)
VALUES
(3712, 'Sec_UBCashiers', 'Utility Billing Cashier Security Group'),
(3733, 'Sec_UBAccountSpec_NoBill', 'Utility Billing Base Account Specialists Group'),
(3734, 'Sec_UBSupervisor', 'Utility Billing Supervisors Group'),
(3735, 'Sec_MtrSupervisor', 'Metering Supervisors Group'),
(3736, 'Sec_Metering', 'Metering Users Group'),
(3737, 'Sec_UBQA', 'Utility Billing Quality Assurance Group'),
(3738, 'Sec_Finance', 'Finance Users Group'),
(3739, 'Sec_Administration', 'NorthStar Administers Group'),
(3740, 'Sec_UBAccountSpec_NoBill', 'Utility Billing Full Account Specialists Group');
GO

CREATE TABLE NSuser (
	id						int,
	userlogin				NVARCHAR(MAX) NOT NULL,
	first_name				NVARCHAR(MAX) NOT NULL,
	last_name				NVARCHAR(MAX) NOT NULL,
	user_description		NVARCHAR(MAX) NOT NULL,
	short_login				NVARCHAR(MAX),
	email					NVARCHAR(MAX),
	userGroupID				int,
	userGroupDescription	NVARCHAR(MAX),
	

	CONSTRAINT FK_ChildTable_ParentTable
	FOREIGN KEY (userGroupID)
	REFERENCES dbo.Sec_Group (id)
);
GO

BULK INSERT dbo.NSuser
FROM 'c:\users\Brower_Admin\Documents\NSUSER.csv'
WITH
(
    FORMAT = 'CSV'
    , FIRSTROW = 1
)

CREATE TABLE TestingID (
	id						int NOT NULL PRIMARY KEY,
	name					NVARCHAR(MAX) NOT NULL
);
GO

INSERT INTO dbo.TestingID(id, name)
VALUES
(1, 'Cashiers'),
(2, 'Account Specialists - No Bill'),
(3, 'Account Specialists'),
(4, 'Metering'),
(5, 'Finance');
GO

CREATE TABLE Question_Directives (
	securityGroupID			int NOT NULL,
	testId					int,
	testGroup				NVARCHAR(MAX) NOT NULL,
	task					NVARCHAR(MAX) NOT NULL,
	taskLevelDescription	NVARCHAR(MAX) NOT NULL,
	AdditionalRequirements	NVARCHAR(MAX)

    CONSTRAINT FK_Questions_SecGroup
        FOREIGN KEY (securityGroupID) REFERENCES dbo.Sec_Group(id),
	CONSTRAINT FK_Questions_TestingID
		FOREIGN KEY (testID) REFERENCES dbo.TestingID (id)
);
GO

BULK INSERT dbo.Question_Directives
FROM "C:\Users\Brower_Admin\Desktop\Cashiers.csv"
WITH
(
    FORMAT = 'CSV'
    , FIRSTROW = 1
)
GO

CREATE TABLE Used_Accounts (
	acct_num		int,
	used_by			NVARCHAR(50),
	security_group	NVARCHAR(50),
	used_on			DATE,
	applied_test	NVARCHAR(100)
);

GO

CREATE TABLE Login_srt_lng
(
	shortLogin		CHAR(8),
	longLogin		CHAR(255)
);

GO

BULK INSERT dbo.Login_srt_lng
FROM "C:\Users\Brower_Admin\Desktop\short_loginTOlong_login.csv"
WITH
(
    FORMAT = 'CSV'
    , FIRSTROW = 1
)

GO

CREATE USER [northstar_test] FOR LOGIN [northstar_test];

GO

EXEC sp_addrolemember 'db_datareader', 'northstar_test';