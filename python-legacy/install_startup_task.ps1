$TaskName = "ESP32 PC Monitor"
$ScriptPath = "C:\esp32_scripts\pc_to_esp32_monitor.py"
$StartIn = "C:\esp32_scripts"

if (!(Test-Path $ScriptPath)) {
    Write-Host "ERROR: Python script not found:"
    Write-Host $ScriptPath
    pause
    exit 1
}

$PythonPath = (Get-Command python.exe).Source

if (!$PythonPath) {
    Write-Host "ERROR: python.exe not found in PATH"
    pause
    exit 1
}

Write-Host "Using Python:"
Write-Host $PythonPath

$Action = New-ScheduledTaskAction `
    -Execute $PythonPath `
    -Argument "`"$ScriptPath`"" `
    -WorkingDirectory $StartIn

$Trigger = New-ScheduledTaskTrigger -AtLogOn

$Settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -RestartCount 10 `
    -RestartInterval (New-TimeSpan -Minutes 1)

Register-ScheduledTask `
    -TaskName $TaskName `
    -Action $Action `
    -Trigger $Trigger `
    -Settings $Settings `
    -Description "Starts ESP32 LCD PC CPU/RAM monitor at Windows login" `
    -Force

Write-Host ""
Write-Host "DONE. Startup task installed:"
Write-Host $TaskName
Write-Host ""
Write-Host "To test it now, run:"
Write-Host "Start-ScheduledTask -TaskName `"$TaskName`""
pause