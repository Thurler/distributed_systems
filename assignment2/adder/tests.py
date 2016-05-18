# This script should treat the log files into a csv file for plotting
# Format is: K, time_8, time_9, time_10


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

# First, we take the overhead time average
overheads = {}
for n in range(8, 11):
    overheads[n] = extract_avg("logs/alloc-%i.log" % (n))

# Then, we iterate for every N and K
times = {}
for k in [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 30]:
    times[k] = {}
    for n in range(8, 11):
        avg = extract_avg("logs/exec_times-%i-%i.log" % (n, k))
        sample = avg - overheads[n]
        times[k][n] = sample

# Finally, we save the results as a csv file
out = open("results.csv", 'w')
for k in times.keys():
    line = "%i," % (k)
    for n in times[k].keys():
        line += "%f," % (times[k][n])
    line = line[:-1] + "\n"
    out.write(line)
out.close()

print "DONE!"
