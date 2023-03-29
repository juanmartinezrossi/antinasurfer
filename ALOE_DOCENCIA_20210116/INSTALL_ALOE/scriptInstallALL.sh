_____________________________________________________________________________________________________________
#!/bin/bash
# Update & Upgrade
sudo apt-get install update
sudo apt-get upgrade 
#INSTALL AUTOMAKE
sudo apt-get install automake
# INSTALL GAWK
sudo apt-get install gawk
# liboost
sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev build-essential libbz2-dev libboost-all-dev
# Modify /etc/sysctl.conf
sudo echo 'kernel.msgmnb=209715200' | sudo tee -a /etc/sysctl.conf
sudo echo 'kernel.msgmax=10485760' | sudo tee -a /etc/sysctl.conf
sudo echo 'net.core.rmem_max=50000000' | sudo tee -a /etc/sysctl.conf
sudo echo 'net.core.wmem_max=1048576' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
# Add Readline
sudo apt-get install libreadline6 libreadline6-dev
#sudo apt-get install libreadline libreadline-dev
# Add fftw
sudo apt-get install fftw3-dev pkg-config
# Install gnuplot-qt
sudo apt-get install gnuplot-qt rlwrap

# Install JACK
# Ask for user name
#echo "Please enter User Name:"
#read input_variable
#echo "You entered: $input_variable, Is it correct?"
#sudo echo "$input_variable	-	rtprio	99" | sudo tee -a /etc/security/limits.conf
#sudo echo "$input_variable	-	memlock    unlimited" | sudo tee -a /etc/security/limits.conf

user=$(whoami)
sudo echo "$user        -        rtprio        99" | sudo tee -a /etc/security/limits.conf
sudo echo "$user        -        memlock       unlimited" | sudo tee -a /etc/security/limits.conf
sudo cat /etc/security/limits.conf


sudo apt-get install libjack-jackd2-dev libjack-jackd2-0
sudo apt-get install libsamplerate0 libsamplerate0-dev
sudo apt-get install jackd2

# Install ALSA
sudo apt-get install libasound2
sudo apt-get install libasound2-dev


# Install UHD
## LOOK AT: https://kb.ettus.com/Building_and_Installing_the_USRP_Open-Source_Toolchain_(UHD_and_GNU_Radio)_on_Linux


#sudo apt-get install libusb-1.0-0-dev python-mako doxygen python-docutils cmake
#git clone git://github.com/EttusResearch/uhd.git
#cd uhd/host
#mkdir build
#cd build
#cmake ../
#make
#make test
#sudo make install
#sudo ldconfig
#cd ../../..
# Test UHD
#uhd_usrp_probe --args="addr=192.168.10.2"

# Alternativelly
#sudo apt-get update
#sudo apt-get install uhd-host






