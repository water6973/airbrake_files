import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

PNUT = 0
SD = 1

actual_apogee = 215.8  # replace with actual apogee of flight in m
target_apogee = 250

datatype = SD
filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test12.csv'
graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/test12.png'

if datatype == SD:
    df = pd.read_csv(filename, names=['Time', 'Altitude', 'Velocity', 'isDeployed', 'isBurning', 'Projected Apogee'])
    df.loc[df['Time'] <= 1.5, 'Projected Apogee'] = np.nan
    df['Error'] = abs(actual_apogee - df['Projected Apogee'])

    plt.figure(figsize=(10, 6))
    # Initialize lists to store x and y coordinates of segments
    x_segments = []
    y_segments = []
    current_color = 'green'  # Initial color
    for index, row in df.iterrows():
        x_segments.append(row['Time'])
        y_segments.append(row['Projected Apogee'])
        if index < len(df) - 1 and row['isDeployed'] != df.loc[index + 1, 'isDeployed']:
            plt.plot(x_segments, y_segments, color=current_color, label='')
            x_segments = []
            y_segments = []
            current_color = 'green' if row['isDeployed'] else 'red'

    # Plot the last segment
    if x_segments:
        plt.plot(x_segments, y_segments, color=current_color, label='Projected Apogee' if current_color == 'green' else '')

    # Plot other data
    plt.plot(df['Time'], df['Velocity'], label='Velocity')
    plt.plot(df['Time'], df['Altitude'], label='Altitude')
    plt.axhline(y=actual_apogee, color='g', linestyle='--', label='Actual Apogee')
    plt.axhline(y=target_apogee, color='r', linestyle='--', label='Target Apogee')
    plt.axvline(x=1.5, color='r', linestyle='--', label='Motor Cutoff (1.5s)')

    # deploy_changes = df[df['isDeployed'].diff() != 0]
    # for time in deploy_changes['Time']:
    #     plt.axvline(x=time, color='k', linestyle='--', alpha=0.5)

elif datatype == PNUT:
    df = pd.read_csv(filename, names=['Time', 'Altitude', 'Velocity', 'Temperature', 'Voltage'])
    plt.figure(figsize=(10, 6))
    plt.plot(df['Time'], df['Altitude'], label='Altitude')
    plt.plot(df['Time'], df['Velocity'], label='Velocity')
    plt.axvline(x=1.5, color='r', linestyle='--', label='Motor Cutoff (1.5s)')

plt.ylim(0, 400)
plt.xlim(0, 10000)

plt.xlabel('Time (Milliseconds)')
plt.ylabel('Altitude (Meters)')
plt.title('Flight Data Graph')
# plt.legend()

# Save the plot to a file
plt.savefig(graph_filename)

print(f"Graph saved as {graph_filename}")