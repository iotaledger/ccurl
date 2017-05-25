#!/bin/bash
sudo apt-get update
sudo apt-get install gcc
wget http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1604/x86_64/cuda-repo-ubuntu1604_8.0.61-1_amd64.deb
sudo dpkg -i cuda-repo-ubuntu1604_8.0.61-1_amd64.deb
sudo apt-get update
sudo apt-get install cuda
export PATH=/usr/local/cuda-8.0/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda-8.0/lib64:$LD_LIBRARY_PATH
sudo apt-get install opencl-headers
echo PATH=/usr/local/cuda/bin:$PATH >> /home/ubuntu/.bashrc
echo LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH >> /home/ubuntu/.bashrc

