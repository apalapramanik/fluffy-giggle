import azure.functions as func
import joblib
import logging
import json
import requests

app = func.FunctionApp(http_auth_level=func.AuthLevel.FUNCTION)

model = joblib.load('iot_model_ap')



@app.route(route="http_trigger_lab6", auth_level=func.AuthLevel.FUNCTION)
def http_trigger_lab6(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    try:
        # Get request body data
        req_body = req.get_json()
        logging.info(f"Request body: {req_body}")
        
        # Extract the necessary features for prediction 
        features = [
             'Location': req_body.get('Location'),
            'MinTemp': req_body.get('MinTemp'),
            'MaxTemp': req_body.get('MaxTemp'),
            'Rainfall': req_body.get('Rainfall'),
            'Evaporation': req_body.get('Evaporation'),
            'Sunshine': req_body.get('Sunshine'),
            'WindGustDir': req_body.get('WindGustDir'),
            'WindGustSpeed': req_body.get('WindGustSpeed'),
            'WindDir9am': req_body.get('WindDir9am'),
            'WindDir3pm': req_body.get('WindDir3pm'),
            'WindSpeed9am': req_body.get('WindSpeed9am'),
            'WindSpeed3pm': req_body.get('WindSpeed3pm'),
            'Humidity9am': req_body.get('Humidity9am'),
            'Humidity3pm': req_body.get('Humidity3pm'),
            'Pressure9am': req_body.get('Pressure9am'),
            'Pressure3pm': req_body.get('Pressure3pm'),
            'Cloud9am': req_body.get('Cloud9am'),
            'Cloud3pm': req_body.get('Cloud3pm'),
            'Temp9am': req_body.get('Temp9am'),
            'Temp3pm': req_body.get('Temp3pm'),
            'RainToday': req_body.get('RainToday')
        ]
        
        # Convert features to the correct format (e.g., NumPy array or DataFrame)
        # Assuming the model expects a 2D array input
        features = [features]  # Reshape into 2D array if necessary
        
        # Make a prediction using the loaded model
        prediction = model.predict(features)
        
        logging.info(f"Model prediction: {prediction}")
        
        # Check if the model predicts rain tomorrow (assuming 'RainTomorrow' is 1 for rain, 0 for no rain)
        if prediction[0] == 1:  # Modify based on how your model encodes rain
            logging.info("Rain predicted for tomorrow. Sending command to device...")
            
            # Send a command to the listening device (Assume the device is reachable via an API)
            device_command_url = 'https://your-device-command-url'  # Replace with actual URL
            command_data = {
                "command": "TurnOnRainAlert",
                "message": "Rain Predicted Tomorrow!"
            }
            headers = {'Content-Type': 'application/json'}
            
            response = requests.post(device_command_url, data=json.dumps(command_data), headers=headers)
            
            if response.status_code == 200:
                logging.info(f"Command sent to device successfully: {response.text}")
                return func.HttpResponse(f"Rain predicted, command sent to device successfully: {response.text}", status_code=200)
            else:
                logging.error(f"Failed to send command to device: {response.text}")
                return func.HttpResponse(f"Rain predicted, but failed to send command to device: {response.text}", status_code=500)
        else:
            logging.info("No rain predicted for tomorrow.")
            return func.HttpResponse("No rain predicted for tomorrow.", status_code=200)
    
    except Exception as e:
        logging.error(f"Error processing the request: {str(e)}")
        return func.HttpResponse(f"Error processing the request: {str(e)}", status_code=500)


# @app.route(route="http_trigger_lab6", auth_level=func.AuthLevel.FUNCTION)
# def http_trigger_lab6(req: func.HttpRequest) -> func.HttpResponse:
#     logging.info('Python HTTP trigger function processed a request.')

#     name = req.params.get('name')
#     if not name:
#         try:
#             req_body = req.get_json()
#             logging.info(req_body)
#         except ValueError:
#             pass
#         else:
#             name = req_body.get('name')

#     if name:
#         return func.HttpResponse(f"Hello, {name}. This HTTP triggered function executed successfully.")
#     else:
#         return func.HttpResponse(
#              "This HTTP triggered function executed successfully. Pass a name in the query string or in the request body for a personalized response.",
#              status_code=200
#         )
    



# @app.route(route="HTTPTrigger")
# def HTTPTrigger(req: func.HttpRequest) -> func.HttpResponse:
#     logging.info('Python HTTP trigger function processed a request.')

#     name = req.params.get('name')
#     if not name:
#         try:
#             req_body = req.get_json()
#             logging.info(req_body)
#         except ValueError:
#             pass
#         else:
#             name = req_body.get('name')

#     if name:
#         return func.HttpResponse(f"Hello, {name}. This HTTP triggered function executed successfully.")
#     else:
#         return func.HttpResponse(
#              "This HTTP triggered function executed successfully. Pass a name in the query string or in the request body for a personalized response.",
#              status_code=200
#         )
    
#default hostkey url: https://apalafunc.azurewebsites.net/api/HTTPTrigger?code=BpICPv2lAhdb2GXSi7cZcXHZPwapM-H61Om9nOmeR-YKAzFuv086CA%3D%3D
