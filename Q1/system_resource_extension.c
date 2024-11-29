import matplotlib.pyplot as plt
import csv

def parse_usage_file(filename):
    times = []
    cpu_usages = []
    memory_usages = []
    network_usages = []

    with open(filename, 'r') as f:
        reader = csv.reader(f, delimiter=',')
        for row in reader:
            # Assuming format: "Time: HH:MM:SS, CPU: X.XX%, Memory: Y.YY%, Network: Z KB"
            time = row[0].split(": ")[1].strip()
            cpu = float(row[1].split(": ")[1].strip('%'))
            memory = float(row[2].split(": ")[1].strip('%'))
            network = float(row[3].split(": ")[1].split()[0])

            times.append(time)
            cpu_usages.append(cpu)
            memory_usages.append(memory)
            network_usages.append(network)

    return times, cpu_usages, memory_usages, network_usages

def plot_system_metrics(filename):
    times, cpu_usages, memory_usages, network_usages = parse_usage_file(filename)

    plt.figure(figsize=(10, 6))
    plt.plot(times, cpu_usages, label='CPU Usage (%)', marker='o')
    plt.plot(times, memory_usages, label='Memory Usage (%)', marker='s')
    plt.plot(times, network_usages, label='Network Usage (KB)', marker='^')

    plt.title('System Resource Monitoring')
    plt.xlabel('Time')
    plt.ylabel('Usage')
    plt.legend()
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_system_metrics('usage.txt')