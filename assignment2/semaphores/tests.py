# This script should treat the log files into a csv file for plotting
# Format is: (P, C), time_2, time_4, time_8, time_16, time_32


def extract_avg(filepath):

    '''This function should extract the average runtime from a given log'''

    # Open file
    f = open(filepath, 'r')
    fil = f.readlines()
    f.close()

    # Take times
    times = []
    for line in fil:
        # Looking for lines "real\tXmYY.ZZZs"
        if line[:4] == "real":
            sample = line[:-1].split('\t')[1]
            minutes = int(sample.split('m')[0])
            seconds = float(sample.split('m')[1][:-1])
            sample = minutes*60 + seconds
            times.append(sample)

    return sum(times)/len(times)

# We iterate for every N, P and C
times = {}
for p in [1, 2, 5, 10]:
    for c in [1, 2, 5, 10]:
        times[(p, c)] = {}
        for n in [2, 4, 8, 16, 32]:
            avg = extract_avg("logs/exec_times-%i-%i-%i.log" % (n, p, c))
            times[(p, c)][n] = avg

# Finally, we save the results as a csv file
# out = open("results.csv", 'w')
for r in times.keys():
    p, c = r
    line = "(%i, %i)," % (p, c)
    for n in sorted(times[r].keys()):
        print n
        line += "%f," % (times[r][n])
    line = line[:-1] + "\n"
    # out.write(line)
# out.close()

print "DONE!"
