#!/usr/local/bin/python3

import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("./points_1000.csv")

fig, ax = plt.subplots()

ax.scatter(df['x'], df['y'], alpha=0.3, s=10)
circle = plt.Circle((0,0), 1, color='red', fill=False, label='unit circle')
ax.add_patch(circle)
num_points = len(df['x'])
ax.set_title(f'{num_points:,} sample points')

fig.savefig(f'./plot_{num_points}.png')
