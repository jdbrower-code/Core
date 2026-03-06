#######################################################################################
# IVR_Inbound.ps1 
# Pulls in Data from a CSV file and inputs it into the CSRCALLD table in the NorthStar
# Database. 
# 
#
# Changes to make to con
#     Change SqlServer to NS-ProdDB2016\Northstar
#     Change SqlDB to Northstar_live 
#     Change Every harris_test to harris_live
# 
#
# Author(s): Jason Brower
#
# Dependencies: None
#
# Change log:
# <10/28/2024> - Initial creation of Script Tested in NorthStar_Test
#########################################################################################

$ErrorActionPreference = "Continue"

#Varibles for Server and Database Target
$SqlServer = "NS-Test2016\NorthStar"  
$SqlDB     = "Northstar_test"

#SQL Credentials
$SqlUser   = "San_Marcos\Brower_Admin"
$SqlPW     = Get-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\Password.txt" |
             ConvertTo-SecureString -Key (Get-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\aes.key")

#Date Script is run, Used to make
$RunTime = (Get-Date -Format yyyy-MM-dd)

#Test Connection
$ConnectionTest = $null

#SQL Query to be used to test Connection to Server
$SqlQueryForTest =  "Select `
                     call_number, createdon `
                     FROM harris_test.CSRCALLD ` 
                     WHERE call_number < 1000" 
 
             
#Server Conncetion Variable
$ConnectionTest = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB  `
 -query $SqlQueryForTest -TrustServerCertificate


 #Variable to Store Call Log Tag that will be input into 'callcode' field   
 $CallCode = "CALLOG"

 #Path for error log files
 $LogFolder = "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Errors"

 #Run Connection Test
 if($ConnectionTest.count -gt 0) {

    #If Connection Test passes...
    #Create Time Variables...
    #Primarily used to check if a call already exist for any of the accounts
    $Year  = (Get-Date -Format yyyy/MM/dd).Split('/')[0]
    $Month = (Get-Date -Format yyyy/MM/dd).Split('/')[1]
    $Day   = (Get-Date -Format yyyy/MM/dd).Split('/')[2]
    #Concat Time Variables and set hour to exacatly midnight
    $DateAndTime = $Year + '-' + $Month + '-' + $Day + `
                    ' ' + '00' + ':' + '00' + ':' + '00'
    
    
    $Count = 0
    #open csv file and store data in a variable
    #Work through each file in the folder one at a time
    Get-ChildItem "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE" | `
        ForEach-Object {
            $TargetFile = $_.FullName
            if($_.Extension -eq ".csv") {
                $Files = Import-Csv $_.FullName
                $CSV_File = $_.BaseName
                $CallType = ($CSV_File).Split("_")[0]

                try
                {
                    #Working Loop
                    for($i = 0; $i -lt $Files.Count; $i++){
                        $Temp = $Files[$i].ACCOUNT_NO
                        $AccountNumber = $Temp.Split('-')[0]
                        $OccupantCode  = $Temp.Split('-')[1]

                        #confirm no call exists for theses account
                        $CallNumber = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query `
                            "SELECT `
                             max(call_number) as call_number `
                             FROM harris_test.CSRCALLD `
                             WHERE account_no = $AccountNumber `
                             and occupant_code = $OccupantCode `
                             and callcode = '$CallCode' `
                             and calltype = '$CallType' `
                             and createdon = '$DateAndTime'" -TrustServerCertificate
                        
                        #if Call already exists break from working loop and move file to Archive Folder
                        if($CallNumber.call_number -isnot [DBNULL]) {
                           Write-Host "Yes"
                           break
                        }

                        #Update new call number
                        $CallNumberUpDate = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query `
                            "SELECT `
                             max(call_number) as call_number `
                             FROM harris_test.CSRCALLD `
                             WHERE account_no = $AccountNumber `
                             and occupant_code = $OccupantCode `
                             and callcode = '$CallCode' `
                             and calltype = '$CallType' `
                             " -TrustServerCertificate
                        

                        #if Call does not exists pull data for accouns in Cvs file
                       $GetDetails =  Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query `
                        "SELECT account_no, occupant_code, debtor_no, name `
                         FROM harris_test.PU_ACCOUNT `
                         WHERE account_no = $AccountNumber `
                         and occupant_code = $OccupantCode" `
                         -TrustServerCertificate
                        
                        $DebtorNumber = $GetDetails.debtor_no

                        $Name = $Files[$i].name
                        
                        #insert Data into harris_test.CSRCALLD
                        Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB -Query `
                        "INSERT INTO harris_test.CSRCALLD `
                        (call_number, calltype, callcode, note, callername, callstatus, `
                        createdon, createdby, scheduledon, requiredon, account_no, `
                        occupant_code, debtor_no, printed, postbillto) `
                        VALUES `
                        (0,'$CallType', '$CallCode', 'IVR IMPORT', '$Name', 'L', `
                        '$DateAndTime', 'IVR', '$DateAndTime', '$DateAndTime', $AccountNumber, `
                        $OccupantCode, '$DebtorNumber', 0, 1)" -TrustServerCertificate

                        
                        

                    }
                }
                Catch
                {
                    
                    $ErrorFilePath = "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Errors\$CSV_File.log"

                    if(Test-Path $ErrorFilePath)
                    {
                        $_ | Out-File -Append `
                        "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Errors\$CSV_File.log" `
                        -NoClobber -ErrorAction SilentlyContinue
                    }
                    else
                    {
                        New-Item "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Errors\$CSV_File.log"
                        $_ | Out-File -Append "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Errors\$CSV_File.log" `
                        -ErrorAction SilentlyContinue
                    }
                    
                }


                #Move Completed File to teh Archive Folder
                Move-item $TargetFile -Destination "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Archive\"
 
                Get-ChildItem $LogFolder |
                ForEach-Object {
                    $CreationTime = $_.CreationTime
                    $LogFileRM = $_.FullName


                   If($CreationTime -lt (Get-Date).AddDays(-30))
                   {
                       $LogFileRM | Remove-Item
                   }

                }

              

            }
        }
}