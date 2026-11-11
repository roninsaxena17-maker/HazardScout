# HazardScout
A remote control robot used to inspect potentially dangerous areas.

Overview:

HazardScout is a robot project which inspects potentially dangerous or difficult to reach areas. The robot utilises sensors to record information about the surrounding area to detect nearby hazards and display data on a dashboard. The robot combines robotics, sensor integration of computer systems and wireless communication to create a useful inspection robot.

Objectives:

Create a remote control robot for HazardScout.

Monitor temperature, humidity and gas levels in the area.

Detect hazards using a sensor.

Display readings from the sensors on a dashboard.

Investigate using a camera for HazardScout to inspect.

Features:

Remote control using a FS-i6 transmitter.

Obstacle detection using an HC-SR04 sensor.

Temperature and humidity monitoring using a DHT22 sensor.

Smoke detection using an MQ-2 sensor.

Alerts using LED and buzzer.

Recording and displaying sensor data using ThingSpeak.

Everything is controlled and processed using an ESP32.

Parts:

Part -                                             Purpose

ESP32 -                                            Main microcontroller.

FlySky FS-i6 -                                     Remote control.

HC-SR04 -                                          Obstacle detection.

DHT11 -                                            Temperature and humidity monitoring.

MQ-2 -                                             Gas and smoke monitoring.

LED -                                              Visual alert.

Buzzer -                                           Audio alert.

DC Gear Motors -                                   Robot movement.

Motor Driver -                                     Controlling the motors.

Battery -                                          Power supply.

ESP32-CAM -                                        For visual inspection (planned).


Wokwi Simulation:                 
HazardScout has been created with its initial sensor system using Wokwi.

This has the following components:

ESP32

HC-SR04 Sensor

DHT11 Sensor (temperature and humidity)

MQ-2 Sensor (gas)

LED and Buzzer.

The sensor data is sent to ThingSpeak to be displayed to users in graphs and on the dashboard.

ThingSpeak Dashboard

This is where the sensor data is stored. It displays the data from the sensors on the ESP32.

The dashboard displays:

Distance (cm)

Temperature ( , C)

Humidity (%)

MQ-2 (Analog Sensor)

Temperature (, °C)

Humidity (%)

MQ-2 analog sensor reading



# 1. ESP32 Pin Mapping

| Component               | Component pin      | ESP32 GPIO    |
| ----------------------- | ------------------ | ------------- |
| FlySky FS-iA6B receiver | iBUS signal        | GPIO 16 (RX2) |
| Left BTS7960            | RPWM               | GPIO 25       |
|                         | LPWM               | GPIO 26       |
|                         | R_EN               | GPIO 32       |
|                         | L_EN               | GPIO 33       |
| Right BTS7960           | RPWM               | GPIO 27       |
|                         | LPWM               | GPIO 14       |
|                         | R_EN               | GPIO 19       |
|                         | L_EN               | GPIO 23       |
| HC-SR04                 | TRIG               | GPIO 5        |
|                         | ECHO               | GPIO 18       |
| DHT11                   | DATA               | GPIO 4        |
| MQ-2                    | AO (Analog Output) | GPIO 34       |
| MPU-6050                | SDA                | GPIO 21       |
|                         | SCL                | GPIO 22       |

<img width="846" height="372" alt="Screenshot 2026-11-11 at 3 11 21 PM" src="https://github.com/user-attachments/assets/4ad6bee4-adf7-4a48-b704-efddb4d1ed18" />


# 2. ESP32-CAM Pin Mapping

| Camera signal | ESP32-CAM GPIO |
| ------------- | -------------- |
| PWDN          | 32             |
| RESET         | -1             |
| XCLK          | 0              |
| SIOD          | 26             |
| SIOC          | 27             |
| Y9            | 35             |
| Y8            | 34             |
| Y7            | 39             |
| Y6            | 36             |
| Y5            | 21             |
| Y4            | 19             |
| Y3            | 18             |
| Y2            | 5              |
| VSYNC         | 25             |
| HREF          | 23             |
| PCLK          | 22             |
| Flash LED     | 4              |

<img width="431" height="370" alt="Screenshot 2026-11-11 at 3 17 36 PM" src="https://github.com/user-attachments/assets/4410a5ae-31f8-4707-9ce4-dd79d3bd0a53" />

