# HazardScout
A remote control robot used to inspect potentially dangerous areas.

Overview

HazardScout is a robot project which inspects potentially dangerous or difficult to reach areas. The robot utilises sensors to record information about the surrounding area to detect nearby hazards and display data on a dashboard. The robot combines robotics, sensor integration of computer systems and wireless communication to create a useful inspection robot.

Objectives

Create a remote control robot for HazardScout.

Monitor temperature, humidity and gas levels in the area.

Detect hazards using a sensor.

Display readings from the sensors on a dashboard.

Investigate using a camera for HazardScout to inspect.

Features

Remote control using a FS-i6 transmitter.

Obstacle detection using an HC-SR04 sensor.

Temperature and humidity monitoring using a DHT22 sensor.

Smoke detection using an MQ-2 sensor.

Alerts using LED and buzzer.

Recording and displaying sensor data using ThingSpeak.

Everything is controlled and processed using an ESP32.

Parts

Part

Purpose

ESP32

Main microcontroller.

FlySky FS-i6

Remote control.

HC-SR04

Obstacle detection.

DHT22

Temperature and humidity monitoring.

MQ-2

Gas and smoke monitoring.

LED

Visual alert.

Buzzer

Audio alert.

DC Gear Motors

Robot movement.

Motor Driver

Controlling the motors.

Battery

Power supply.

ESP32-CAM

For visual inspection (planned).

Wokwi Simulation

HazardScout has been created with its initial sensor system using Wokwi.

This has the following components:

ESP32

HC-SR04 Sensor

DHT22 Sensor (temperature and humidity)

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
