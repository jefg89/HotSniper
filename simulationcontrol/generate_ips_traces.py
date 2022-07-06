import os
from resultlib import *

def main():
    runs = get_runs()
    for run in runs:
        ips_traces = get_ips_traces(run)
        print(RESULTS_FOLDER + run)
        f = open(RESULTS_FOLDER + run + "/IPS.log", "w")
        for x in ips_traces[0]:
            #print(x/1000000000)
            f.write("{:.3f}\n".format(x/1000000000))
        f.close()
        #active_cores = get_active_cores(run)
        #print(active_cores, "/", len(ips_traces))
        #print((ips_traces[0]))
if __name__ == '__main__':
    main()
