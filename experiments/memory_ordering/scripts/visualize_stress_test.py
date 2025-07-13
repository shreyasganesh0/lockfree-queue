import pandas as pd
import matplotlib.pyplot as plt
import sys
import glob
import os

# Find the most recent CSV file
csv_files = glob.glob('results/stress_test_data_*.csv')
if not csv_files:
    print("No CSV data found!")
    sys.exit(1)

latest_csv = max(csv_files, key=os.path.getctime)
print(f"Analyzing: {latest_csv}")

# First, let's examine the file to debug the issue
print("\nExamining CSV structure...")
with open(latest_csv, 'r') as f:
    lines = f.readlines()
    print(f"Total lines: {len(lines)}")
    print(f"Header: {lines[0].strip()}")
    
    # Check for lines with wrong number of fields
    expected_fields = len(lines[0].strip().split(','))
    for i, line in enumerate(lines):
        fields = line.strip().split(',')
        if len(fields) != expected_fields:
            print(f"Line {i+1} has {len(fields)} fields instead of {expected_fields}: {line.strip()}")

# Try to read the CSV with error handling
try:
    # First attempt - standard read
    df = pd.read_csv(latest_csv)
except pd.errors.ParserError as e:
    print(f"\nError reading CSV: {e}")
    print("Attempting to fix and reload...")
    
    # Create a cleaned version of the CSV
    cleaned_lines = []
    with open(latest_csv, 'r') as f:
        lines = f.readlines()
        header = lines[0]
        cleaned_lines.append(header)
        expected_fields = len(header.strip().split(','))
        
        for i, line in enumerate(lines[1:], 1):
            fields = line.strip().split(',')
            if len(fields) == expected_fields:
                cleaned_lines.append(line)
            else:
                print(f"Skipping malformed line {i+1}: {line.strip()}")
    
    # Write cleaned data to temporary file
    temp_csv = latest_csv.replace('.csv', '_cleaned.csv')
    with open(temp_csv, 'w') as f:
        f.writelines(cleaned_lines)
    
    # Try reading the cleaned version
    df = pd.read_csv(temp_csv)
    print(f"Successfully loaded {len(df)} rows after cleaning")

# Continue with visualization only if we have data
if df.empty:
    print("No valid data to visualize!")
    sys.exit(1)

# Create figure with subplots
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(15, 12))
fig.suptitle('Memory Ordering Detection Under System Stress', fontsize=16)

# 1. Detection rate by test type
detection_by_type = df.groupby('Test_Type')['Detected'].agg(['sum', 'count'])
detection_by_type['rate'] = (detection_by_type['sum'] / detection_by_type['count'] * 100)

if not detection_by_type.empty:
    detection_by_type['rate'].plot(kind='bar', ax=ax1, color='steelblue')
    ax1.set_title('Detection Rate by Stress Condition')
    ax1.set_ylabel('Detection Rate (%)')
    ax1.set_xlabel('')
    ax1.tick_params(axis='x', rotation=45)
    
    # Add value labels on bars
    for i, v in enumerate(detection_by_type['rate']):
        ax1.text(i, v + 1, f'{v:.1f}%', ha='center', va='bottom')

# 2. Average iterations to detection
detected_runs = df[df['Detected'] == 1]
if not detected_runs.empty:
    avg_iterations = detected_runs.groupby('Test_Type')['Iterations'].mean()
    avg_iterations.plot(kind='bar', ax=ax2, color='darkgreen')
    ax2.set_title('Average Iterations to Detection')
    ax2.set_ylabel('Iterations')
    ax2.set_yscale('log')
    ax2.tick_params(axis='x', rotation=45)

# 3. Time to detection distribution
detected_times = df[df['Detected'] == 1]['Time_Seconds']
if not detected_times.empty:
    ax3.hist(detected_times, bins=20, color='coral', edgecolor='black')
    ax3.set_title('Time to Detection Distribution')
    ax3.set_xlabel('Time (seconds)')
    ax3.set_ylabel('Count')

# 4. Core configuration comparison
if 'Core_Config' in df.columns:
    core_comparison = df.groupby('Core_Config')['Detected'].agg(['sum', 'count'])
    core_comparison['rate'] = (core_comparison['sum'] / core_comparison['count'] * 100)
    
    # Map the core configs to friendly names
    labels = []
    values = []
    for config in core_comparison.index:
        if config == 'min':
            labels.append('Adjacent\n(min distance)')
        elif config == 'max':
            labels.append('Opposite\n(max distance)')
        else:
            labels.append(config)
        values.append(core_comparison.loc[config, 'rate'])
    
    bars = ax4.bar(labels, values, color=['orange', 'purple'][:len(labels)])
    ax4.set_title('Detection Rate by Core Distance')
    ax4.set_ylabel('Detection Rate (%)')
    
    # Add value labels
    for bar, value in zip(bars, values):
        ax4.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1, 
                f'{value:.1f}%', ha='center', va='bottom')

plt.tight_layout()
plt.savefig('results/stress_test_analysis.png', dpi=300, bbox_inches='tight')
print("\nVisualization saved to: results/stress_test_analysis.png")

# Print summary statistics
print("\nSummary Statistics:")
print(f"Total runs: {len(df)}")
print(f"Total detections: {df['Detected'].sum()}")
print(f"Overall detection rate: {df['Detected'].mean() * 100:.1f}%")

if not detected_runs.empty:
    print(f"Average iterations to detection: {detected_runs['Iterations'].mean():,.0f}")
    print(f"Average time to detection: {detected_runs['Time_Seconds'].mean():.2f} seconds")

plt.show()
