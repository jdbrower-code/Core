#######################################################################################
# IVR_Inbound.ps1 
# Pulls in Data from a CSV file and inputs it into the CSRCALLD table in the NorthStar
# Database. 
# 
#
#
# Author(s): Jason Brower
#
# Dependencies: None
#
# Change log:
# <10/28/2024> - Initial creation of Script Tested in NorthStar_Test
# <10/29/2024> - Created Error trap and updated to live database and tables
#########################################################################################


$ErrorActionPreference = "Continue"

Import-Module TUN.CredentialManager, SqlServer;

[string]$credTarget = 'AlertWorks Script'

$cred = $(Get-StoredCredential -Target $credTarget -AsCredentialObject -IncludeSecurePassword)


#Varibles for Server and Database Target
$SqlServer = "NS-ProdDB2016\Northstar"  
$SqlDB     = "Northstar_live"


#Date Script is run, Used to make
$RunTime = (Get-Date -Format yyyy-MM-dd)

#Test Connection
$ConnectionTest = $null

#SQL Query to be used to test Connection to Server
$SqlQueryForTest =  "Select `
                     call_number, createdon `
                     FROM harris_live.CSRCALLD ` 
                     WHERE call_number < 1000" 
 
             
#Server Conncetion Variable
$ConnectionTest = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB  `
 -Username $cred.UserName -Password $cred.Password `
 -query $SqlQueryForTest -TrustServerCertificate


 #Variable to Store Call Log Tag that will be input into 'callcode' field   
 $CallCode = "CALLOG"

 #Path for error log files
 $LogFolder = "c:\Scripts\AlertWorks\FileInput\LIVE\Errors"

  #path for dup call number
 $dupcall = "c:\Scripts\AlertWorks\FileInput\LIVE\Errors\dupcall.txt"

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
    Get-ChildItem "c:\Scripts\AlertWorks\FileInput\LIVE" | `
        ForEach-Object {
            $TargetFile = $_.FullName
            if($_.Extension -eq ".csv") {
                $Files = Import-Csv $_.FullName
                #Check for "'" in the Name Column and correct
                $Files | ForEach-Object {$_.NAME = $_.NAME.Replace("'", "''") } 
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
                        $CallNumber = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                        -Username $cred.UserName -Password $cred.Password  -Query `
                            "SELECT `
                             max(call_number) as call_number `
                             FROM harris_live.CSRCALLD `
                             WHERE account_no = $AccountNumber `
                             and occupant_code = $OccupantCode `
                             and callcode = '$CallCode' `
                             and calltype = '$CallType' `
                             and createdon = '$DateAndTime'" -TrustServerCertificate
                        
                        #if Call already exists break from working loop and move file to Archive Folder
                        if($CallNumber.call_number -isnot [DBNULL]) {
                            if(Test-Path $dupcall)
                            {
                                Write-Output "Existing Call Number!!" | Out-File -Append $dupcall
                                Write-Output "CSV File: " $CSV_File | Out-File -Append $dupcall
                                Write-Output "call Number: " $CallNumber.call_number | Out-File -Append $dupcall 
                                Write-Output "Account Number: "$AccountNumber | Out-File -Append $dupcall
                                Write-Output "CSV Line: " $i | Out-File -Append $dupcall
                            }
                            else
                            {
                                New-Item $dupcall
                                Write-Output "Existing Call Number!!" | Out-File -Append $dupcall
                                Write-Output "CSV File: " $CSV_File | Out-File -Append $dupcall
                                Write-Output "call Number: " $CallNumber.call_number | Out-File -Append $dupcall 
                                Write-Output "Account Number: "$AccountNumber | Out-File -Append $dupcall
                                Write-Output "CSV Line: " $i | Out-File -Append $dupcall
                            }
                        }

                        #Update new call number
                        $CallNumberUpDate = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                        -Username $cred.UserName -Password $cred.Password `
                        -Query `
                            "SELECT `
                             max(call_number) as call_number `
                             FROM harris_live.CSRCALLD `
                             WHERE account_no = $AccountNumber `
                             and occupant_code = $OccupantCode `
                             and callcode = '$CallCode' `
                             and calltype = '$CallType' `
                             " -TrustServerCertificate
                        

                        #if Call does not exists pull data for accouns in Cvs file
                       $GetDetails =  Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                       -Username $cred.UserName -Password $cred.Password `
                       -Query `
                        "SELECT account_no, occupant_code, debtor_no, name `
                         FROM harris_live.PU_ACCOUNT `
                         WHERE account_no = $AccountNumber `
                         and occupant_code = $OccupantCode" `
                         -TrustServerCertificate
                        
                        $DebtorNumber = $GetDetails.debtor_no

                        $Name = $Files[$i].name
                        
                        #insert Data into harris_live.CSRCALLD
                        Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                        -Username $cred.UserName -Password $cred.Password `
                        -Query `
                        "INSERT INTO harris_live.CSRCALLD `
                        (call_number, calltype, callcode, note, callername, callstatus, `
                        createdon, createdby, scheduledon, requiredon, account_no, `
                        occupant_code, debtor_no, printed, postbillto) `
                        VALUES `
                        (0,'$CallType', '$CallCode', 'IVR IMPORT', '$Name', 'L', `
                        '$DateAndTime', 'IVR', '$DateAndTime', '$DateAndTime', $AccountNumber, `
                        $OccupantCode, '$DebtorNumber', 0, 1)" -TrustServerCertificate


                        #Get Callnumber created previously to append notes to the CSRSCHDD table

                         $CallNumberForNotes = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                         -Username $cred.UserName -Password $cred.Password `
                         -Query `
                        "SELECT max(call_number) FROM harris_live.CSRCALLD WHERE account_no = $AccountNumber" -TrustServerCertificate

                        #pull int from table
                        $CallNumberForNotesInt = $CallNumberForNotes.Column1

                        #create and array of sequence numbers (one for each note)
                        $SeqNumberForNotes = @()

                        for($j = 1; $j -lt 19; $j++)
                        {
                            $SeqNumberForNotes += $j;

                        }


                        #Create and array for the notes 
                        $NotesForCSRSCHDD = @()

                        #Add noted to notes array
                        $NotesForCSRSCHDD += 'Last Disposition: ' + $Files[$i].'Last Disposition'
                        $NotesForCSRSCHDD += "Last Result Date: " + $Files[$i].'Last Result Date'
                        $NotesForCSRSCHDD += "Attempts: " + $Files[$i].Attempts
                        $NotesForCSRSCHDD += "Billed Minutes: " + $Files[$i].'Billed Minutes'
                        $NotesForCSRSCHDD += "BATCHNUMBER: " + $Files[$i].BATCHNUMBER
                        $NotesForCSRSCHDD += "CLASS: " + $Files[$i].CLASS
                        $NotesForCSRSCHDD += "CUST_NO" + $Files[$i].CUST_NO
                        $NotesForCSRSCHDD += "PHONE: " + $Files[$i].PHONE
                        $NotesForCSRSCHDD += "SERVICE_ADDR: "  + $Files[$i].SERVICE_ADDR
                        $NotesForCSRSCHDD += "NAME: " + $Files[$i].NAME
                        $NotesForCSRSCHDD += "EMAIL_ADDRESS: " + $Files[$i].EMAIL_ADDRESS
                        $NotesForCSRSCHDD += "DUE_DATE: " + $Files[$i].DUE_DATE
                        $NotesForCSRSCHDD += "LATE_CHG_JRNL: " + $Files[$i].LATE_CHG_JRNL
                        $NotesForCSRSCHDD += "BAL_CURRENT: " + $Files[$i].BAL_CURRENT
                        $NotesForCSRSCHDD += "BAL_OVRDUE: " + $Files[$i].BAL_OVRDUE
                        $NotesForCSRSCHDD += "BAL_LATE_CHG: " + $Files[$i].BAL_LATECHG
                        $NotesForCSRSCHDD += "BAL_TOTAL: " + $Files[$i].BAL_TOTAL
                        $NotesForCSRSCHDD += "PEND_TOTAL: " + $Files[$i].PEND_TOTAL

                        #Notes loop that creastes a note with a qequence number for all notes in notes array

                        for($l = 1; $l -lt 19; $l++)
                        {
                            
                            #Create and tie sequence numnber to call number of current account
                            Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                            -Username $cred.UserName -Password $cred.Password `
                            -Query `
                            "SET ANSI_WARNINGS OFF; `
                            INSERT INTO harris_live.CSRSCHDD `
                            (call_number, sequence_no) `
                            VALUES `
                            ($CallNumberForNotesInt, $l) `
                            SET ANSI_WARNINGS ON;" -TrustServerCertificate

                            #set note to single variable to prevent string error
                            $tempNote = $NotesForCSRSCHDD[$l - 1]
                            
                            #update note field of current sequence number with current note
                            Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB `
                            -Username $cred.UserName -Password $cred.Password `
                            -Query `
                            "SET ANSI_WARNINGS OFF; `
                            UPDATE harris_live.CSRSCHDD `
                            SET notes = '$tempNote' `
                            WHERE call_number = $CallNumberForNotesInt `
                            and sequence_no = $l `
                            SET ANSI_WARNINGS ON;" -TrustServerCertificate
                            
                        }
                        
                        

                    }
                }
                Catch
                {
                    #Create Log file of any and all errors encountered in the process
                    $ErrorFilePath = "$LogFolder\$CSV_File.log"

                    if(Test-Path $ErrorFilePath)
                    {
                        $_ | Out-File -Append `
                        "$LogFolder\$CSV_File.log" `
                        -NoClobber -ErrorAction SilentlyContinue
                    }
                    else
                    {
                        New-Item "$LogFolder\$CSV_File.log"
                        $_ | Out-File -Append "$LogFolder\$CSV_File.log" `
                        -ErrorAction SilentlyContinue
                    }
                    
                }


                #Move Completed File to teh Archive Folder
                Move-item $TargetFile -Destination "c:\Scripts\AlertWorks\FileInput\LIVE\Archive"
                

                #Check age of error log files and delete if older than a month
                Get-ChildItem $LogFolder |
                ForEach-Object {
                    $CreationTime = $_.CreationTime
                    $LogFileRM = $_.FullName


                   If($CreationTime -lt (Get-Date).AddDays(-30))
                   {
                       $LogFileRM | Remove-Item
                   }



                   #Check age of Archived CSV files and delete if older than a month
                   Get-ChildItem "c:\Scripts\AlertWorks\FileInput\LIVE\Archive" | `
                   ForEach-Object {
                        $CSV_CreationTime = $_.CreationTime
                        $CSV_FileRM = $_.FullName
                   }

                   if($CSV_CreationTime -lt (Get-Date).AddDays(-30))
                   {
                        $CSV_FileRM | Remove-Item
                   }

                }

              

            }
        }
}
else
{
    #Send Email to App Admin if script fails to connect to the database
    $SMTPServer = "irelay"
    $Date = Get-Date -Format "dddd MM/dd"
    $Subject = Write-Output "IVR_INBOUND Failure "
    $to = "jbrower@sanmarcostx.gov" 
    $from = new-object System.Net.Mail.MailAddress "servicedesk@sanmarcostx.gov", "IT - Service Desk"
    $Text = "The Connection to The Sql Server failed! " 

    Send-MailMessage -SmtpServer $SMTPServer -From $from -To $to -Subject $Subject -Body $Text
}