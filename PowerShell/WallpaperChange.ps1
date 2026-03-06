Set-PSRepository -Name "PSGallery" -InstallationPolicy Trusted
Install-Script -AcceptLicense -Name FP.SetWallPaper -Confirm:$false
[bool]$modCheck = $false

$MonitorCount = @(Get-CimInstance -Namespace root\wmi -ClassName WmiMonitorBasicDisplayParams).Length

$images = @(
    Get-ChildItem "\\san_marcos\files\Profiles\brower_Jason\DesktopBackgrounds"
)
    


try
{
    Install-Module -Name FP.SetWallpaper
    $modCheck = $true   
}
catch
{
    break
}

if($modCheck)
{
    for($i = 0; $i -lt $MonitorCount; $i++)
    {
        $imageSelection = $images | Get-Random
        Write-Host $imageSelection
        Get-Monitor | Select-Object -Index $i  | Set-WallPaper -path \\san_marcos\files\Profiles\brower_Jason\DesktopBackgrounds\$imageSelection -Force

        
    }
    
    
    
}
else
{
    
    Install-Module -Name FP.SetWallpaper
    Start-Sleep -s 30

    for($i = 0; $i -lt $MonitorCount; $i++)
        {
            $imageSelection = $images | Get-Random
            Write-Host $imageSelection
            Get-Monitor | Select-Object -Index $i  | Set-WallPaper -path \\san_marcos\files\profiles\Brower_Jason\DesktopBackgrounds\$imageSelection -Force

        
        }
}