$key = New-Object Byte[] 32
[Security.Cryptography.RNGCryptoServiceProvider]::Create().GetBytes($Key)
$Key | out-file "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\aes.key"

$Creds = Get-Credential


$Creds.Password | ConvertFrom-SecureString -Key (Get-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\aes.key") | `
Set-Content "\\san_marcos\files\DeptShares\IT\Application Admin\Security\AES Key\Password.txt"