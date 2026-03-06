$SqlServer = "NS-ProdDB2016\Northstar"  
$SqlDB     = "Northstar_live"

$SqlQueryForTest =  "Select `
                     call_number, createdon `
                     FROM harris_live.CSRCALLD ` 
                     WHERE call_number < 1000" 


$cred = Get-Credential

#Server Conncetion Variable
$ConnectionTest = Invoke-Sqlcmd -ServerInstance $SqlServer -Database $SqlDB  `
 -Username $cred.UserName -Password  $cred.Password`
 -query $SqlQueryForTest -TrustServerCertificate


 if($ConnectionTest.count -gt 0)
 {
    return $true
 }
 else
 {
    return $false
 }