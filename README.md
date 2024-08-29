# ESP32 Radio and Connection Manager with OLED Interface

## Overview

This project uses an ESP32 microcontroller to create a multifunctional device that can:
- Scan and connect to WiFi networks.
- Scan and list Bluetooth devices.
- Stream online radio stations via Bluetooth using A2DP technology.
- Display information and manage user interactions through an OLED display.

## Components Used

1. **ESP32:** The central microcontroller that handles WiFi and Bluetooth connectivity, processes data, and controls the OLED display.
2. **OLED Display SSD1306:** Displays menus and information about connections and radio streams.
3. **WiFi and Bluetooth Modules:** The ESP32 integrates both WiFi and Bluetooth to enable internet connectivity and audio streaming to Bluetooth devices.
4. **Buttons:** Three buttons (Prev, Next, Select) allow the user to navigate through menus and select options.

## Key Features

1. **Menu Navigation:**
   - The device has a main menu that allows the user to choose between WiFi configuration, Bluetooth scanning, and online radio streaming.
   - The user can navigate through menus and select options using the buttons.

2. **WiFi Connectivity:**
   - The device can scan available WiFi networks and allows the user to select a network and enter the password to connect.
   - The connection status is displayed on the OLED screen, and the WiFi connection enables access to online radio streams.

3. **Bluetooth Device Scanning and Display:**
   - The device scans nearby Bluetooth devices and displays the scan results in the serial console.
   - This functionality allows the user to see what Bluetooth devices are available for connection.

4. **Online Radio Streaming:**
   - The device can play online radio streams specified from a predefined list of radio stations from Sweden.
   - The audio stream is transmitted via Bluetooth to compatible devices (e.g., Bluetooth headphones or speakers).
   - The user can navigate through stations and select the desired station for playback.

## Iterations and Challenges

1. **First Iteration: Arduino Nano ESP32:**
   - The initial attempt used the Arduino Nano ESP32.
   - **Challenge:** This module supports only Bluetooth Low Energy (BLE) and not classic Bluetooth, which is required for A2DP audio streaming.
   - **Result:** The device was unable to stream audio, leading to the selection of a different microcontroller.

2. **Second Iteration: ESP32-CAM:**
   - The second iteration used the ESP32-CAM module.
   - **Challenge:** Electrical issues were encountered due to some pins sharing connections with built-in components on the board, causing conflicts.
   - **Solution:** The problem was resolved by reconfiguring the pin connections to avoid conflicts with onboard components.
   - **Bluetooth Connectivity:** Successfully connected to Bluetooth headphones by hardcoding the device directly in the code.
   - **Limitation:** Could not implement Bluetooth scanning and pairing in this iteration, limiting the device to connect only to predefined Bluetooth devices.

## Project Purpose

The purpose of this project is to create a portable and versatile device that brings online radio streaming into an easy-to-use and accessible format. By utilizing the ESP32, the project showcases the advanced connectivity capabilities of this microcontroller, combining WiFi and Bluetooth functionalities into a compact and efficient solution. The project serves as a practical example for:
- Education and learning about IoT (Internet of Things) connectivity.
- Developing DIY projects that creatively combine hardware and software.
- Creating a customizable radio player that is easy to integrate into various applications (e.g., home audio systems).

## Benefits and Advantages

- **Intuitive Interface:** The OLED display provides clear visual feedback and allows easy navigation through menus.
- **Multi-Connectivity:** The device combines WiFi and Bluetooth functionalities, offering a wide range of options for connecting to the internet and external devices.
- **Portability and Flexibility:** The project can be powered by a mobile power source, such as a battery, making it ideal for on-the-go use.
- **Scalability:** The modular design of the project allows for adding new features or integrating with other IoT systems.
