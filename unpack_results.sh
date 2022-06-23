#!/bin/bash
if [[ ! -f "results.tar" ]]; then
    echo "results.tar file does not exist"
    exit
fi

tar -xvf results.tar

if [[ ! -d "results/" ]]; then
    echo "Creating results folder"
    mkdir results
fi

for f in *; do
    if [[ -d "$f" ]] &&  [[ "$f" != "results" ]]; then
        echo "$f"
        gzip -d "$f"/PeriodicThermal.log.gz
        cp "$f"/PeriodicThermal.log results/"$f".log
        gzip -d "$f"/execution.log.gz 
        # replace app0 with Packet
        cat "$f"/execution.log | grep "app0" >> sent_raw.log
    fi
done

cd results
for file_name in *; do
    if  [[ "$f" != "*.out" ]]; then
        python3 ../csv_helper.py $file_name
    fi
done

mv *.out ../
cd ..
echo "Cleaning files"
rm -rf results/ results_*
