---
publishDate: 2026-08-25

title: LogiShield - Offline Package Tamper Monitoring System

excerpt: Our project aims to resolve the problem of tampering before accepting the delivery package. 

image: coverpicture.png

tags:
  - Delivery
  - Customer Satisfaction
  - Package
  - Tamper

---

> Dont doubt, just get your package checked before opening! 

---


## Acknowledgements

LogiShield is developed by Suprio Dutta, Subhadeep Sardar, and Arkaprava Bhattacharya, and is purpose-built for the IEEE MYOSA Event.

We would like to extend special thanks to the MYOSA platform. This platform had all sensing-related functions pre-integrated, providing a ready-to-use embedded sensing environment that eliminated the need for us to assemble and debug components from scratch, which enabled us to integrate mechanical, optical, and barometric pressure signals into a single asset integrity verification system.

We also thank our respected mentor, Dr Prasun Chowdhury, HOD of ECE Department, St.Thomas' College of Engineering & Technology, Khidderpore  whose valuable guidance and professional technical insights helped us refine the three core functional directions of this system: physical tamper detection, event logging, and secure package verification.

The APDS9960 and MPU650 sensors were damaged and did not work properly, which was later verified by MYOSA community. So we changed the sensors with market purchase sensors and worked with them. Therefore we were unable to use the gesture sensing mode of the APDS9960 sensor.

---

## Overview

LogiShield is a smart intelligent security system which provides the physical evidence of tampering in high value shipments using multiple sensing technologies.

The main problem that LogiShield aims to solve is insider swapping. Where the original contents of any valuable parcel are removed and something else is replaced with the contents. Then the parcel is resealed and sent onward.Traditional tracking system can tell about where a parcel has travelled basically the movement of the parcel but it can’t tell whether the parcel was tampered, opened or damaged during its journey.

LogiShield turns the parcel into a smart, self-monitoring system that can check continuously about the physical condition of the parcel throughout its journey. It combines mechanical, optical and barometric sensors to identify physical events such as impacts, unwanted light entering the package and possible parcel opening.  Whenever an unusual event is detected, LogiShield records the exact time with the help of a dedicated RTC and stores the data locally. So, the receiver can review the package’s journey later. It uses ESP32-S3 as its main controller integrating the MPU6050, BMP180, and APDS9960 sensors with an OLED display and real-time clock. Before accepting the delivery, the customer can simply scan a QR code to see the parcel’s journey and verify whether any tampering has occurred with the parcel or not. The current development also focuses on detecting when the box is opened using physical and hardware-based methods especially RGB variations. This approach aims to make the system more reliable while reducing the dependency on complex software calculations.

**Key Features**
-   Multi-modal tamper detection using mechanical, optical, and barometric sensing.
-   Impact and shock detection using the MPU6050.
-   Light intrusion detection using the APDS9960.
-   Accurate event timestamping using the DS3231 RTC.
-   Offline forensic logging to preserve evidence without continuous internet connectivity.
-   Dedicated mobile application for scanning and interpreting the generated QR code.
-   Low-power autonomous operation for extended logistics journeys.
-   Dynamic QR-code verification.
-   Customer-side package integrity verification before accepting delivery.
-   Cost-effective and scalable design for high-value logistics and supply-chain applications.
---

## DEMO/Examples

### **Images**

<p align="center">
  <img src="./demo_pictures_videos/module_image.jpeg" width="800"><br/>
  <i>The hardware build for LogiShield</i>
</p>



<p align="center">
  <img src="./demo_pictures_videos/oled_image.png" width="800"><br/>
  <i>OLED showing live parameters and tamper count</i>
</p>


<p align="center">
  <img src="./demo_pictures_videos/qr_code_image.jpeg" width="800"><br/>
  <i>QR_Code generation in OLED containing tampering data</i>
</p>


<p align="center">
  <img src="./demo_pictures_videos/dashboard.jpeg" width="800"><br/>
  <i>Tampering analysis in dedicated web application in a Dashboard format</i>
</p>


### **Videos**

<video controls width="100%">
  <source src="./demo_pictures_videos/demo_video.mp4" type="video/mp4">
</video>


---

## Features (Detailed)

**1. Multi-Modal Tamper Detection**
LogiShield does not depend on just one sign of tampering. It usages multiple sensors to watch different unusual events that occurred to the parcel.

 - BMP180- It monitors the air pressure inside the parcel. If there is change in the pressure then it detects that parcel may have been dropped heavily. 
 - APDS9960- It detects unexpected light entering the package showing probable opening of package.
 - MPU6050 -Detects sudden unusual movements, shocks, and impacts that may occur during its journey.
 
 **2**. ****Box-Opening Detection****
One of the main goals of LogiShield is to identify whether the cardboard package has been physically opened.

To do this, the system uses APDS9960 sensor to monitor the light if enters the parcel. If the parcel opened that can change the RGB configuration inside the parcel, which indicate that the parcel has ben opened.

**3.** **Impact and Movement Detection**
The MPU6050 monitors the movement and acceleration of the parcel during the it’s transportation. If the parcel is hit or faced sudden sharp movement, the system detects it and record it as part of it’s tamper history.

**4.** **Timestamped Event Recording**
The DS3231 RTC is used to provide accurate time information. If LogiShield detects an unusual event, it records the time along with the data. This creates a timeline of what the parcel experienced during its journey.

**5.** **Offline Event Storage**
To avoid dependency on network connectivity, LogiShield stores the recorded into ESP32 using LittleFS. This ensures that the recorded evidence remains safe during the parcel’s journey, even in area where network connection is not available.

**6.** **Two-Press Activation**
The physical control switch provides a simple way to activate the system.

Pressing the switch 2 times starts the active monitoring stage. When the parcel is the packaged and considered out for delivery, this switch is turned ON. From this point onward, the system keeps monitoring the parcel throughout the journey until it reaches to the destination.

**7.** **Three-Press QR Generation**
When the parcel reaches the customer, the customer can get the recorded information of the parcel by pressing the switch 3 times. Then a QR code will generate on the SSD1306 OLED.

**8.** **OLED-Based Verification**
The OLED display is used to show the generated QR code. Customers can simply scan the QR code to view the relevant information through the verification application.

**9.** **Dedicated Mobile Application**
Our team has developed a mobile application through which customers can scan the QR code and view the recorded information about the parcel in a dashboard format.

---

## Usage Instructions

**Hardware Demonstration:**

1. Insert the LogiShield into the package and seal the package properly.
2. Power on the device before shipping the parcel.
3. Before the parcel goes for out of delivery, press the control switch 2 times. This activates LogiShield and it starts monitoring the parcel.
4. During the transportation of the parcel, the LogiShield uses BMP180, APDS9960, and MPU6050 sensors to monitor pressure variations, light invasion, and mechanical vibrations, respectively.
5. Whenever any unusual event detected, the system stores the event data along with the exact time using the DS3231 RTC.
6. When the parcel reaches to the destination, the customer presses the control switch thrice to generate the QR code.
7. LogiShield then creates a QR code and shows it on the SSD1306 OLED screen.
8. Then the customer scans the QR code using LogiShield mobile application.
9. The app then shows what happened to the parcel during its journey, whether any unusual event occurred or not. If occurred then the recorded event with exact time.
---

## Tech Stack

 - **Hardware:** 
    * MYOSA ESP32-S3
    * BMP180 Pressure Sensor 
    * MPU6050 Accelerometer/Gyroscope
    * APDS9960 Optical Sensor
    * DS3231 RTC
    * SSD1306 OLED Display
    * Push Switch 
    * Power Supply

 - **Firmware:** 
    * C++
    * Arduino Framework
    * I²C communication 

 - **Library:** 
    * Wire 
    * Adafruit SSD1306 
    * QRcodeOled
    * RTClib by Adafruit
    * Myosa - LightProximityAndGesture
    * Myosa - BarometricPressure

 - **WEB Interface:** 
    * React Library
      + HTML
      + Tailwind CSS
      + Javascript
    * JSON
    * html5-qrcode
    * *(NO Backend Required)*
---


## Requirements/ Installation

**Hardware Requirements:**

  - ESP32 Development Board
  - MPU6050 Sensor
  - BMP180 Sensor
  - APDS9960 Sensor
  - DS3231 RTC
  - SSD1306 OLED Display

**Software Requirements:**

  - Arduino IDE
  - ESP32 board support package
  - Required Arduino Libraries
  - Any Web Browser

**Arduino Installation:**

  - Install the Arduino IDE
  - Install the ESP32 board support package through the Arduino Board Manager
  - Install the libraries required by the respective firmware
  - Open the required [`.ino`](logishield_arduino_code.ino) file from the repository
  - Select the appropriate board and serial port
  - Compile and upload the file

**Web Application:**
  
  - Press the switch in the hardware module 3 times and get the QR code.
  - Open the web application from the provided link below

       [`Logishield_Application`](https://logishield.vercel.app/)

  - Click on the 'Scan' Option in the header.
  - Upload the QR code image using live picture or upload from gallery.
  - Click on 'Scan' to get the analysis result. 
---

## File Structure

```
/logishield
  ├─ logidshield_arduino_code.ino
  ├─ demo_pictures_videos/
  │   ├─ dashboard.jpeg
  │   ├─ module_image.jpeg
  │   ├─ oled_image.png
  │   ├─ qr_code_image.jpeg
  │   └─ demo_video.mp4
  ├─ coverpicture.png
  └─ README.md
```
---
## License 

This project is licensed under the MIT License.

---


## Contribution Notes

Any issues and upgradation are welcome and can reach out to us by our Socials provided below:
   - Suprio Dutta - inform.suprio@gmail.com
   - Subhadeep Sardar - ssubhadeep500@gmail.com
   - Arkaprava Bhattacharya - arkaprava181204@gmail.com

The public Github Repo containing our Arduino and Frontend codes are given below:- 
    
  [`Logishield_Github_Repo`](https://github.com/arkaprava181204/Logishield_a_packet_tracker_Myosa_6_0.git) 

*Interested can fork the Repo to make contributions.* 


