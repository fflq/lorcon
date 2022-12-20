#!/bin/bash

gcc examples/flq_inject.c -lorcon2
#sudo ./a.out -i wlp8s0 -c "40 HT20" -s seeyou
# -g or -H open mcs set
#sudo ./a.out -i wlp8s0 -c "40 HT20" -g -s seeyou
sudo ./a.out $1

