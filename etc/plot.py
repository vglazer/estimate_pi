#!/usr/local/bin/python3

import pandas as pd
import matplotlib.pyplot as plt

for num_points in [1000, 2000, 4000, 8000]:
    df = pd.read_csv(f"./points_{num_points}.csv")

    fig, ax = plt.subplots()

    ax.scatter(df["x"], df["y"], alpha=0.3, s=10)
    circle = plt.Circle((0, 0), 1, color="red", fill=False, label="unit circle")
    ax.add_patch(circle)
    ax.set_title(f"{num_points:,} sample points")

    fig.savefig(f"./plot_{num_points}.png")
