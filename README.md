# Smart Shopping Trolley using ESP32 & GROW-GM861S 🛒

## 📋 Project Overview
This repository contains the firmware and web-interface code for an automated **Smart POS Billing Terminal** integrated directly into a shopping trolley. 

By utilizing an **ESP32** microcontroller and a **GROW-GM861S** barcode scanner, this project allows users to scan items as they place them into their cart. The system instantly calculates the total cost, displays it on a local hardware screen, and seamlessly syncs with a responsive **Web Dashboard** that can be accessed via a smartphone.

## ✨ Key Features
* **Real-Time Scanning:** High-speed 1D and 2D barcode scanning using the active scanning tag of the GROW-GM861S module.
* **Interactive Web Dashboard:** The ESP32 hosts an embedded web server displaying a "Product Counter Sheet" that tracks item codes, product names, individual prices, and automatically updates the grand total.
* **Local Hardware Feedback:** A physical LCD screen is mounted on the cart to display the current item count and the running total (e.g., `Total: Rs. 22`) without needing to check the phone.
* **Fully Wireless & Portable:** Powered by a rechargeable 18650 battery unit, communicating entirely over local Wi-Fi.

## 🛠️ Hardware Components
* **Microcontroller:** ESP32 Development Board
* **Scanner:** GROW-GM861S 1D/2D Barcode Scanner Module
* **Display:** 16x2 Character LCD Display (with I2C interface)
* **Power:** 3.7V 18650 Li-ion Battery and Battery Holder
* **Miscellaneous:** Custom PCB / Perfboard, jumper wires, and passive components.

## 🚀 Installation & Setup

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/PrateekSinghRajput/Smart-Shopping-Trolley-ESP32-GROW-GM861S-Barcode-Scanner.git](https://github.com/PrateekSinghRajput/Smart-Shopping-Trolley-ESP32-GROW-GM861S-Barcode-Scanner.git)
