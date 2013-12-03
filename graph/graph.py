import matplotlib.pyplot as plt
import csv

from sys import argv, exit
from collections import defaultdict

def main():
    if len(argv) != 2:
        print "Syntax is: python graph.py <datafile> "
        exit(1)

    data = loadData(argv[1])

    for (impl, [elementCount, writesPerSec]) in data.items():
        plt.plot(elementCount, writesPerSec, label=impl)

    plt.legend(data.keys())
    plt.show()

def loadData(fileName):
    # maps a series to a pair of lists of independent variable, dependent variable pairs
    data = defaultdict(lambda: [[],[]])
    with open(fileName, 'rb') as dataFile:
        reader = csv.DictReader(dataFile)
        for row in reader:
            data[row['Implementation']][0].append(row['NumElements'])
            data[row['Implementation']][1].append(float(row['ThroughputWPerSec']))
    return data




if __name__ == '__main__':
    main()
