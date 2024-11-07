---
title: 'Lab 5: Connecting IoT to the Cloud with Microsoft Azure'

---

# Lab 5: Connecting IoT to the Cloud with Microsoft Azure


**Group Members**:  
- Member 1: Khalil Ahmad  
- Member 2: Apala Pramanik  
- Member 3: Audrey Vazzana  
- Member 4: Raoul  


---

## Table of Contents
1. Introduction
2. Setting up IoT Central
3. DEevelopment Process
4. Dashboard Creation
5. Virtual Device Implementation
6. Results
7. Conclusion
8. Appendix: Code 

---

## 1. Introduction
In this lab, we utilized Azure IoT Central to simulate a cloud-based IoT application. Azure IoT Central is a FaaS that simplifies device management and telemetry data collection. Our goal was to create an IoT Central application, configure virtual devices to send telemetry data, and visualize this data on a custom dashboard. Each group member created and deployed their own virtual device to the cloud.

---

## 2. Development Process

In this section, we document the steps taken during the development of our IoT Central integration and highlight the challenges we faced along the way.

### 1. **Project Planning and Initial Setup**
The initial phase of development involved researching the Azure IoT Central platform and setting up the environment. The main goals were:
- Set up Azure IoT Central.
- Create a virtual device to simulate telemetry data transmission.
- Develop Python scripts for device-to-cloud communication.

Resources that helped during this phase:
- Azure IoT Central Documentation: Provided guidance on how to configure the IoT Central application and set up device credentials.
- Open-source projects and Python IoT libraries: These were referenced for implementing device command handling and telemetry data transmission.

### 2. **Coding and Device Implementation**
Each team member created a Python script to simulate a virtual device using the `iotc` Python library. Below were the key steps:
- **IoT Central connection**: We used the scope ID, device ID, and device key from our IoT Central application to connect each device.
- **Command Handling**: The devices were designed to receive a `SendData` command from IoT Central. When the command was received, telemetry data for temperature, wind speed, and humidity was generated and sent to the cloud.
- **Telemetry Data**: In addition to command-based data transmission, devices sent telemetry data at regular intervals (every 60 seconds).
  
### 3. **Challenges and Troubleshooting**
Several challenges arose during development, and the following troubleshooting techniques were used:
- **IoT Central Connection Issues**: At times, devices failed to connect due to incorrect credentials. The error was traced back to a misconfiguration in the device key or scope ID. Double-checking credentials resolved this issue.
- **Telemetry Data Format**: Initially, telemetry data was not displayed properly on the IoT Central dashboard. We discovered that the data format being sent was incorrect, and converting numerical data to strings resolved the problem.
- **Command Execution Failures**: While testing command execution, some devices failed to respond to the `SendData` command. This was resolved by ensuring that the `on_commands` function was properly registered and called upon receiving commands.
  
### 4. **Testing and Validation**
After resolving the major issues, the devices were tested thoroughly to ensure they could:
- Connect to IoT Central.
- Respond to commands.
- Send telemetry data at regular intervals.
  
We used the IoT Central dashboard to monitor the devices in real-time and validate the data being received.

### 5. **Lessons Learned**
Through the development process, we gained valuable insights into the complexities of IoT integration. Azure IoT Central simplifies many tasks, but ensuring accurate device communication and telemetry data requires careful attention to detail.



---

## 3. Device Configuration

While the rest of the code remains same, the IoT central credentials are different for each member. So, we only change the following 3 lines of code and replace the credentials with our own.


#### Images of device configurations here


##### Khalil device configuration on Azure IOT Central
![khalil_iot_device](https://hackmd.io/_uploads/rJCHZwEkJx.png)

##### Khalil IOT Central credentials
![khalil code ids](https://hackmd.io/_uploads/SkmPBU411g.png)



##### Apala device configuration on Azure IOT Central
![Apala_Iot_device](https://hackmd.io/_uploads/Hk6ZfP4kke.png)


##### Apala IOT Central credentials
![apala code ids](https://hackmd.io/_uploads/r1N7rLVkJe.png)

##### Audrey device configuration on Azure IOT Central
![audrey_iot_device](https://hackmd.io/_uploads/S14MzDV1ye.png)

##### Audrey IOT Central credentials
![audrey code ids](https://hackmd.io/_uploads/SJfOHU4Jyg.png)


##### Raoul device configuration on Azure IOT Central
![raoul_iot_device](https://hackmd.io/_uploads/HJofMvVkJx.png)

##### Raoul IOT Central credentials
![raoul code ids](https://hackmd.io/_uploads/BkFOr8E1kg.png)








---

## 4. Dashboard Creation
We created a custom dashboard to visualize the telemetry data from all devices. The following charts were included:

1. **Pie Chart**: Visualizes temperature, humidity and windspeed readings from all 4 devices as a pie chart.
2. **Line Chart**: Visualizes temperature, humidity and windspeed readings from all 4 devices as a line chart.
3. **Bar Chart**: Visualizes temperature, humidity and windspeed readings from all 4 devices as a bar chart.



#### Dashboard screenshots here

![pie chart](https://hackmd.io/_uploads/H1PmwLNy1e.png)
![dashboard zoom](https://hackmd.io/_uploads/rJSVDI4ykl.png)
![bar chart](https://hackmd.io/_uploads/S1Z8D84yJx.png)




---

## 5. Virtual Device Implementation

Each group member implemented their virtual device using the `iotc` Python library. Below is the code for one of the devices:


---

### 1. **IoT Central Credentials**
While the rest of the code remains same, the IoT central credentials are different for each member. So, we only change the following 3 lines of code and replace the credentials with our own. The scope ID, device ID, and device key are set with values obtained from a configuration file (`lab 5.txt`). These credentials allow the device to securely connect to IoT Central.

```python
scopeId = '0ne00CE8964'  # From lab 5.txt
device_id = '26znol6fzh7'  # From lab 5.txt
device_key = 'qoCaBmM9hRi7QJhw4HKIM9t68Od55cv2VhC5/NqNzWE='  # From lab 5.txt
```

---

### 2. **Command Handling Function**
This function handles incoming commands sent from IoT Central. If the `SendData` command is received, the device generates and sends telemetry data (temperature, wind speed, and humidity) and updates a property (`LastCommandReceived`).

```python
def on_commands(command: Command):
    print(f"Command received: {command.name}")
    
    if command.name == "SendData":
        print("Sending data as per the SendData command")
        iotc.send_telemetry({
            'Temperature': str(random.uniform(15.0, 30.0)),
            'WindSpeed': str(random.uniform(0.0, 15.0)),
            'Humidity': str(random.uniform(30.0, 70.0))
        })
        iotc.send_property({
            "LastCommandReceived": time.time()
        })
        print("Telemetry and LastCommandReceived property sent")
    
    command.reply()
```

---

### 3. **IoT Central Client Initialization**
The IoT Central client is initialized with the device credentials and the connection type is set to use a device key. This establishes the device's connection to IoT Central.

```python
iotc = IoTCClient(
    device_id,
    scopeId,
    IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
    device_key
)

iotc.connect()
```

---

### 4. **Setting Up Command Listener**
Once the connection is established, a listener is set up to handle incoming commands. The `on_commands` function is passed to the command event handler.

```python
iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)
```

---

### 5. **Sending Device Properties**
Upon powering up the device, it sends an initial property (`LastPowerOn`) to IoT Central, which contains the timestamp of when the device was last powered on.

```python
iotc.send_property({
    "LastPowerOn": time.time()
})

print("Device connected and LastPowerOn property sent")
```

---

### 6. **Telemetry Data Generation and Transmission**
In a continuous loop, telemetry data for temperature, wind speed, and humidity is randomly generated and sent to IoT Central every 60 seconds. The `is_connected()` method ensures that the loop runs only when the device is connected.

```python
while iotc.is_connected():
    temperature = random.uniform(15.0, 30.0)
    wind_speed = random.uniform(0.0, 15.0)
    humidity = random.uniform(30.0, 70.0)

    iotc.send_telemetry({
        'Temperature': str(temperature),
        'WindSpeed': str(wind_speed),
        'Humidity': str(humidity)
    })

    print(f"Telemetry sent - Temperature: {temperature}, Wind Speed: {wind_speed}, Humidity: {humidity}")

    time.sleep(60)
```



---

## 6. Results

- All devices successfully sent telemetry data to IoT Central.
- The commands from the IoT Central were executed on the virtual devices.
- We monitored the data in real-time using the dashboard, observing telemetry values and device status.
  

#### Images of data on Azure IoT Central here

#### Khalil's Data
![khalil commandprom](https://hackmd.io/_uploads/S12irLNkke.png)
![khalil 2](https://hackmd.io/_uploads/HykmUUNJ1x.png)
![khalil 1](https://hackmd.io/_uploads/ryy78UE1Je.png)

#### Apala's Data
![apala commandprom](https://hackmd.io/_uploads/SJZRr84ykx.png)
![apala2](https://hackmd.io/_uploads/rkV28IV1ke.png)
![apala1](https://hackmd.io/_uploads/ByEhUIE1Je.png)


#### Audrey's Data
![audrey commandprom](https://hackmd.io/_uploads/r1bkI84kJg.png)
![audrey2](https://hackmd.io/_uploads/rJa38LNkJl.png)
![audrey1](https://hackmd.io/_uploads/SyC2UIEkJe.png)



#### Raouls's Data
![Raoul commandprom](https://hackmd.io/_uploads/rkUgII4kkl.png)
![Raoul2](https://hackmd.io/_uploads/SJIaU8N1kg.png)
![Raoul1](https://hackmd.io/_uploads/H1LpU84y1l.png)

---

## 7. Conclusion
This lab allowed us to understand how IoT devices connect to the cloud using Azure IoT Central. We were able to create virtual devices, send telemetry data, and respond to cloud commands. The simplicity of the IoT Central platform made it easier to manage multiple devices and analyze the data.

---

## 8. Appendix: Code
Python script with proper documentation.


```python
import random
import time
from datetime import datetime
from iotc.models import Command
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

# Replace these with your IoT Central credentials
scopeId = '0ne00CE8964'  # From lab 5.txt
device_id = '26znol6fzh7'  # From lab 5.txt
device_key = 'qoCaBmM9hRi7QJhw4HKIM9t68Od55cv2VhC5/NqNzWE='  # From lab 5.txt

# Function to handle incoming commands
def on_commands(command: Command):
    print(f"Command received: {command.name}")
    
    if command.name == "SendData":
        # Send data when the "SendData" command is received
        print("Sending data as per the SendData command")
        iotc.send_telemetry({
            'Temperature': str(random.uniform(15.0, 30.0)),
            'WindSpeed': str(random.uniform(0.0, 15.0)),
            'Humidity': str(random.uniform(30.0, 70.0))
        })
        # Update the LastCommandReceived property
        iotc.send_property({
            "LastCommandReceived": time.time()
        })
        print("Telemetry and LastCommandReceived property sent")
    
    # Acknowledge the command
    command.reply()

# Initialize IoTC client
iotc = IoTCClient(
    device_id,
    scopeId,
    IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
    device_key
)

iotc.connect()

# Set up the command listener
iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)

# Send the LastPowerOn property once the device is powered on
iotc.send_property({
    "LastPowerOn": time.time()
})

print("Device connected and LastPowerOn property sent")

# Start sending telemetry data every 60 seconds
while iotc.is_connected():
    # Generate and send telemetry data for temperature, wind speed, and humidity
    temperature = random.uniform(15.0, 30.0)  # Simulated temperature in Celsius
    wind_speed = random.uniform(0.0, 15.0)    # Simulated wind speed in km/h
    humidity = random.uniform(30.0, 70.0)     # Simulated humidity in percentage

    # Send telemetry data
    iotc.send_telemetry({
        'Temperature': str(temperature),
        'WindSpeed': str(wind_speed),
        'Humidity': str(humidity)
    })

    print(f"Telemetry sent - Temperature: {temperature}, Wind Speed: {wind_speed}, Humidity: {humidity}")

    # Wait for 60 seconds before sending the next telemetry data
    time.sleep(5)
```
