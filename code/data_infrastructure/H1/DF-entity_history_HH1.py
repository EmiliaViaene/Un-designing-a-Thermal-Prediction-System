#!/usr/bin/python3.9

# ## DataFoundry Entity: history
# ### GOAL: Get temperature dataset for the last 3-5 days and upload to DataFoundry entity dataset

# In[7]:


import requests
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from datetime import datetime, timedelta


# ### Import csv files from DataFoundry

# In[45]:


# Get the database from the DataFoundry link
df_window = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/QWM2TVQ4OE9KRWF1UU1tRUxzbXJFS1IxM3hhMVZORnNlSlMvRmVyZUFsdz0=", low_memory=False)
df_window = df_window[(df_window.participant == "H1")] # select the correct participant

# Clean up the dataframe
df_window = df_window.drop(["Unnamed: 7", "device_id", "id", "participant", "recipient", "pp1", "pp2", "pp3", "activity", "light", "participant", "curtain"], axis='columns')

# get the left window sensor values (voor = oost)
df_window_voor = df_window.loc[df_window['sender'] == "HH1_window_1"].copy()
df_window_voor["curtain_voor"] = np.where(df_window_voor.loc[:,"distance sensor"] > 35, 1, 0) # set curtain state (0= closed; 1 = open)
# df_window_voor = df_window_voor.rename(columns={"light sensor": "light_left"}, errors="raise")
df_window_voor["shade_voor"] = np.where(df_window_voor.loc[:,"light sensor"] < 2000, 1, 0) # set shade (0= no shade; 1 = yes shade)
df_window_voor = df_window_voor.drop(["sender", "window", "distance sensor", "reed sensor", "light sensor"], axis='columns')
df_window_voor['ts'] = pd.to_datetime(df_window_voor['ts']) #Turn timestamp into datetime dtype
df_window_voor = df_window_voor.drop_duplicates() #Drop duplicate rows
# df_window_voor.resample('10min', on='ts').last() #get one value per 10 minutes
df_window_voor['hr'] = df_window_voor['ts'].dt.hour
df_window_voor['date'] = df_window_voor['ts'].dt.date
df_window_voor = df_window_voor.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_window_voor['ts'] = df_window_voor["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_voor = df_window_voor.drop(["hr", "date"], axis='columns')

# # get the right window sensor values (achter = west)
df_window_achter = df_window.loc[df_window['sender'] == "HH1_window_2"].copy()
df_window_achter["curtain_achter"] = np.where(df_window_achter.loc[:,"distance sensor"] > 35, 1, 0) # set curtain state (0= closed; 1 = open)
# df_window_achter = df_window_achter.rename(columns={"light sensor": "light_right"}, errors="raise")
df_window_achter["shade_achter"] = np.where(df_window_achter.loc[:,"light sensor"] < 2000, 1, 0) # set shade (0= no shade; 1 = yes shade)
df_window_achter = df_window_achter.drop(["sender", "window", "distance sensor", "reed sensor", "light sensor"], axis='columns')
df_window_achter['ts'] = pd.to_datetime(df_window_achter['ts']) #Turn timestamp into datetime dtype
df_window_achter = df_window_achter.drop_duplicates() #Drop duplicate rows
# df_window_achter.resample('10min', on='ts').last() #get one value per 10 minutes
# df_window_achter['ts'] = df_window_achter["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_achter['hr'] = df_window_achter['ts'].dt.hour
df_window_achter['date'] = df_window_achter['ts'].dt.date
df_window_achter = df_window_achter.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_window_achter['ts'] = df_window_achter["ts"].dt.round('h')  #Round the datestamp column to hours
df_window_achter = df_window_achter.drop(["hr", "date"], axis='columns')


# # get the right window sensor values
df_windowdoor_voor = df_window.loc[df_window['sender'] == "HH1_windowdoor_3"].copy()
df_windowdoor_voor = df_windowdoor_voor.rename(columns={"reed sensor": "windowdoor_voor"}, errors="raise")
df_windowdoor_voor = df_windowdoor_voor.drop(["sender", "light sensor", "window", "distance sensor"], axis='columns')
df_windowdoor_voor['ts'] = pd.to_datetime(df_windowdoor_voor['ts']) #Turn timestamp into datetime dtype
df_windowdoor_voor = df_windowdoor_voor.drop_duplicates() #Drop duplicate rows
# df_windowdoor_voor.resample('10min', on='ts').last() #get one value per 10 minutes
df_windowdoor_voor['hr'] = df_windowdoor_voor['ts'].dt.hour
df_windowdoor_voor['date'] = df_windowdoor_voor['ts'].dt.date
df_windowdoor_voor = df_windowdoor_voor.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_windowdoor_voor['ts'] = df_windowdoor_voor["ts"].dt.round('h')  #Round the datestamp column to hours
df_windowdoor_voor = df_windowdoor_voor.drop(["hr", "date"], axis='columns')

# # get the right window sensor values
df_windowdoor_achter = df_window.loc[df_window['sender'] == "HH1_windowdoor_4"].copy()
df_windowdoor_achter = df_windowdoor_achter.rename(columns={"reed sensor": "windowdoor_achter"}, errors="raise")
df_windowdoor_achter = df_windowdoor_achter.drop(["sender", "light sensor", "window", "distance sensor"], axis='columns')
df_windowdoor_achter['ts'] = pd.to_datetime(df_windowdoor_achter['ts']) #Turn timestamp into datetime dtype
df_windowdoor_achter = df_windowdoor_achter.drop_duplicates() #Drop duplicate rows
# df_windowdoor_achter.resample('10min', on='ts').last() #get one value per 10 minutes
df_windowdoor_achter['hr'] = df_windowdoor_achter['ts'].dt.hour
df_windowdoor_achter['date'] = df_windowdoor_achter['ts'].dt.date
df_windowdoor_achter = df_windowdoor_achter.groupby(['date', 'hr']).agg(lambda x: x.mode().max()).reset_index() #get the mode for each hour since its possible they only opened it for a few mins at the beginning of the hour
df_windowdoor_achter['ts'] = df_windowdoor_achter["ts"].dt.round('h')  #Round the datestamp column to hours
df_windowdoor_achter = df_windowdoor_achter.drop(["hr", "date"], axis='columns')

#### Create a df with a timestamp for each hour of the last three days -- merge the dataframes with this df (makes sure there's values for each hour + limits the df to the last 72 hours)
# Get the current time
now = datetime.now()
today = now.strftime('%Y-%m-%d') + "T00:00:00"
# Create a date range for the last 24 hours with hourly frequency
last_day_range = pd.date_range(end=today, periods=73, freq='H')
# Create the DataFrame
last_three_days = pd.DataFrame(last_day_range, columns=['ts'])

### merge the window dataframes on the timestamps for the last 72 hours
from functools import reduce
# Create a list of DataFrames to merge
dataframes_to_merge = [
    last_three_days,
    df_window_voor,
    df_window_achter,
    df_windowdoor_voor,
    df_windowdoor_achter
]

# Function to merge two DataFrames
def merge_asof(df1, df2):
    return pd.merge_asof(df1.sort_values('ts'), df2.sort_values('ts'), on='ts', tolerance=pd.Timedelta('120 min'))

# Use reduce to merge all DataFrames in one go
df_window = reduce(merge_asof, dataframes_to_merge)

# df_window = df_window.drop_duplicates(subset=['ts']) #Drop duplicate rows


# In[51]:


# Get the database from the DataFoundry link
df_indoor = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/Yy9PQlp3clNidmNyc0pYS1JBV1NlQ1JpbGNKWHBIQlVVMjlwQW9nOFY5UT0=")
df_indoor = df_indoor[(df_indoor.participant == "H1")] # select the correct participant

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


# In[52]:


# Get the database from the DataFoundry link
df_API = pd.read_csv (r"https://data.id.tue.nl/datasets/downloadPublic/N0FsN2loZlVYMWhBaE0rd0l5T2NadzR3YTNBcnovQlJ0SG13dHMxL0U1RT0=")
df_API = df_API[(df_API.participant == "H1")] # select the correct participant

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

# In[153]:


# Create a list of DataFrames to merge
dataframes_to_merge = [
    df_window,
    df_indoor,
    df_API
]
# Function to merge two DataFrames
def merge_asof(df1, df2):
    return pd.merge_asof(df1.sort_values('ts'), df2.sort_values('ts'), on='ts', tolerance=pd.Timedelta('120 min'))
# Use reduce to merge all DataFrames in one go
df_merge = reduce(merge_asof, dataframes_to_merge)
# get a column with only the day and month (lvgl labeling purposes)
df_merge['date'] = df_merge['ts'].dt.strftime('%d/%m') 

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
columns_to_convert = ['pp1', 'pp2', 'curtain_voor','shade_voor', 'windowdoor_voor', "windowdoor_achter", 'curtain_achter', 'shade_achter', 
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

# today_df = df_merge[(df_merge.time_since_today > 0) & (df_merge.time_since_today < 24)]
# today_df = today_df.reset_index(drop=True)  # Reset index for two days ago's data
# today_df.head()
df_threedaysago.head(3)


# In[155]:


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
    for column in df[['windowdoor_voor', 'windowdoor_achter', 'curtain_voor', 'curtain_achter', 'shade_voor', 'shade_achter']]:
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

# Make sure again that all columns are of object dtype
columns_to_convert = ['ts', 'date', 'pp1', 'pp2', 'curtain_voor','shade_voor', 'windowdoor_voor', "windowdoor_achter", 'curtain_achter', 'shade_achter', 
                      'time_since_today', 'Temperature_API', 'Temperature_indoor']
df_yesterday[columns_to_convert] = df_yesterday[columns_to_convert].astype('object')
df_twodaysago[columns_to_convert] = df_twodaysago[columns_to_convert].astype('object')
df_threedaysago[columns_to_convert] = df_threedaysago[columns_to_convert].astype('object')


# In[157]:


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
        windowdoor_voor = df['windowdoor_voor'].iloc[ind]
        windowdoor_achter = df['windowdoor_achter'].iloc[ind]        
        curtain_voor = df['curtain_voor'].iloc[ind]
        shade_voor = df['shade_voor'].iloc[ind]
        curtain_achter = df['curtain_achter'].iloc[ind]
        shade_achter = df['shade_achter'].iloc[ind]
        pp1 = df['pp1'].iloc[ind]
        pp2 = df['pp2'].iloc[ind]
        date_yesterday = df_yesterday['date'].iloc[ind]
        date_twodaysago = df_twodaysago['date'].iloc[0]
        date_threedaysago = df_threedaysago['date'].iloc[0]
        NaN_times = df['NaN_times'].iloc[ind]
        
        # Prepare the parameters to be sent in the POST request
        PARAMS = {
            "pp1": pp1, 
            "pp2": pp2, 
            "windowdoor_voor": windowdoor_voor, 
            "windowdoor_achter": windowdoor_achter, 
            "curtain_voor": curtain_voor, 
            "shade_voor": shade_voor, 
            "curtain_achter": curtain_achter, 
            "shade_achter": shade_achter, 
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

url_yesterday = "https://data.id.tue.nl/datasets/entity/11747/item/"
api_token_yesterday = "RHlYR1V5VUZ1UlczNjhHdnZGelZ2b3U4OER3eDQreFdidWcyVmE5cGFJND0=" 

url_twodaysago = "https://data.id.tue.nl/datasets/entity/11787/item/"
api_token_twodaysago = "dEswYzVyWW9XVFZya2ljL1RZU1p2MDM2QW5sdWN1SURqc0MvV2t3d3kwZz0="  

url_threedaysago = "https://data.id.tue.nl/datasets/entity/11788/item/"
api_token_threedaysago = "UXdRakZkYUh6TlNrVnlxZzJ6NVVscC9GMVkydW94b2tuZWwvbUVrLzlxYz0="  

# Call the function for each dataframe
post_data_to_api(df_yesterday, url_yesterday, api_token_yesterday)
post_data_to_api(df_twodaysago, url_twodaysago, api_token_twodaysago)
post_data_to_api(df_threedaysago, url_threedaysago, api_token_threedaysago)

