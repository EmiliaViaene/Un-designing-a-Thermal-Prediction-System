#!/usr/bin/python3.9

# # Forecast: sun strenth (gr_w: in W/m2)

# In[1]:


import requests
import pandas as pd
import numpy as np
from datetime import datetime


# In[2]:

# api-endpoint
URL = "https://data.meteoserver.nl/api/uurverwachting.php?lat=51.319809&long=4.856430&key=1ad6efe54c"

# sending get request and saving the response as response object
r = requests.get(url = URL)
 
# extracting data in json format
data = r.json()

# Flatten (normalize) the JSON file
forecast = pd.json_normalize(data,record_path=['data']) # for the main data (list)
# weather = pd.json_normalize(data,record_path=['list', 'weather']) # get the weather descriptions in a separate df

# combine list and weather df and drop unnecessary columns (+transform dt column to pd datetime)
# forecast = pd.concat([data], axis=1)
forecast = forecast.drop(["cape", "loc", "winds", "windb", "windknp", "windkmh", "vis", "neersl", "luchtd", "luchtdmmhg", "luchtdinhg", "hw", "mw", "lw", "snd", "tw", "snv", "icoon", "ico", "cond"], axis='columns')
forecast = forecast.rename(columns={"tijd": "Timestamp"}, errors="raise")
# forecast['Timestamp'] = pd.to_datetime(forecast['Timestamp']) ## Turn timestamp into datetime dtype
# forecast = forecast.round(1)  ## round temperature to ... decimal

# ### Export json to entity dataset in DataFoundry (last5days)

# In[5]:


from oocsi import OOCSI
import time

# connect to OOCSI running on the local machine ('localhost')
oocsi = OOCSI('forecast_sun', 'oocsi.id.tue.nl')

for ind in forecast.index:
    # PARAMS: iter for each column I want the values to go to the entity dataset
    Timestamp = forecast['Timestamp'].iloc[ind]
    tijd_nl = forecast['tijd_nl'].iloc[ind]
    offset = forecast['offset'].iloc[ind]    
    temp = forecast['temp'].iloc[ind]
    windr = forecast['windr'].iloc[ind]
    windrltr = forecast['windrltr'].iloc[ind]
    rv = forecast['rv'].iloc[ind]
    gr = forecast['gr'].iloc[ind]
    gr_w = forecast['gr_w'].iloc[ind]
    samenv = forecast['samenv'].iloc[ind]
    oocsi.send('api_solar',{ "Timestamp": Timestamp, "tijd_nl": tijd_nl, "offset": offset, "temp": temp, "windr": windr, "windrltr": windrltr, "rv": rv, "gr": gr, "gr_w": gr_w, "samenv": samenv})   
    time.sleep(1)

oocsi.stop()