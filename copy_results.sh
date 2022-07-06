#!/bin/bash
PROJECT=/home/gomjef00/hotSniper_paper/
cd /tmp/
cd HotSniper/
FILE=$PROJECT/HotSniper/results/benchmarks/$1

if [ ! -d "$FILE" ]; then
    mkdir $FILE
fi
cp -r results/* $PROJECT/HotSniper/results/benchmarks/$1

