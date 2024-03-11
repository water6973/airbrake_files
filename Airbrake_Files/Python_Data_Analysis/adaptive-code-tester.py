import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

mass = 0.598  # kg (update for each flight)
actual_apogee = 239  # meters 
target_apogee = 250 

k = 0.002
g = 9.8

slice_length = 5

def mean_projected_apogee(row, df, slice):
    # Determine the start index for the last twenty values before the current row
    start_index = max(0, row.name - slice)
    # Get the last five (or fewer) projections up to the current index
    sliced_projections = df.loc[start_index:row.name - 1, 'Raw Projected Apogee']
    # Calculate and return the mean of these values
    return sliced_projections.mean()

def apply_constraints(df):
    for i in range(1, len(df)):
        if 2000 <= df.loc[i, 'Time'] <= 6000:
            # Ensure velocity only decreases by 1 m/s or stays the same
            prev_velocity = df.loc[i - 1, 'Velocity']
            current_velocity = df.loc[i, 'Velocity']
            if current_velocity > prev_velocity:
                df.loc[i, 'Velocity'] = prev_velocity
            else:
                df.loc[i, 'Velocity'] = max(prev_velocity - 1, current_velocity)
            
            prev_altitude = df.loc[i - 1, 'Altitude']
            current_altitude = df.loc[i, 'Altitude']
            if current_altitude < prev_altitude:
                df.loc[i, 'Altitude'] = prev_altitude
    return df

def main():
    filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Clean_Data/test16.csv'
    graph_filename = '/home/gavin/Documents/GitHub/airbrake_files/Airbrake_Files/Flight_Data/Graphs/theoretical_test16.png'

    df = pd.read_csv(filename, usecols=[0, 1], names=['Time', 'Altitude'])

    df['Velocity'] = (df['Altitude'].diff() / df['Time'].diff()) * 1000

    # Apply velocity constraints
    df = apply_constraints(df)

    # Calculate projected apogee and error
    df['Raw Projected Apogee'] = df['Altitude'] + (mass / (2 * k)) * np.log((k * (df['Velocity'] ** 2)) / (mass * g) + 1)
    df['Projected Apogee'] = df.apply(mean_projected_apogee, axis=1, df=df, slice=slice_length)

    df.loc[df['Time'] <= 1500, 'Projected Apogee'] = np.nan

    # Plotting the results
    plt.figure(figsize=(10, 6))
    plt.axhline(y=actual_apogee, color='g', linestyle='--', label='Actual Apogee')
    plt.axhline(y=target_apogee, color='r', linestyle='--', label='Target Apogee')
    plt.plot(df['Time'], df['Altitude'], label='Altitude')
    plt.plot(df['Time'], df['Projected Apogee'], label='Projected Apogee')
    plt.plot(df['Time'], df['Velocity'], label='Velocity')
    plt.ylim(0, 400)
    plt.xlim(0, 10000)

    plt.xlabel('Time (Seconds)')
    plt.ylabel('Altitude (Meters)')
    plt.title('Time vs Altitude with Projected Apogee')
    plt.legend()

    # Save the plot to a file
    plt.savefig(graph_filename)

    print(f"Graph saved as {graph_filename}")

if __name__ == "__main__":
    main()