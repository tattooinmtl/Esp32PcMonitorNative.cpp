# ESP32 PC Monitor Native

A small, native Windows application that collects PC resource statistics and sends them once per second to an ESP32 over USB serial.

The application runs without a visible window and automatically reconnects if the ESP32 is unplugged or unavailable.

## Metrics sent

- CPU utilization, in percent
- RAM utilization, in percent
- Used and total physical RAM, in GiB
- Used space on the `C:` drive, in percent
- Windows uptime, in seconds

Upload and download values are included in the protocol for compatibility but are currently always `0.0`.

## Requirements

- Windows 10 or 11
- An ESP32 connected on `COM4` through `COM10`
- ESP32 firmware capable of reading the serial protocol described below
- Visual Studio with the **Desktop development with C++** workload to build from source

This repository contains the Windows sender application. It does not currently include the ESP32 display firmware.

## Build

1. Open `Esp32PcMonitorNative.slnx` in Visual Studio.
2. Select `Release` and `x64`.
3. Choose **Build > Build Solution**.
4. Find the executable at `x64\Release\Esp32PcMonitorNative.exe`.

The project uses C++20 and only Windows system APIs; it has no third-party dependencies.

## Run

Connect the ESP32 and launch `Esp32PcMonitorNative.exe`. The program tries `COM4` through `COM10` at 115200 baud, using 8 data bits, no parity, and one stop bit. Only one instance can run at a time.

Because it is a background Windows application, it does not open a console or status window. Use Task Manager to stop it.

## Start automatically with Windows

One option is to create a Scheduled Task:

1. Open **Task Scheduler** and choose **Create Task**.
2. Set the trigger to **At log on**.
3. Set the action to **Start a program** and select the built executable.
4. Save the task, then sign out and back in to test it.

Keep the executable at a permanent location before creating the task. Moving it later will break the task.

## Serial protocol

The application sends one UTF-8/ASCII line per second:

```text
CPU=12.3,RAM=45.6,USED=14.5,TOTAL=31.8,DISKC=62.1,UP=0.0,DOWN=0.0,UPTIME=12345
```

Each message ends with a newline. Numeric values use one decimal place, except uptime, which is an integer number of seconds.

| Field | Meaning |
| --- | --- |
| `CPU` | CPU utilization (%) |
| `RAM` | Physical RAM utilization (%) |
| `USED` | Used physical RAM (GiB) |
| `TOTAL` | Total physical RAM (GiB) |
| `DISKC` | Used space on the C: drive (%) |
| `UP` | Upload rate placeholder (currently `0.0`) |
| `DOWN` | Download rate placeholder (currently `0.0`) |
| `UPTIME` | Windows uptime (seconds) |

## Current limitations

- Serial ports are limited to `COM4` through `COM10`.
- Disk monitoring is limited to the `C:` drive.
- Network upload and download monitoring is not implemented.
- There is no configuration interface or tray icon.

