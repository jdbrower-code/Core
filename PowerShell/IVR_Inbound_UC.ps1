<#
--------------
IVR_inbound.vbs
--------------
This script will look in a folder for a CSV file and create a Call in Northstar if the 
contents satisfies the format.
CSV file must contain a first row of labels for the columns being imported.
There MUST be a column called account and a column called occupant (not case sensitive).
any other column label will be imported as schedule notes in the call.
The note lines are 60 characters long. the combined length of the label and the content for
any column must not exceed 58 characters (we are putting 2 chars between). 
The note will be created like this... ( ": " is inserted between the Label and Content)

LABEL: Content
Example:
File contains data that looks like this...

Account,Occupant,My Label,Time,Action,Response
12345,0,IVR callout,20091225 13:01:25,Disconnect Warning,No Answer

this file would add a Call to account 12345-0 with these lines in the schedule notes...

My Label: IVR callout
Time: 20091225 13:01:25
Action: Disconnect Warning
Response: No Answer

after the file is processed successfully it will move it into a subfolder called "Archive".
if the file does not process successfully it will move it to a subfolder called "Errors".

A log file will be created each time this process is run that will describe the steps taken 
and report any Errors.

An IVR call will only be added once per account per day.
If multiple records exist or are processed from multiple import files for an account 
on the same day all notes will be appended to a single IVR call record for that day.

resume on errors so they can be trapped in the log file. 
on error resume next

-----------------------------------------------------------------------------------
#The sourceFolder is where the inbound IVR file is to be placed for processing.#>

$sourceFolder = "C:\Inetpub\wwwroot\AlertWorks\LIVE"

<#
 Enter the call code and call type to create in northstar to record the details from the inbound IVR file.
 Assumption: this is not creating an order, it will create a Logged call. 
 This callcode / calltype must be setup in NorthStar.
 -----------------------------------------------------------------------------------#>

$callCode = "CALLOG"
$callType = ""

$LogFile = "ivr_inbound.log"

<# Enter the ODBC Data Source#>
$Server = "NS-Test2016.ci.san-marcos.tx.us"
$DataBase = "Northstar"

#User
$UID = "harris_live"

#Get Previously Set up AES Key
$AESKey = Get-Content "C:\inetpub\wwwroot\AlertWorks\Creds\AESKey.txt"

#Pull Secured Password and apply encryption Key
$pwdTxt = Get-Content "C:\inetpub\wwwroot\AlertWorks\Creds\Pwd.txt"
$password = $pwdTxt | ConvertTo-SecureString -Key $AESKey

#Set Cred file for log in
$cred = New-Object System.Management.Automation.PSCredential -ArgumentList $UID, $password

#Connect to Database
$SqlConnection = New-Object System.Data.SqlClient.SqlConnection
$SqlConnection.ConnectionString = "Server = $Server; Database = $DataBase; User ID = $cred.UID; Password = $cred.password"
$SqlCmd = New-Object System.Data.SqlClient.SqlCommand
$SqlCmd.Connection = $SqlConnection
$SqlAdapter = New-Object System.Data.SqlClient.SqlDataAdapter
$SqlAdapter.SelectCommand = $SqlCmd
$DataSet = New-Object System.Data.DataSet
$SqlAdapter.Fill($DataSet)
$SqlConnection.Close()