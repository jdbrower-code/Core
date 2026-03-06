<# 
$SqlServer = "NS-Test2016\NorthStar"
$SqlDB = "Northstar_test"
$uid = "Brower_Admin"
$pwd = "Family!Is!Life!"
$SqlConnection = New-Object System.Data.SqlClient.SqlConnection
$SqlConnection.ConnectionString = "Server = $SqlServer; Database = SqlDB; Integrated Security = True;"
$SqlCmd = New-Object System.Data.SqlClient.SqlCommand


$SqlQuery = "Select `
  call_number, `
  calltype, `
  callcode, `
  note, `
  callername, `
  createdon, `
  createdby `

  FROM harris_test.CSRCALLD"

  $SqlCmd.CommandText = $SqlQuery
  $SqlCmd.Connection = $SqlConnection
  $SqlAdapter = New-Object System.Data.SqlClient.SqlDataAdapter
  $SqlAdapter.SelectCommand = $SqlCmd
  #$DataSet = New-Object System.Data.DataSet
  #$SqlAdapter.Fill($DataSet)

  
  Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query $SqlQuery -TrustServerCertificate | ConvertTo-Csv | Out-File E:\PowerShell\Test.csv



$Files = @()

$PhoneNumber     = @() 
$LastDisposition = @()
$LastResultDate  = @()
$Attempts        = @()
$BilledMinutes   = @()
$BatchNumber     = @()
$AccountNo       = @()
$Phone           = @()
$ServiceADDR     = @()
$EmailAddress    = @()
$DueDate         = @()
$LateChrgJrnl    = @()
$BalCurrent      = @()
$BalLateChg      = @()
$BalOverdue      = @()
$Bal_Total       = @()
$Class           = @()
$CustNo          = @()
$Name            = @()
$PendTotal       = @()
#>




$columnArrays = @{}

  Get-ChildItem "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE" | `
    ForEach-Object {
        if($_.Extension -eq ".csv") {
            $Files = Import-Csv $_.FullName
            $CSV_File = $_.BaseName
            $Files |
            
        }
        else {
            Write-host "False"
        }
    }

$Create_Year = (Get-Date).Year.ToString()
$Create_Month = (Get-Date).Month
$Create_Day = (Get-Date).Day
$Create_Hour = (Get-Date).Hour
$Create_Min = (Get-Date).Minute
$Create_Sec = (Get-Date).Second


Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query `
                                        "SELECT `
                                         max(call_number) as call_number FROM harris_test.CSRCALLD" -TrustServerCertificate