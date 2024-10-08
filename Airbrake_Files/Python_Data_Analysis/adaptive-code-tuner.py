# NOTE: make sure data is in .csv format with time in seconds and altitude in meters

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Gravity constant
g = 9.8  # Gravity  

# Define dynamic ranges for slice lengths and k values
slice_length_range = range(1, 20, 1) 
k_values_range = np.arange(0.0015, 0.003, 0.0001)

# Files, their respective actual apogees, and masses
files_apogees_masses = {
    '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test5.csv': {'apogee': 298, 'mass': 0.585},
    '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test2.csv': {'apogee': 341, 'mass': 0.580},
    '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test11.csv': {'apogee': 266.4, 'mass': 0.606},
}

def mean_projected_apogee(row, df, slice_length, k):
    start_index = max(0, row.name - slice_length)
    sliced_projections = df.loc[start_index:row.name - 1, 'Raw Projected Apogee']
    return sliced_projections.mean()

def calculate_error_for_file(filename, details, slice_length, k):
    df = pd.read_csv(filename, names=['Time', 'Altitude'])
    df['Velocity'] = (df['Altitude'].diff() / df['Time'].diff())
    df['Adjusted Velocity'] = df['Velocity'].copy() 
    for i in range(1, len(df)):
        if (df.at[i, 'Time'] > 2 and df.at[i, 'Time'] < 6): # change these values (sets the interval of time where velocity change clamp is applied)
            prev_velocity = df.at[i - 1, 'Adjusted Velocity']
            current_velocity = df.at[i, 'Velocity']
            if current_velocity > prev_velocity:
                df.at[i, 'Adjusted Velocity'] = min(prev_velocity + 5, current_velocity) # sets velocity change clamp in the positive direction
            elif current_velocity < prev_velocity:
                df.at[i, 'Adjusted Velocity'] = max(prev_velocity - 5, current_velocity) # sets velocity change clamp in the negative direction
    mass = details['mass']  # Use mass specific to this file
    df['Raw Projected Apogee'] = df['Altitude'] + (mass / (2 * k)) * np.log((k * df['Adjusted Velocity'] ** 2) / (mass * g) + 1)
    df['Projected Apogee'] = df.apply(mean_projected_apogee, axis=1, df=df, slice_length=slice_length, k=k)
    df.loc[df['Time'] <= 1.5, 'Projected Apogee'] = np.nan  # Exclude early time values
    df.loc[df['Time'] >= 10, 'Projected Apogee'] = np.nan
    df['Error'] = abs(details['apogee'] - df['Projected Apogee'])
    return df['Error'].mean()

# Test all combinations of slice lengths and k values
best_combination = (None, None)
best_error = float('inf')

for slice_length in slice_length_range:
    for k in k_values_range:
        print(f"Testing slice_length={slice_length}, k={k}")
        errors = [
            calculate_error_for_file(filename, details, slice_length, k)
            for filename, details in files_apogees_masses.items()
        ]
        avg_error = sum(errors) / len(errors)  # Average error across all files
        print(f"Average error for slice_length={slice_length}, k={k}: {avg_error}")
        
        if avg_error < best_error:
            best_error = avg_error
            best_combination = (slice_length, k)

best_slice_length, best_k = best_combination
if best_slice_length is not None and best_k is not None:
    print(f"Best slice length: {best_slice_length}, Best k value: {best_k:.4f}, with an average error of: {best_error}")
else:
    print("No valid combination found.")
