# Removes the ESP32 PC Monitor startup task and stops the app.
#   irm https://raw.githubusercontent.com/tattooinmtl/Esp32PcMonitorNative.cpp/main/uninstall.ps1 | iex

$TaskName = "ESP32 PC Monitor Native"

Get-Process -Name "Esp32PcMonitorNative" -ErrorAction SilentlyContinue | Stop-Process -Force
Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue

Write-Host "Removed '$TaskName'. The files in C:\esp32_scripts were left in place."
