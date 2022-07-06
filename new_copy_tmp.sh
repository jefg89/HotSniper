#!/bin/bash

rm -rf /tmp/HotSniper

PROJECT=/home/gomjef00/hotSniper_paper/
#PROJECT=/home/jef/attetstation/HotSniper
#cp $PROJECT/HotSniper/HotSniper/benchmarks/run_on_tmp_ssh.sh .
rsync -arv \
    --exclude HotSniper/benchmarks/cpu2006 \
    --exclude HotSniper/benchmarks/jikes \
    --exclude HotSniper/benchmarks/npb \
    --exclude HotSniper/benchmarks/myapps \
    --exclude HotSniper/results \
    --exclude HotSniper/.git \
    --exclude HotSniper/benchmarks/parsec/parsec-2.1/pkgs/apps/facesim \
    --exclude HotSniper/benchmarks/parsec/parsec-2.1/pkgs/apps/raytrace \
    --exclude HotSniper/benchmarks/parsec/parsec-2.1/pkgs/apps/vips \
    --exclude HotSniper/benchmarks/parsec/parsec-2.1/pkgs/libs \
    --exclude HotSniper/benchmarks/parsec/parsec-2.1/pkgs/tools \
    --exclude HotSniper/common \
    --exclude HotSniper/test \
    $PROJECT/HotSniper /tmp

    #--exclude HotSniper/benchmarks/parsec/ \
cd /tmp/
cd HotSniper/
./pyrun.sh 10
./copy_results.sh $1


