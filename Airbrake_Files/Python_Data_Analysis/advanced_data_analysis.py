import pandas as pd
import matplotlib.pyplot as plt

# Load the data
df = pd.read_csv('path_to_file.csv')

# Plot altitude and mean_projected_apogee
plt.figure(figsize=(10, 6))
plt.plot(df['flight_time'], df['altitude'], label='Altitude')
plt.plot(df['flight_time'], df['mean_projected_apogee'], label='Mean Projected Apogee', linestyle='--')

# Find indices where deploy changes state and plot vertical lines
for i in range(1, len(df)):
    if df['deploy'].iloc[i] and not df['deploy'].iloc[i-1]:  # False to True
        plt.axvline(x=df['flight_time'].iloc[i], color='g', linestyle='--', label='Deploy Activated')
    elif not df['deploy'].iloc[i] and df['deploy'].iloc[i-1]:  # True to False
        plt.axvline(x=df['flight_time'].iloc[i], color='r', linestyle='--', label='Deploy Deactivated')

# Plot horizontal line at apogee
max_altitude = df['altitude'].max()
plt.axhline(y=max_altitude, color='b', linestyle='-', label=f'Apogee ({max_altitude} units)')

# Improve plot
plt.xlabel('Flight Time')
plt.ylabel('Altitude / Mean Projected Apogee')
plt.title('Flight Profile')
plt.legend()
plt.grid(True)
plt.tight_layout()

# Show plot
plt.show()
