#! /usr/bin/bash
echo "Hello World"
git clone https://github.com/EliasKramer/Leonardo.git
cd Leonardo
git checkout cmake
cd CMakeLeonardo
sudo snap install cmake --classic

#https://developer.nvidia.com/cuda-downloads?target_os=Linux&target_arch=x86_64&Distribution=Ubuntu
#installing cuda for ubuntu 22
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-ubuntu2204.pin
sudo mv cuda-ubuntu2204.pin /etc/apt/preferences.d/cuda-repository-pin-600
wget https://developer.download.nvidia.com/compute/cuda/12.2.0/local_installers/cuda-repo-ubuntu2204-12-2-local_12.2.0-535.54.03-1_amd64.deb
sudo dpkg -i cuda-repo-ubuntu2204-12-2-local_12.2.0-535.54.03-1_amd64.deb
sudo cp /var/cuda-repo-ubuntu2204-12-2-local/cuda-*-keyring.gpg /usr/share/keyrings/
sudo apt-get update
sudo apt install libnvidia-compute-495
sudo apt-get -y install cuda

nvcc --version

cd CMakeLeonardo
cmake ..
cd ..
make

#sudo apt-get install cmake

#todo flt_max flt_min removal