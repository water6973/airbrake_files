import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

actual_apogee = 233  # replace with actual apogee of flight in m
target_apogee = 250  # replace with actual apogee of flight in m
k = 0.026 # change this (basically adjusting drag coefficient, the higher it is the lower projected apogee will be)
mass = 0.607

def calculate_projected_apogee(altitude, velocity):
    return altitude + (mass / (2 * k)) * np.log((k * velocity**2) / (mass * 9.8) + 1)

filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test7.csv' # change this to be compatible with your own system
graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/test7.png' # this too

df = pd.read_csv(filename, names=['Time', 'Altitude', 'Velocity', 'isDeployed', 'isBurning', 'Projected Apogee'])

df.loc[df['Time'] <= 1.5, 'Velocity'] = np.nan
df['Error'] = abs(actual_apogee - calculate_projected_apogee(df['Altitude'], df['Velocity']))

df['Adjusted Velocity'] = df['Velocity'].copy() 
for i in range(1, len(df)):
    if (df.at[i, 'Time'] > 2000 and df.at[i, 'Time'] < 4500): # change these values (sets the interval of time where velocity change clamp is applied)
        prev_velocity = df.at[i - 1, 'Adjusted Velocity']
        current_velocity = df.at[i, 'Velocity']
        if current_velocity > prev_velocity:
            df.at[i, 'Adjusted Velocity'] = min(prev_velocity + 5, current_velocity) # sets velocity change clamp in the positive direction
        elif current_velocity < prev_velocity:
            df.at[i, 'Adjusted Velocity'] = max(prev_velocity - 5, current_velocity) # sets velocity change clamp in the negative direction


# Calculate projected apogee using the provided equation
df['Projected Apogee'] = calculate_projected_apogee(df['Altitude'], df['Adjusted Velocity'])

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(df['Time'], df['Altitude'], label='Altitude')
plt.plot(df['Time'], df['Adjusted Velocity'], label='Adjusted Velocity')
plt.plot(df['Time'], df['Projected Apogee'], label='Projected Apogee')
plt.axhline(y=actual_apogee, color='g', linestyle='--', label='Actual Apogee')
plt.axhline(y=target_apogee, color='r', linestyle='--', label='Target Apogee')
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
