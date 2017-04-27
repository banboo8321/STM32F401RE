/**
  @page Microsoft Azure IoT HTTP Client Application
  
  @verbatim
  ******************** (C) COPYRIGHT 2015 STMicroelectronics *******************
  * @file    readme.txt  
  * @version V2.0.0
  * @date    15-11-2016
  * @brief   This application contains an example showing how to connect a Nucleo 
based microsystem to Microsoft Azure IoT. Updated to Microsoft Azure IoT SDK v1.0.11

  ******************************************************************************
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  @endverbatim

@par Example Description 

FP-CLD-AZURE1 provides a software running on STM32 which offers a complete middleware to build applications based on 
WiFi connectivity (SPW01SA) and to connect STM32 Nucleo boards with Microsoft Azure IoT services. The software leverages 
functionalities provided by the following expansion boards:
   -X-NUCLEO-IKS01A1 or X-NUCLEO-IKS01A2 featuring temperature, humidity, pressure and motion MEMS sensors
   -X-NUCLEO-IDW01M1 a Wi-Fi evaluation board based on the SPWF01SA module
   -X-NUCLEO-NFC01A1 a Dynamic NFC tag evaluation board

FP-CLD-AZURE1 is an function package software for STM32Cube. The software runs on the STM32 micro-controller and 
includes drivers that recognize WiFi module (SPWF01SA), sensor devices (HTS221, LPS25HB, LPS22HB, LSM6DS0, LIS3MDL, LSM6DSL, LSM303AGR) and dynamic 
NFC/RFID tag (M24SR64-Y). It also includes the porting of the Microsoft Azure IoT device SDK for easy connection with 
Azure IoT services. The expansion software is built on STM32Cube software technology 
to ease portability across different STM32 microcontrollers. The software comes with examples for registering the 
device on Microsoft Azure IoT Hub, transmit data and receive commands.
  
@par Hardware and Software environment

HW setup for running the application is composed by the following boards:
	- STM32F401RE-Nucleo RevC 
	- X-NUCLEO-IDW01M1
	- X-NUCLEO-IKS01A1 / X-NUCLEO-IKS01A2
	- X-NUCLEO-NFC01A1 
The Wi-Fi expansion board X-NUCLEO-IDW01M1 can be connected to the STM32 Nucleo motherboard through the Morpho extension connector.
The Dynamic NFC tag board X-NUCLEO-NFC01A1 can be connected to X-NUCLEO-IDW01M1 expansion board through the Arduino UNO R3 extension 
connector.
Finally, the sensors boards X-NUCLEO-IKS01A1 or X-NUCLEO-IKS01A2 can be easily connected to X-NUCLEO-NFC01A1 expansion board through the Arduino UNO R3 
extension connector.

    

@par How to use it? 

Before you can run the supplied sample application, you must first create an account on Microsoft Azure and then create an instance of the Azure IoT hub following instruction provided here:
https://github.com/Azure/azure-iot-sdks/blob/master/doc/setup_iothub.md. 
Azure IoT hub is a fully managed service that enables reliable and secure bi-directional communication between the STM32 Nucleo based microsystem and cloud applications. 
Please refer to User Manual for more information on how to register Nucleo based microsystem to Microsoft Azure IoT Hub and generate the correspondent connection string.

This package contains projects for 3 IDEs IAR, µVision and System Workbench. 
In order to make the  program work, you must do the following:
 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.
   
For IAR:
 - Open IAR toolchain (this firmware has been successfully tested with
   Embedded Workbench V7.40.3).
 - Open one of the IAR project file EWARM\STM32F401RE-Nucleo\Project.eww
 - Rebuild all files and load your image into target memory.
 - Run the example.

For µVision:
 - Open µVision V5.15.0 toolchain (this firmware has been 
   successfully tested with MDK-ARM Professional Version: 5.15.0).
 - Open one of the µVision project files MDK-ARM\STM32F401RE-Nucleo\Project.uvprojx
 - Rebuild all files and load your image into target memory.
 - Run the example.
 
For System Workbench:
 - Open System Workbench for STM32 (this firmware has been successfully tested with System Workbench for STM32 Version 1.6.0.201512071631). 
 - Set the default workspace proposed by the IDE (please be sure that there are not spaces in the workspace path).
 - Press "File" -> "Import" -> "Existing Projects into Workspace"; press "Browse" in the "Select root directory" and choose the path where the System
   Workbench project is located (it should be SW4STM32\STM32F401RE-Nucleo). 
 - Rebuild all files and load your image into target memory.
 - Run the example.

Once launched the application, it is possible to dynamically change Wi-Fi Access Point parmaters via serial interface or using the NFC board and a 
companion mobile application. 
Once connected to an access point, the application will contact you Azure IoT Hub, trasmitting sensors data and received commands from remote applications. 

Serial interface configuration:
- BaudRate: 460800
- Data: 8 bit
- Parity: none
- Stop: 1 bit
- Flow Control: none
- NewLine RX: CR+LF
- NewLine TX: CR


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */




