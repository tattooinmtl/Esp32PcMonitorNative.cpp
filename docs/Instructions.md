# Esp32PcMonitorNative - Installation & Usage Guide

**Version:** Native Win32 Background Telemetry Service
**Platform:** Windows 11 x64
**Language:** C++ (Win32 API)
**IDE:** Visual Studio 2026 Community/Professional
**Target:** ESP32-C6 + Waveshare LCD 1.3

---

# Overview

Esp32PcMonitorNative is a lightweight native Windows application that continuously gathers telemetry from the PC and streams it to an ESP32 over USB Serial.

Unlike the previous .NET Worker Service, this version:

* Runs without a console window
* Does not require the .NET runtime
* Uses direct Win32 API calls
* Automatically reconnects if the ESP32 is unplugged
* Can start automatically with Windows
* Designed to consume minimal CPU and memory

---

# Current Telemetry

The application currently sends:

```
CPU Usage
RAM Usage %
RAM Used (GB)
RAM Total (GB)
Disk C: Usage %
PC Uptime
```

Serial packet format:

```text
CPU=12.4,RAM=41.8,USED=12.1,TOTAL=28.9,DISKC=71.0,UP=0.0,DOWN=0.0,UPTIME=12345
```

This protocol is parsed directly by the ESP32 dashboard.

---

# Visual Studio Project

Project Type

```
C++ Desktop Application
```

Configuration

```
Release
x64
```

Subsystem

```
Windows (/SUBSYSTEM:WINDOWS)
```

This prevents a console window from appearing.

---

# Default Serial Settings

```
Baud Rate : 115200
Data Bits : 8
Parity    : None
Stop Bits : 1
```

Preferred COM Port

```
COM4
```

If COM4 is unavailable the application scans additional COM ports.

---

# Building

Build configuration

```
Release
```

Platform

```
x64
```

Result

```
Esp32PcMonitorNative.exe
```

Example output

```
C:\esp32_scripts\Esp32PcMonitorNative.exe
```

---

# Manual Launch

```
C:\esp32_scripts\Esp32PcMonitorNative.exe
```

No console window will appear.

The application runs silently in the background.

---

# Automatically Start With Windows

Create startup task

```cmd
schtasks /Create /TN "ESP32 PC Monitor Native" /TR "C:\esp32_scripts\Esp32PcMonitorNative.exe" /SC ONLOGON /RL LIMITED /F
```

---

# Start Immediately

```cmd
schtasks /Run /TN "ESP32 PC Monitor Native"
```

---

# Stop Process

```cmd
taskkill /IM Esp32PcMonitorNative.exe /F
```

---

# Remove Startup Task

```cmd
schtasks /Delete /TN "ESP32 PC Monitor Native" /F
```

---

# Check Running Process

Command Prompt

```cmd
tasklist | findstr Esp32PcMonitorNative
```

PowerShell

```powershell
Get-Process Esp32PcMonitorNative
```

---

# Verify Startup Task

```cmd
schtasks /Query /TN "ESP32 PC Monitor Native"
```

---

# Troubleshooting

## ESP32 not detected

Verify Device Manager

```
Ports (COM & LPT)
```

Expected

```
USB Serial Device (COM4)
```

---

## COM Port Busy

Close:

* Arduino Serial Monitor
* Arduino Serial Plotter
* PuTTY
* VSCode Serial Monitor
* Any Python serial scripts

---

## Change Baud Rate

Current

```
115200
```

Both ESP32 firmware and native application must match.

---

# Current Dashboard Pages

Page 1

```
System Overview
```

Displays

* CPU
* RAM
* Disk
* Network

---

Page 2

```
CPU / RAM
```

Displays

* CPU Donut Chart
* RAM Donut Chart
* Memory Usage

---

Page 3

```
Disk
```

Displays

* Disk C Usage

---

Page 4

```
Network
```

Displays

* Upload
* Download

(Current implementation reports placeholder values until the native network collector is expanded.)

---

Page 5

```
Status
```

Displays

* Service Running
* Last Packet Time
* PC Uptime

---

Page 6

```
About
```

Displays

* ESP32 Information
* Heap Memory
* Dashboard Version

---

# ESP32 Controls

```
BOOT
```

Cycles

```
Page 1
Page 2
Page 3
Page 4
Page 5
Page 6
```

```
RST
```

Resets the ESP32.

---

# Current Architecture

```
Windows 11
      │
      ▼
Esp32PcMonitorNative.exe
      │
      ├── CPU Collector
      ├── RAM Collector
      ├── Disk Collector
      ├── Serial Manager
      └── Packet Builder
      │
      ▼
USB Serial (115200)
      │
      ▼
ESP32-C6
      │
      ▼
Rounded Dashboard UI
```

---

# Planned Roadmap

## Telemetry

* GPU Usage
* GPU VRAM
* GPU Temperature
* CPU Temperature
* SSD Temperature
* Disk Read/Write Speed
* Network Upload
* Network Download
* Battery Information
* Internet Latency

---

## AI Integration

* llama.cpp Status
* Ollama Status
* OpenGWN Status
* Active Model
* Tokens/sec
* Active Provider
* Model Context Size
* Inference Queue

---

## ESP32 Dashboard

* Rounded animated donut charts
* Smooth animations
* Page transitions
* FPS counter
* Live graphs
* Small trend history
* Theme system
* Day/Night mode
* Multiple dashboard layouts

---

# Performance Targets

Native Backend

```
CPU Usage
Target:
< 1%

Idle:
~0.05–0.20%
```

Memory

```
Target:
< 1 MB

Expected after optimization:
~500 KB to ~900 KB
```

Serial

```
1 packet/sec
```

No console window.

Automatic reconnect.

Automatic Windows startup.

Designed for continuous 24/7 operation.
