import subprocess
import time
import matplotlib.pyplot as plt
import numpy as np

# executable = "./knapsack"
executable = "./knapsack.exe"

seeds = ["10", "12", "1209", "239", "85"]

for seed in seeds:
    inputFile = seed
    outputFile = f'out-knap-{seed}.txt'

    start = time.perf_counter()
    proc = subprocess.run([executable, inputFile, outputFile], input=seed, text=True, capture_output=True)
    end = time.perf_counter()


for seed in seeds:
    N = 0
    C = 0

    data = {}
    max_fit = []
    mean_fit = []
    gens = []

    # Open the file for reading
    with open(f'out-knap-{seed}.txt', 'r') as file:
        current_gen = None

        for line in file:
            words = line.strip().split()

            if len(words) == 0:
                continue

            if len(words) == 4:
                N = int(words[1])
                C = int(words[3])
                continue

            if len(words) == 2 and words[0] == 'GEN':
                current_gen = int(words[1])
                data[current_gen] = {"fitness":[], "data": []}
                continue

            if len(words) == 2 and words[0] == 'FIT_MEAN':
                data[current_gen]["fitness_mean"] = float(words[1])
                continue

            if current_gen is not None:
                row_data = [int(word) for word in words]
                fitness = row_data.pop()

                data[current_gen]["fitness"].append(fitness)
                data[current_gen]["data"].append(row_data)


    for gen, values in data.items():
        gens.append(gen)
        max_fit.append(max(values['fitness']))
        mean_fit.append(values['fitness_mean'])

        # print(f"Generation {gen}")
        # print(f"FIT_MEAN: {values['fitness_mean']}")
        # print(f"FIT_MAX: {max(values['fitness'])}\n")


    # Create a new figure and axis
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize = (15,9))

    ax1.plot(gens, max_fit, label='Fitness Max', color='blue')
    ax1.set_xlabel('Generation')
    ax1.set_ylabel('Fitness')
    ax1.set_title('Fitness Max over Generations')

    ax2.plot(gens, mean_fit, label='Fitness Mean', color='red')
    ax2.set_xlabel('Generation')
    ax2.set_ylabel('Fitness')
    ax2.set_title('Fitness Mean over Generations')

    fig.text(0.5, 0.95, f'N items: {N}  |  Knapsack Capacity: {C}', fontsize=16, ha='center')

    plt.savefig(f'fitness-{seed}')
    # plt.show()