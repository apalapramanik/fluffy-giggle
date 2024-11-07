import pandas as pd
import numpy as np
import pickle
import time
from iotc import IoTCClient, IOTCConnectType, IOTCEvents  # Assuming IoT Central client library
from sklearn.preprocessing import StandardScaler

# Load the trained model
model_filename = 'trained_model.pkl'  # Update with actual file name
with open(model_filename, 'rb') as model_file:
    trained_model = pickle.load(model_file)

# Load and scale the dataset (use the same preprocessing used during training)
data_filename = 'weatherAUS_validation.csv'  # Update with actual file name
rain_validation = pd.read_csv(data_filename)
scaler = StandardScaler()

# Assuming 'rain_validation' is preprocessed in the same way the training data was
numerical_features = ['MinTemp', 'MaxTemp', 'Rainfall', 'Evaporation', 'Sunshine', 'WindGustSpeed']  # Add the rest
rain_validation[numerical_features] = scaler.fit_transform(rain_validation[numerical_features])

# IoT Central setup
scope_id = "0ne00D18E8C"
device_id = "13rio316qdu"
device_key = "6VdOgcQq5PagwDmn/MOXHtTiNhQzrKYZmGvrDfaZnaU="

# Define a command callback
def on_commands(command):
    print(f"Received command: {command.name}")
    
    # Predict using the trained model
    if command.name == 'PredictRain':  # Example command name
        # Prepare features for the model
        features = rain_validation.iloc[0]  # Select first row, modify as needed
        prediction = trained_model.predict([features])[0]
        
        # Log the prediction
        print(f"Prediction made: {prediction}")
        
        # Send a command to a listener device based on the prediction
        if prediction == 1:  # Example logic: 1 could mean rain is predicted
            print("Sending 'TurnOnFan' command as rain is predicted.")
            iotc.send_property({'Command': 'RainPredicted'})
        else:
            print("Sending 'TurnOffFan' command as no rain is predicted.")
            iotc.send_property({'Command': 'NORainPredicted'})
        
        command.reply()

# Connect to IoT Central
iotc = IoTCClient(device_id, scope_id, IOTCConnectType.IOTC_CONNECT_DEVICE_KEY, device_key)
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
        
        # Predict rain and send commands based on the prediction
        features = row[numerical_features].values.reshape(1, -1)
        prediction = trained_model.predict(features)[0]
        print(f"Prediction: {prediction}")
        
        if prediction == 1:
            iotc.send_property({'Command': 'RainPredicted'})
        else:
            iotc.send_property({'Command': 'NORainPredicted'})
        
        # Sleep for a short duration before sending the next row of data
        time.sleep(10)  # Adjust as needed (10 seconds per data entry)
