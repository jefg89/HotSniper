#!/bin/bash

if [ $# -eq 1 ]
then
    RUNS=$1
else
    RUNS=1
fi

for (( i=1; i<=$RUNS; i++ ))
do
    rm -rf benchmarks/*.log
    PYTHONIOENCODING="UTF-8" python3 simulationcontrol/run.py
done

