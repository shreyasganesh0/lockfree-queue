"""
Analyze memory reordering detetction results and create a visualization.
The data was taken by running the bin/memory_ordering experiment 10 times.
Charts show the non deterministic nature of memory ordering
"""

import matplotlib.pyplot as plt
import numpy as np
import os

#!/usr/bin/env python3
"""
Analyze memory reordering detection results and create visualizations.
This script parses the experimental output and generates statistical analysis
and charts to demonstrate the non-deterministic nature of memory reordering.
"""

import matplotlib.pyplot as plt
import numpy as np
import os

# Your experimental data - the iteration counts where reordering was detected
detection_iterations = [
    101739127,
    101338235,
    100270916,
    761688,
    102408891,
    9992,
    100595764,
    100142033,
    100547611,
    100092265
]

min_iterations = min(detection_iterations)
max_iterations = max(detection_iterations)
mean_iterations = np.mean(detection_iterations)
median_iterations = np.median(detection_iterations)
std_iterations = np.std(detection_iterations)

print("Memory Reordering Detection Statistics")
print("=" * 40)
print(f"Runs analyzed: {len(detection_iterations)}")
print(f"Minimum iterations: {min_iterations:,} ({min_iterations/1e6:.2f}M)")
print(f"Maximum iterations: {max_iterations:,} ({max_iterations/1e6:.2f}M)")
print(f"Mean iterations: {mean_iterations:,.0f} ({mean_iterations/1e6:.2f}M)")
print(f"Median iterations: {median_iterations:,.0f} ({median_iterations/1e6:.2f}M)")
print(f"Standard deviation: {std_iterations:,.0f} ({std_iterations/1e6:.2f}M)")
print(f"Coefficient of variation: {(std_iterations/mean_iterations)*100:.1f}%")
print()

os.makedirs('results', exist_ok=True)

plt.style.use('seaborn-v0_8-darkgrid')
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 10))

# Subplot 1: Bar chart comparing detection iterations across runs
run_numbers = range(1, len(detection_iterations) + 1)
colors = ['red' if x < 1e6 else 'orange' if x < 50e6 else 'steelblue' 
          for x in detection_iterations]

bars = ax1.bar(run_numbers, detection_iterations, color=colors, alpha=0.7)
ax1.set_xlabel('Run Number', fontsize=12)
ax1.set_ylabel('Iterations Until Detection', fontsize=12)
ax1.set_title('Memory Reordering Detection Across Multiple Runs', fontsize=14, fontweight='bold')
ax1.set_yscale('log')  # Logarithmic scale due to huge variance

# Add value labels on bars
for bar, value in zip(bars, detection_iterations):
    height = bar.get_height()
    if value < 1e6:
        label = f'{value:,}'
    else:
        label = f'{value/1e6:.1f}M'
    ax1.text(bar.get_x() + bar.get_width()/2., height,
             label, ha='center', va='bottom', fontsize=9)

ax1.axhline(y=mean_iterations, color='green', linestyle='--', 
            label=f'Mean: {mean_iterations/1e6:.1f}M iterations', alpha=0.7)
ax1.axhline(y=median_iterations, color='purple', linestyle='--', 
            label=f'Median: {median_iterations/1e6:.1f}M iterations', alpha=0.7)
ax1.legend()

ax1.grid(True, which="both", ls="-", alpha=0.2)

# Create logarithmic bins to better show the distribution
log_bins = np.logspace(np.log10(min_iterations * 0.5), 
                      np.log10(max_iterations * 1.5), 20)

counts, bins, patches = ax2.hist(detection_iterations, bins=log_bins, 
                                color='darkblue', alpha=0.7, edgecolor='black')
ax2.set_xlabel('Iterations Until Detection (log scale)', fontsize=12)
ax2.set_ylabel('Number of Runs', fontsize=12)
ax2.set_title('Distribution of Memory Reordering Detection Points', fontsize=14, fontweight='bold')
ax2.set_xscale('log')

# Highlight the outliers
outlier_threshold = 1e6
early_detections = [x for x in detection_iterations if x < outlier_threshold]
if early_detections:
    ax2.axvspan(min_iterations * 0.5, outlier_threshold, 
                alpha=0.2, color='red', 
                label=f'Early detection ({len(early_detections)} runs)')
    ax2.legend()

# Add text box with key insights
textstr = (f'Key Insights:\n'
          f'• Extreme variance: {min_iterations:,} to {max_iterations:,}\n'
          f'• Most detections near maximum iterations\n'
          f'• Rare early detection at {min_iterations:,} iterations\n'
          f'• Proves non-deterministic hardware behavior')
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax2.text(0.02, 0.95, textstr, transform=ax2.transAxes, fontsize=10,
         verticalalignment='top', bbox=props)

# Adjust layout and save
plt.tight_layout()
plt.savefig('results/reordering_detection_distribution.png', dpi=300, bbox_inches='tight')
print(f"Visualization saved to: results/reordering_detection_distribution.png")

with open('results/reordering_detection_data.csv', 'w') as f:
    f.write("Run,Iterations\n")
    for i, iterations in enumerate(detection_iterations, 1):
        f.write(f"{i},{iterations}\n")
print(f"Raw data saved to: results/reordering_detection_data.csv")

plt.show()
