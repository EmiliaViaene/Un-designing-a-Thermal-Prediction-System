#!/usr/bin/python3.9

# ## DataFoundry Entity: history
# ### GOAL: Get temperature dataset for the last 3-5 days and upload to DataFoundry entity dataset

# In[2]:


import requests
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime


# ### Import csv files from DataFoundry

# In[4]:


# Get the database from the DataFoundry link
df_window = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/QWM2TVQ4OE9KRWF1UU1tRUxzbXJFS1IxM3hhMVZORnNlSlMvRmVyZUFsdz0=", low_memory=False)
df_window = df_window[(df_window.participant == "H2")] # select the correct participant

# Clean up the dataframe
df_window = df_window.drop(["Unnamed: 7", "device_id", "id", "participant", "recipient", "pp1", "pp2", "pp3", "activity", "light", "participant", "curtain"], axis='columns')

# get the left window sensor values
df_window_left = df_window.loc[df_window['sender'] == "HH2_window_1"].copy()
df_window_left["curtain_left"] = np.where(df_window_left.loc[:,"distance sensor"] > 35, 1, 0) # set curtain state (0= closed; 1 = open)
# df_window_left = df_window_left.rename(columns={"light sensor": "light_left"}, errors="raise")
df_window_left["shade_left"] = np.where(df_window_left.loc[:,"light sensor"] < 2000, 1, 0) # set shade (0= no shade; 1 = yes shade)
df_window_left = df_window_left.drop(["sender", "window", "distance sensor", "reed sensor", "light sensor"], axis='columns')
df_window_left['ts'] = pd.to_datetime(df_window_left['ts']) #Turn timestamp into datetime dtype
df_window_left = df_window_left.drop_duplicates() #Drop duplicate rows
# df_window_left.resample('10min', on='ts').last() #get one value per 10 minutes
df_window_left['hr'] = df_window_left['ts'].dt.hour
df_window_left['date'] = df_window_left['ts'].dt.date
df_window_left = df_window_left.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_window_left['ts'] = df_window_left["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_left = df_window_left.drop(["hr", "date"], axis='columns')

# # get the right window sensor values
df_window_right = df_window.loc[df_window['sender'] == "HH2_window_2"].copy()
df_window_right["curtain_right"] = np.where(df_window_right.loc[:,"distance sensor"] > 35, 1, 0) # set curtain state (0= closed; 1 = open)
# df_window_right = df_window_right.rename(columns={"light sensor": "light_right"}, errors="raise")
df_window_right["shade_right"] = np.where(df_window_right.loc[:,"light sensor"] < 2000, 1, 0) # set shade (0= no shade; 1 = yes shade)
df_window_right = df_window_right.drop(["sender", "window", "distance sensor", "reed sensor", "light sensor"], axis='columns')
df_window_right['ts'] = pd.to_datetime(df_window_right['ts']) #Turn timestamp into datetime dtype
df_window_right = df_window_right.drop_duplicates() #Drop duplicate rows
# df_window_right.resample('10min', on='ts').last() #get one value per 10 minutes
# df_window_right['ts'] = df_window_right["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_right['hr'] = df_window_right['ts'].dt.hour
df_window_right['date'] = df_window_right['ts'].dt.date
df_window_right = df_window_right.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_window_right['ts'] = df_window_right["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_right = df_window_right.drop(["hr", "date"], axis='columns')


# # get the right window sensor values
df_window_door = df_window.loc[df_window['sender'] == "HH2_windowdoor_1"].copy()
df_window_door = df_window_door.rename(columns={"reed sensor": "window_door"}, errors="raise")
df_window_door = df_window_door.drop(["sender", "light sensor", "window", "distance sensor"], axis='columns')
df_window_door['ts'] = pd.to_datetime(df_window_door['ts']) #Turn timestamp into datetime dtype
df_window_door = df_window_door.drop_duplicates() #Drop duplicate rows
# df_window_door.resample('10min', on='ts').last() #get one value per 10 minutes
df_window_door['hr'] = df_window_door['ts'].dt.hour
df_window_door['date'] = df_window_door['ts'].dt.date
df_window_door = df_window_door.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_window_door['ts'] = df_window_door["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_door = df_window_door.drop(["hr", "date"], axis='columns')


df_window_merge1 = pd.merge_asof(df_window_right.sort_values('ts'), df_window_left.sort_values('ts'), on='ts',tolerance =pd.Timedelta('120 min'))
df_window = pd.merge_asof(df_window_door.sort_values('ts'), df_window_merge1.sort_values('ts'), on='ts',tolerance =pd.Timedelta('120 min'))
df_window = df_window.drop_duplicates(subset=['ts']) #Drop duplicate rows
df_window.tail()


# In[5]:


# Get the database from the DataFoundry link
df_door = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/VmF4cU95cm1KZ3Z4TjlnV1lnNEZhSU1HVmNsc3RoeXRyanU3UGJtbzNOZz0=")
df_door = df_door[(df_door.participant == "H2")] # select the correct participant

# Clean up the dataframe
df_door = df_door.drop(["window", "Unnamed: 7", "device_id", "activity", "participant", "sender", "id", "recipient", "pp1", "pp2", "pp3"], axis='columns')
df_door = df_door.rename(columns={"reed sensor": "door_indoors"}, errors="raise")
df_door['ts'] = pd.to_datetime(df_door['ts']) ## Turn timestamp into datetime dtype
df_door = df_door.dropna() ## drop rows with empty (NA) cells
df_door = df_door.drop_duplicates() ## Drop duplicate rows
df_door['hr'] = df_door['ts'].dt.hour
df_door['date'] = df_door['ts'].dt.date
df_door = df_door.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
# df_door = df_door.groupby(['date', 'hr']).agg(pd.Series.mode).reset_index()
df_door['ts'] = df_door["ts"].dt.round('h')  #Round the datestamp column to hours
df_door = df_door.drop(["hr", "date"], axis='columns')
df_door.tail()


# In[6]:


# Get the database from the DataFoundry link
df_indoor = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/Yy9PQlp3clNidmNyc0pYS1JBV1NlQ1JpbGNKWHBIQlVVMjlwQW9nOFY5UT0=")
df_indoor = df_indoor[(df_indoor.participant == "H2")] # select the correct participant

# Clean up the dataframe
df_indoor = df_indoor.drop(["Unnamed: 7", "device_id", "activity", "participant", "sender", "id", "recipient", "pp1", "pp2", "pp3"], axis='columns')
df_indoor = df_indoor.rename(columns={"Temperature": "Temperature_indoor"}, errors="raise")
df_indoor['ts'] = pd.to_datetime(df_indoor['ts']) ## Turn timestamp into datetime dtype
df_indoor['Temperature_indoor'] = np.round(df_indoor['Temperature_indoor'] * 10) / 10 ## round temperature to 1 decimal
df_indoor = df_indoor.dropna() ## drop rows with empty (NA) cells
df_indoor = df_indoor.drop_duplicates() ## Drop duplicate rowsindex_list= df_indoor2.Timestamp[(df_indoor2.Timestamp >= "2024-08-08 16:00:00") & (df_indoor2.Timestamp <= "2024-08-08 19:20:00")].index.tolist(
df_indoor['hr'] = df_indoor['ts'].dt.hour
df_indoor['date'] = df_indoor['ts'].dt.date
df_indoor = df_indoor.groupby(['date', 'hr']).first().reset_index()
df_indoor['ts'] = df_indoor["ts"].dt.round('h')  #Round the datestamp column to hours
df_indoor = df_indoor.drop(["hr", "date"], axis='columns')


# In[7]:


# Get the database from the DataFoundry link
df_API = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/N0FsN2loZlVYMWhBaE0rd0l5T2NadzR3YTNBcnovQlJ0SG13dHMxL0U1RT0=")
df_API = df_API[(df_API.participant == "H2")] # select the correct participant

# Clean up the dataframe
df_API = df_API.drop(["activity", "device_id", "sender", "participant", "Unnamed: 7", "id", "recipient", "pp1", "pp2", "pp3", "Temperature_API_MAX", "Temperature_API_MIN", "weather_description", "weather_main"], axis='columns')
df_API['ts'] = pd.to_datetime(df_API['ts']) ## Turn timestamp into datetime dtype
df_API['Temperature_API'] = np.round(df_API['Temperature_API'] * 10) / 10 ## round temperature to 1 decimal
df_API['hr'] = df_API['ts'].dt.hour
df_API['date'] = df_API['ts'].dt.date
df_API = df_API.groupby(['date', 'hr']).first().reset_index()
df_API['ts'] = df_API["ts"].dt.round('h')  #Round the datestamp column to hours
df_API = df_API.drop(["hr", "date"], axis='columns')


# ### Export json to entity dataset in DataFoundry (last5days)

# In[56]:


# Merge the datasets on the timestamp
df_merge = pd.merge_asof(df_indoor.sort_values('ts'), df_API.sort_values('ts'), on='ts',tolerance =pd.Timedelta('120 min'))
df_merge = pd.merge_asof(df_merge.sort_values('ts'), df_window.sort_values('ts'), on='ts',tolerance =pd.Timedelta('120 min'))
df_merge = pd.merge_asof(df_merge.sort_values('ts'), df_door.sort_values('ts'), on='ts',tolerance =pd.Timedelta('120 min'))
df_merge['date'] = df_merge['ts'].dt.strftime('%d/%m') # get a column with only the day and month (lvgl labeling purposes)


## get difference between today midnight and the timestamp for each row
today = datetime.now()
today_morning = today.strftime('%Y-%m-%d') + "T00:00:00"
df_merge["time_since_today"] = today_morning
df_merge['time_since_today'] = pd.to_datetime(df_merge['time_since_today']) ## Turn timestamp into datetime dtype
df_merge['time_since_today'] = (df_merge.ts - df_merge.time_since_today) / pd.Timedelta(hours=1)
df_merge = df_merge.drop_duplicates(subset=["ts"]) ## Drop duplicate rows

# get rows for the last three days only, and reset the index
df_merge = df_merge[(df_merge.time_since_today > -73) & (df_merge.time_since_today < 0)]
df_merge = df_merge.reset_index(drop=True)

# Get the min and max values for 'Temperature_indoor' and 'Temperature_API' to set the range in the lvgl chart
minvalue = df_merge[["Temperature_indoor", "Temperature_API"]].min().min()  # Overall minimum value
maxvalue = df_merge[["Temperature_indoor", "Temperature_API"]].max().max()  # Overall maximum value

# Assign the min and max values to new columns 'pp1' and 'pp2'
df_merge["pp1"] = minvalue
df_merge["pp2"] = maxvalue

# Convert specified columns to strings, as only strings can be passed as JSON parameters
columns_to_convert = ['pp1', 'pp2', 'curtain_left', 'door_indoors', 
                      'shade_left', 'window_door', 'curtain_right', 'shade_right', 
                      'time_since_today', 'Temperature_API', 'Temperature_indoor']
df_merge[columns_to_convert] = df_merge[columns_to_convert].astype('object')
df_merge['ts'] = df_merge['ts'].map(str)
df_merge['date'] = df_merge['date'].astype('object')

# Reverse the dataframe so that the JSON will eventually be sent in the right order
df_merge = df_merge.reindex(index=df_merge.index[::-1]) # reverse the dataframe so that the JSON will eventually be sent in the right order
#get a string id per row to pass as unique resource_id
df_merge.reset_index(inplace=True, drop=True)
df_merge["id"] = df_merge.index + 1
df_merge['id'] = df_merge['id'].map(str)

# Get rows for the last three days only, and reset the index for each subset
df_yesterday = df_merge[(df_merge.time_since_today > -25) & (df_merge.time_since_today < 0)]
df_yesterday = df_yesterday.reset_index(drop=True)  # Reset index for yesterday's data
# df_merge["id"] = df_merge.index + 1
# df_merge['id'] = df_merge['id'].map(str)

df_twodaysago = df_merge[(df_merge.time_since_today > -49) & (df_merge.time_since_today < -24)]
df_twodaysago = df_twodaysago.reset_index(drop=True)  # Reset index for two days ago's data

df_threedaysago = df_merge[(df_merge.time_since_today > -73) & (df_merge.time_since_today < -48)]
df_threedaysago = df_threedaysago.reset_index(drop=True)  # Reset index for three days ago's data

df_yesterday.head()

# today_df = df_merge[(df_merge.time_since_today > 0) & (df_merge.time_since_today < 24)]
# today_df = today_df.reset_index(drop=True)  # Reset index for two days ago's data
# today_df.head()


# In[58]:


def update_nan_times(df):
    """
    Update the 'NaN_times' column in the given DataFrame to indicate the indexes of NaN values
    in specific columns and fill NaN values with 0.
    
    Parameters:
    df (DataFrame): The DataFrame to be processed.
    """
    df["NaN_times"] = "0"  # Create a column to store the notifications for NaN values
    column_index = 0  # Initialize the index for updating the NaN_times column

    # Iterate over the relevant columns to check for NaN values
    for column in df[['window_door', 'curtain_left', 'curtain_right', 'shade_left', 'shade_right', 'door_indoors']]:
        na_values = df[column].isna().tolist()  # Create a list indicating where NaN values are present
        na_values = np.flip(na_values)
        na_values = [i for i, n in enumerate(na_values) if n == True]  # List all indexes where NaN is True
        # Check if there are any NaN values found
        if any(na_values) == True:
            # If NaN values are found, create a string indicating the first and last index of NaNs
            na_values = [na_values[0]] + [na_values[-1]]  # Get the first and last index of NaN values
            na_values = 'na: ' + '-'.join(str(x) for x in na_values)  # Format the output string
        else:
            na_values = ""  # If no NaN values, set to an empty string

        # Update the NaN_times column with the formatted NaN information for the current column
        df.loc[column_index, 'NaN_times'] = na_values  
        column_index += 1  # Increment the index for the next iteration

    # Fill NaN values with 0 (since they will mess up the JSON output)
    df.fillna("0", inplace=True)

# Now apply the function to the three DataFrames
update_nan_times(df_yesterday)
update_nan_times(df_twodaysago)
update_nan_times(df_threedaysago)

df_twodaysago.head()


# In[64]:


def post_data_to_api(df, url, api_token):
    """
    Function to post data to the specified API endpoint using the provided dataframe.
    
    Parameters:
    df (DataFrame): The dataframe containing the data to be sent.
    url (str): The API endpoint URL.
    api_token (str): The API token for authentication.
    """
    # Loop through the dataframe to add rows to the DF dataset as JSON
    for ind in df.index:
        # Create a dictionary for headers to be sent to the API
        HEADERS = {
            'api_token': api_token,  # API token for authentication
            'resource_id': df["id"].iloc[ind],  # Resource ID from the dataframe
            'token': 'token_for_identifier'  # Token for additional authentication
        }
        
        # Extract parameters from the dataframe for the current index
        ts = df['ts'].iloc[ind]
        temp_out = df['Temperature_API'].iloc[ind]
        temp_in = df['Temperature_indoor'].iloc[ind]
        timesince = df['time_since_today'].iloc[ind]
        window_door = df['window_door'].iloc[ind]
        curtain_left = df['curtain_left'].iloc[ind]
        shade_left = df['shade_left'].iloc[ind]
        curtain_right = df['curtain_right'].iloc[ind]
        shade_right = df['shade_right'].iloc[ind]
        pp1 = df['pp1'].iloc[ind]
        pp2 = df['pp2'].iloc[ind]
        door_indoors = df['door_indoors'].iloc[ind]
        date_yesterday = df_yesterday['date'].iloc[ind]
        date_twodaysago = df_twodaysago['date'].iloc[0]
        date_threedaysago = df_threedaysago['date'].iloc[0]
        NaN_times = df['NaN_times'].iloc[ind]
        
        # Prepare the parameters to be sent in the POST request
        PARAMS = {
            "pp1": pp1, 
            "pp2": pp2, 
            "door_indoors": door_indoors, 
            "window_door": window_door, 
            "curtain_left": curtain_left, 
            "shade_left": shade_left, 
            "curtain_right": curtain_right, 
            "shade_right": shade_right, 
            "Temperature_outdoor": temp_out, 
            "Temperature_indoor": temp_in, 
            "time_since_today": timesince, 
            "ts": ts,
            "date_yesterday": date_yesterday,
            "date_twodaysago": date_twodaysago,
            "date_threedaysago": date_threedaysago,
            "NaN_times": NaN_times
        }    
        
        # Send a POST request to the API with the headers and parameters
        r = requests.post(url=url, headers=HEADERS, json=PARAMS)

url_twodaysago = "https://data.id.tue.nl/datasets/entity/11744/item/"
api_token_twodaysago = "NlBRYlJVWlRDS09LbHlDUlJpNXVqbjVuV1ZuV0hQSmNlemoxQkZsdXFBRT0="  

url_threedaysago = "https://data.id.tue.nl/datasets/entity/11746/item/"
api_token_threedaysago = "d1QwS3ZWSjBnR3U1RVREK3JtOEZLa0hkMitEY25GV3FBM3VkaEZ5Rm9uaz0="  

url_yesterday = "https://data.id.tue.nl/datasets/entity/11745/item/"
api_token_yesterday = "UzBNRUlmcE13ckJmRS9aSGxDdCszaXh2YTdrL0Q2QWhiUTFNMWhrd3g5bz0="  

# Call the function for each dataframe
post_data_to_api(df_twodaysago, url_twodaysago, api_token_twodaysago)
post_data_to_api(df_threedaysago, url_threedaysago, api_token_threedaysago)
post_data_to_api(df_yesterday, url_yesterday, api_token_yesterday)

