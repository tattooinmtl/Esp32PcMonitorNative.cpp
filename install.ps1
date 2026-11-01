# Installs ESP32 PC Monitor Native and starts it at every Windows logon.
# One-line install (PowerShell):
#   irm https://raw.githubusercontent.com/tattooinmtl/Esp32PcMonitorNative.cpp/main/install.ps1 | iex

$ErrorActionPreference = "Stop"

$Repo       = "tattooinmtl/Esp32PcMonitorNative.cpp"
$InstallDir = "C:\esp32_scripts"
$ExeName    = "Esp32PcMonitorNative.exe"
$TaskName   = "ESP32 PC Monitor Native"
$ExePath    = Join-Path $InstallDir $ExeName
$DownloadUrl = "https://github.com/$Repo/releases/latest/download/$ExeName"

Write-Host "Installing ESP32 PC Monitor to $InstallDir"
New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null

# Stop a running copy so the exe can be replaced
Get-Process -Name ([IO.Path]::GetFileNameWithoutExtension($ExeName)) -ErrorAction SilentlyContinue |
    Stop-Process -Force
Start-Sleep -Milliseconds 500

Write-Host "Downloading $DownloadUrl"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Invoke-WebRequest -Uri $DownloadUrl -OutFile $ExePath -UseBasicParsing
Unblock-File -Path $ExePath

# Logon task for the current user, no time limit, restart if it crashes
$User      = "$env:USERDOMAIN\$env:USERNAME"
$Action    = New-ScheduledTaskAction -Execute $ExePath -WorkingDirectory $InstallDir
$Trigger   = New-ScheduledTaskTrigger -AtLogOn -User $User
$Principal = New-ScheduledTaskPrincipal -UserId $User -LogonType Interactive -RunLevel Limited
$Settings  = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -ExecutionTimeLimit ([TimeSpan]::Zero) `
    -RestartCount 10 `
    -RestartInterval (New-TimeSpan -Minutes 1)

Register-ScheduledTask `
    -TaskName $TaskName `
    -Action $Action `
    -Trigger $Trigger `
    -Principal $Principal `
    -Settings $Settings `
    -Description "Sends PC CPU/RAM/disk usage to the ESP32 LCD over USB serial" `
    -Force | Out-Null

Start-ScheduledTask -TaskName $TaskName

Write-Host ""
Write-Host "DONE. '$TaskName' is installed and running."
Write-Host "It will start automatically every time you log in to Windows."
Write-Host "Flash the ESP32 with firmware\NativeCVersionESP32_display if it is a new board."
