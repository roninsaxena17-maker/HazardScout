# HazardScout
HazardScout is a robot that can be controlled from a distance. It is built to check places that may be dangerous and to watch the conditions around it.

Project Description

HazardScout is a project about robots that explores places that may be unsafe or hard for people to reach. HazardScout uses sensors to gather data about the environment to find obstacles that're close and to send the data to a dashboard that people can watch. HazardScout mixes robotics, computer systems putting sensors together and wireless talk to make a real inspection robot that people can use.

Objectives

Build a HazardScout robot that can be controlled from a distance.

Monitor the temperature, humidity and gas levels in the environment.

Detect obstacles with a sensor.

Send sensor readings to a dashboard.

Explore whether a camera can let HazardScout do inspection.

Features

control that uses a FlySky FS-i6 transmitter.

Obstacle detection that uses an HC-SR04 sensor.

Temperature and humidity monitoring that uses a DHT22 sensor.

Smoke level monitoring that uses an MQ-2 sensor.

LED and buzzer alerts that warn about obstacles.

Sensor data logging and viewing that uses ThingSpeak.

Control and data processing that runs on an ESP32.

Hardware Components

Component | Purpose

ESP32 Main microcontroller

FlySky FS-i6 transmitter | Remote control

HC-SR04 ultrasonic sensor | Obstacle detection

DHT22 sensor | Temperature and humidity monitoring

MQ-2 sensor | Gas and smoke level monitoring

LED | Visual alert

Buzzer Audible alert

DC gear motors | Robot movement

Motor driver | Motor control

Battery | Power supply

ESP32-CAM | Planned visual inspection

Wokwi Simulation

HazardScout’s first sensor system is being made and tested with Wokwi.

The simulation includes:

ESP32

HC-SR04 ultrasonic sensor

DHT22 temperature and humidity sensor

MQ-2 gas sensor

LED and buzzer

Sensor readings are sent to ThingSpeak, where people can see them on graphs and in dashboard fields.

ThingSpeak Dashboard

ThingSpeak stores. Shows the sensor data that comes from the ESP32.

The dashboard includes:

Distance (cm)

Temperature (, °C)

Humidity (%)

MQ-2 analog sensor reading
