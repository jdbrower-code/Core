<#connect to Northstar_Test#>



$SqlServer = "NS-Test2016\NorthStar"
$SqlDB     = "Northstar_test"
$SqlUser   = "San_Marcos\Brower_Admin"
$SqlPW     = Get-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\Password.txt" |
             ConvertTo-SecureString -Key (Get-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\aes.key")

#Test Connection
$ConnectionTest = $null

$SqlQueryForTest =  "Select `
                     call_number, createdon `
                     FROM harris_test.CSRCALLD `
                     WHERE call_number < 1000" 
 
             

$ConnectionTest = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB  `
 -query $SqlQueryForTest -TrustServerCertificate


 $CallCode = "CALLOG"

 if($ConnectionTest.count -gt 0) {

    #Create Time Variables
    #Primarily used to check if a call already exist for any of the accounts
    $Year  = (Get-Date -Format yyyy/MM/dd).Split('/')[0]
    $Month = (Get-Date -Format yyyy/MM/dd).Split('/')[1]
    $Day   = (Get-Date -Format yyyy/MM/dd).Split('/')[2]
    #Concat Time Variables and set hour to exacatly midnight
    $DateAndTime = $Year + '-' + $Month + '-' + $Day + `
                    ' ' + '00' + ':' + '00' + ':' + '00'
    
    
    
    #open csv file and store data in a variable
    Get-ChildItem "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE" | `
        ForEach-Object {
            $TargetFile = $_.FullName
            if($_.Extension -eq ".csv") {
                $Files = Import-Csv $_.FullName
                $CSV_File = $_.BaseName
                $CallType = ($CSV_File).Split("_")[0]

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
                    
                    #if Call already exists break from loop
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
                    #$CallNumberUpDate.call_number
                    try
                    {
                        $CallNumber = $CallNumberUpDate.call_number + 1
                    }
                    Catch
                    {
                        #Write-Output "Something Went Wrong!" "Call Num =" `
                         #$CallNumber "Name =" $Name "Account =" $AccountNumber
                        
                        
                        
                    }
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



                Move-item $TargetFile -Destination "\\san_marcos\files\DeptShares\Finance\Utility Billing\AlertWorks\LIVE\Archive\"
    
            }
        }
}