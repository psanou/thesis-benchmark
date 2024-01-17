import matplotlib.pyplot as plt
import pandas as pd
from pymeos import *
import numpy as np

pymeos_initialize()
SIZE = "medium"
PATH='../datasets/ships_'+SIZE+'.csv'
df = pd.read_csv(PATH)
df = df.dropna()
df["Timestamp"] = pd.to_datetime(df["Timestamp"],format='mixed')
df = df.drop_duplicates(["Timestamp"])
df = df.sort_values(by="Timestamp")



df['point'] = df.apply(lambda row: TGeogPointInst(point=(row['Latitude'], row['Longitude']), timestamp=row['Timestamp']),axis=1)
df['SOG'] = df.apply(lambda row: TFloatInst(value=(not np.isnan(row['SOG']) and row['SOG']) or 0 , timestamp=row['Timestamp']), axis=1)
df['ROT'] = df.apply(lambda row: TFloatInst(value=(not np.isnan(row['ROT']) and row['ROT']) or 0, timestamp=row['Timestamp']), axis=1)
df['Heading'] = df.apply(lambda row: TFloatInst(value=(not np.isnan(row['Heading']) and row['Heading']) or 0, timestamp=row['Timestamp']), axis=1)
# df['Navigational status'] = df.apply(lambda row: TTextInst(value=row['Navigational status'] or "", timestamp=row['Timestamp']), axis=1)
# df['Name'] = df.apply(lambda row: TTextInst(value=row['Name'] or "", timestamp=row['Timestamp']), axis=1)
df.drop(['Latitude', 'Longitude'], axis=1, inplace=True)
df.head()
trajectories = df.groupby('id').aggregate(
    {
        'point': lambda x: TGeogPointSeq.from_instants(x, lower_inc=True,upper_inc=True),
        'SOG':  lambda x: TFloatSeq.from_instants(x, lower_inc=True,upper_inc=True),
        'ROT': lambda x: TFloatSeq.from_instants(x, lower_inc=True,upper_inc=True),
        'Heading': lambda x: TFloatSeq.from_instants(x, lower_inc=True,upper_inc=True)
    }
).rename({'point': 'trajectory'}, axis=1)
print(trajectories["SOG"])
#trajectories["trajectory"].values[0].plot()

#trajectories['distance'] = trajectories['trajectory'].apply(lambda t: t.length())
pymeos_finalize()
