$ErrorActionPreference = "SilentlyContinue"

$xlsxLocation = "\\san_marcos\files\deptshares\Finance\Utility Billing\Shared Utility Billing\_QAQC Dev\CoSM FTP Historical Reports"
                                                                                                       
$Files = Get-ChildItem $xlsxLocation

$ExcelFiles = @()

foreach($i in $Files)
{
    $ExcelFiles +=  Get-ChildItem $i.FullName -filter "*.xlsx"
}

#$ExcelFiles

$excelObj = New-Object -ComObject Excel.Application

$excelObj.Visible = $False

$excelObj.DisplayAlerts = $False

$count = 0

$SqlConnectedFiles = @()

foreach($file in $ExcelFiles)
{
    $workbook = $excelObj.Workbooks.Open($file.FullName ) 

    foreach($worksheet in $workbook.Worksheets)
    {
        foreach($listobject in $worksheet.ListObjects)
        {
            $commandText = $listobject.QueryTable.CommandText 
            if(-not([string]::IsNullOrWhiteSpace($commandText)))
            {
                $count++
                Write-Host $file.Directory\$file 
                $SqlConnectedFiles += $file.Directory
                
                
                
            }
        }
    }

    #$workbook.Worksheets
}

$workbook.Close($False) # closed do not save
$excelObj.DisplayAlerts = $True
$excelObj.Quit()
[System.Runtime.Interopservices.Marshal]::ReleaseComObject($excelObj) | Out-Null Remove-Variable excelObj | Out-Null

$count

