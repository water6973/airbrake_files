# NOTE: make sure data is in .csv format with time in seconds and altitude in meters

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

mass = 0.580 # kg (update for each flight)
actual_apogee = 298 # meters (341 for test 2, 293 for test 3, 293 for test 4, 298 for test 5)

k = 0.0016
g = 9.8

slice_length = 5

def mean_projected_apogee(row, df, slice):
    # Determine the start index for the last twenty values before the current row
    start_index = max(0, row.name - slice)
    # Get the last twenty (or fewer) projections up to the current index
    sliced_projections = df.loc[start_index:row.name - 1, 'Raw Projected Apogee']
    # Calculate and return the mean of these values
    return sliced_projections.mean()

def main():
    filename = 'C:/Users/gavin/Documents/test5.csv'
    graph_filename = 'C:/Users/gavin/Documents/test-5-adaptive.png'

    df = pd.read_csv(filename, names=['Time', 'Altitude'])

    df['Velocity'] = (df['Altitude'].diff() / df['Time'].diff())
    df['Altitude'] = df['Altitude']

    # Calculate projected apogee and error

    # df['Projected Apogee'] = df['Altitude'] + df['Velocity'] * df['Remaining Time'] * 1/2
    df['Raw Projected Apogee'] = df['Altitude'] + (mass / (2 * k)) * np.log((k * (df['Velocity'] ** 2)) / (mass * g) + 1)
    df['Projected Apogee'] = df.apply(mean_projected_apogee, axis=1, df=df, slice=slice_length)

    df.loc[df['Time'] <= 1.5, 'Projected Apogee'] = np.nan

    df['Error'] = abs(actual_apogee - df['Projected Apogee'])

    # Plotting the results
    plt.figure(figsize=(10, 6))
    plt.plot(df['Time'], df['Altitude'], label='Altitude')
    plt.plot(df['Time'], df['Projected Apogee'], label='Projected Apogee')
    plt.plot(df['Time'], df['Velocity'], label='Velocity')
    plt.axhline(y=actual_apogee, color='r', linestyle='-', label='Actual Apogee')
    plt.plot(df['Time'], df['Error'], label='Error', color='green')
    plt.ylim(0, 400)
    plt.xlim(0, 10)

    plt.xlabel('Time (Seconds)')
    plt.ylabel('Altitude (Meters)')
    plt.title('Time vs Altitude with Projected Apogee and Error')
    plt.legend()

    # Save the plot to a file
    plt.savefig(graph_filename)

    print(f"Graph saved as {graph_filename}")

if __name__ == "__main__":
    main()
