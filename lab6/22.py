import time
import pandas as pd
import requests
import joblib
import json
from iotc.models import Command
from iotc import IoTCClient, IOTCConnectType, IOTCEvents

# IoT Central device credentials
scope_id = '0ne00CE8964'
device_id = 'kvx09hbvz4'  # Replace with your actual device ID
device_key = 'rLlYVJkNJpTDWqQNPs+9qN88SGWpT3EaJ4eQOQd8Mpo='  # Replace with your actual device key
iotc_sub_domain = "khalil-iot-device1"  # Replace with your IoT Central subdomain
listening_device_id = "6q7c5htawl"  # Replace with your listening device ID
api_key = "SharedAccessSignature sr=16ac11da-3859-4507-b0e9-358df3141a7c&sig=zqPKOfZgDym%2F9VLz2ni%2Bq2lOCrNJDxZ%2BwK7EhvdlFsE%3D&skn=apalamlazure&se=1760638092063"  # Replace with your actual API key

# Load the trained model
model = joblib.load('iot_model_ap')  # Ensure the model is accessible

# Load the 5% split dataset (rain_validation.csv)
rain_validation = pd.read_csv("rain_validation.csv")  # Assuming you have this file

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

# Sending telemetry data and making predictions
for index, row in rain_validation.iterrows():
    if iotc.is_connected():
        # Prepare telemetry data
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

        # Prepare features for prediction
        features = [
            row['Location'],  # Ensure this is encoded as needed
            row['MinTemp'],
            row['MaxTemp'],
            row['Rainfall'],
            row['Evaporation'],
            row['Sunshine'],
            row['WindGustDir'],  # Ensure this is encoded as needed
            row['WindGustSpeed'],
            row['WindDir9am'],  # Ensure this is encoded as needed
            row['WindDir3pm'],  # Ensure this is encoded as needed
            row['WindSpeed9am'],
            row['WindSpeed3pm'],
            row['Humidity9am'],
            row['Humidity3pm'],
            row['Pressure9am'],
            row['Pressure3pm'],
            row['Cloud9am'],
            row['Cloud3pm'],
            row['Temp9am'],
            row['Temp3pm'],
            row['RainToday']  # This may not be needed for prediction
        ]
        
        # Reshape features for the model
        features = [features]

        # Make a prediction using the loaded model
        prediction = model.predict(features)

        # Check if the model predicts rain tomorrow
        if prediction[0] == 1:  # Assuming '1' indicates rain
            print("Rain predicted. Sending command to the listening device...")

            # Prepare command data
            command_data = {
                "command": "TurnOnRainAlert",
                "message": "Rain is predicted tomorrow. Prepare accordingly."
            }

            # Send the command to the listening device
            command_url = f"https://{iotc_sub_domain}.azureiotcentral.com/api/devices/{listening_device_id}/commands/SendData?api-version=2022-05-31"
            headers = {'Content-Type': 'application/json', "Authorization": api_key}

            response = requests.post(command_url, data=json.dumps(command_data), headers=headers)

            if response.status_code == 200:
                print("Command sent successfully!")
            else:
                print(f"Failed to send command: {response.text}")

        time.sleep(10)  # Adjust as needed (10 seconds per data entry)
