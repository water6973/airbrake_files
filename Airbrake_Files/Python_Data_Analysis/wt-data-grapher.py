# import numpy as np
# import pandas as pd
# import matplotlib.pyplot as plt

# filename = '/run/media/gavin/0403-0201/wt-c-2.csv' # change this to be compatible with your own system
# graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/wt-c-2.png' # this too

# df = pd.read_csv(filename, names=['Time', 'Altitude'])

# # Plotting the results
# plt.figure(figsize=(10, 6))
# plt.plot(df['Time'], df['Altitude'], label='Altitude')

# plt.xlim(220000, 275000)

# plt.xlabel('Time (Milliseconds)')
# plt.ylabel('Altitude (Meters)')
# plt.title('Flight Data Graph')
# plt.legend()

# # Save the plot to a file
# plt.savefig(graph_filename)

# print(f"Graph saved as {graph_filename}")

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# File paths
filename1 = '/run/media/gavin/0403-0201/wt-e-4.csv' 
filename2 = '/run/media/gavin/0403-0201/wt-c-2.csv' 
graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/wt-c-2-4.png'  

# Read in the data
df1 = pd.read_csv(filename1, names=['Time', 'Altitude'])
df2 = pd.read_csv(filename2, names=['Time', 'Altitude'])

# Filter the datasets for the desired time intervals
start_time1, end_time1 = 180000, 220000
start_time2, end_time2 = 220000, 275000  

df1_filtered = df1[(df1['Time'] >= start_time1) & (df1['Time'] <= end_time1)]
df2_filtered = df2[(df2['Time'] >= start_time2) & (df2['Time'] <= end_time2)]

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(df1_filtered['Time'], df1_filtered['Altitude'], label='Test 1')
plt.plot(df2_filtered['Time'], df2_filtered['Altitude'], label='Test 2', alpha=1)  

plt.xlabel('Time (Milliseconds)')
plt.ylabel('Altitude (Meters)')
plt.title('Flight Data Overlay Graph')
plt.legend()

# Save the plot to a file
plt.savefig(graph_filename)

print(f"Graph saved as {graph_filename}")