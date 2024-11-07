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
