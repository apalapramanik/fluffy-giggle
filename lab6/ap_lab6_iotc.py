import time
import pandas as pd
from iotc.models import Command
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

# IoT Central device credentials
scope_id = '0ne00D18E8C'
device_id = '1oqxy244di7'
device_key = 'm3k5AQsOKu9PRApxzXOFFEDaJSozi/CeG7dSEIQPnww='

# Load the 5% split dataset (rain_validation.csv)
rain_validation = pd.read_csv("rain_validation.csv")  # Assuming you have already split and saved this portion

# Function to handle incoming commands
def on_commands(command: Command):
    print(f"{command.name} command was sent")
    command.reply()

# Connect to IoT Central
iotc = IoTCClient(
    device_id,
    scope_id,
    IOTCConnectType.IOTC_CONNECT_DEVICE_KEY,
    device_key
)

iotc.connect()
iotc.on(IOTCEvents.IOTC_COMMAND, on_commands)

# Sending telemetry data from the 5% split dataset
for index, row in rain_validation.iterrows():
    if iotc.is_connected():
        TEL = {
                         
            'Location': row['Location'],
            'MinTemp': row['MinTemp'],
            'MaxTemp': row['MaxTemp'],
            'Rainfall': row['Rainfall'],
            'Evaporation': row['Evaporation'],
            'Sunshine': row['Sunshine'],
            'WindGustDir': row['WindGustDir'],
            'WindGustSpeed': row['WindGustSpeed'],
            'WindDir9am': row['WindDir9am'],
            'WindDir3pm': row['WindDir3pm'],
            'WindSpeed9am': row['WindSpeed9am'],
            'WindSpeed3pm': row['WindSpeed3pm'],
            'Humidity9am': row['Humidity9am'],
            'Humidity3pm': row['Humidity3pm'],
            'Pressure9am': row['Pressure9am'],
            'Pressure3pm': row['Pressure3pm'],
            'Cloud9am': row['Cloud9am'],
            'Cloud3pm': row['Cloud3pm'],
            'Temp9am': row['Temp9am'],
            'Temp3pm': row['Temp3pm'],
            'RainToday': row['RainToday']
        }
        
        # Send telemetry to IoT Central
        iotc.send_telemetry(TEL)
        print(f"Telemetry sent: {TEL}")
        
        # Sleep for a short duration before sending the next row of data
        time.sleep(10)  # Adjust as needed (10 seconds per data entry)



###################################################################################################################

"""
listening device

scope: 0ne00D18E8C
dev id: 13rio316qdu
primary : 6VdOgcQq5PagwDmn/MOXHtTiNhQzrKYZmGvrDfaZnaU=


"""