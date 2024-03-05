import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

actual_apogee = 233  # replace with actual apogee of flight in m
k = 0.0016
mass = 0.641

def calculate_projected_apogee(altitude, velocity):
    return altitude + (mass / (2 * k)) * np.log((k * velocity**2) / (mass * 9.8) + 1)

filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test7.csv'
graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/test7.png'

df = pd.read_csv(filename, names=['Time', 'Altitude', 'Velocity', 'isDeployed', 'isBurning', 'Projected Apogee'])

df.loc[df['Time'] <= 1.5, 'Velocity'] = np.nan
df['Error'] = abs(actual_apogee - calculate_projected_apogee(df['Altitude'], df['Velocity']))

# Add Adjusted Velocity column
df['Adjusted Velocity'] = df['Velocity'].copy()  # Initialize with original velocity
for i in range(1, len(df)):
    prev_velocity = df.at[i - 1, 'Adjusted Velocity']
    current_velocity = df.at[i, 'Velocity']
    if current_velocity > prev_velocity:
        df.at[i, 'Adjusted Velocity'] = min(prev_velocity + 1, current_velocity)
    elif current_velocity < prev_velocity:
        df.at[i, 'Adjusted Velocity'] = max(prev_velocity - 1, current_velocity)

# Calculate projected apogee using the provided equation
df['Projected Apogee'] = calculate_projected_apogee(df['Altitude'], df['Adjusted Velocity'])

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(df['Time'], df['Altitude'], label='Altitude')
plt.plot(df['Time'], df['Adjusted Velocity'], label='Adjusted Velocity')
plt.plot(df['Time'], df['Projected Apogee'], label='Projected Apogee')
plt.axhline(y=actual_apogee, color='r', linestyle='--', label='Actual Apogee')
plt.axvline(x=1500, color='r', linestyle='--', label='Motor Cutoff (1.5s)')

deploy_changes = df[df['isDeployed'].diff() != 0]
for time in deploy_changes['Time']:
    plt.axvline(x=time, color='k', linestyle='--', alpha=0.5)

plt.ylim(0, 400)
plt.xlim(0, 10000)

plt.xlabel('Time (Milliseconds)')
plt.ylabel('Altitude (Meters)')
plt.title('Flight Data Graph')
plt.legend()

# Save the plot to a file
plt.savefig(graph_filename)

print(f"Graph saved as {graph_filename}")
