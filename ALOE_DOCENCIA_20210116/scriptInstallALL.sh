_____________________________________________________________________________________________________________
#!/bin/bash

# Update & Upgrade
sudo apt-get install update
sudo apt-get upgrade 
# liboost
sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev build-essential libbz2-dev libboost-all-dev
# Modify /etc/sysctl.conf
# Delete specific lines
sudo grep -v "kernel.msgmnb" /etc/sysctl.conf > /tmp/tmp
sudo mv /tmp/tmp /etc/sysctl.conf
sudo grep -v "kernel.msgmax" /etc/sysctl.conf > /tmp/tmp
sudo mv /tmp/tmp /etc/sysctl.conf
sudo grep -v "net.core.rmem_max" /etc/sysctl.conf > /tmp/tmp
sudo mv /tmp/tmp /etc/sysctl.conf
sudo grep -v "net.core.wmem_max" /etc/sysctl.conf > /tmp/tmp
sudo mv /tmp/tmp /etc/sysctl.conf
# Add specific lines
sudo echo 'kernel.msgmnb=209715200' | sudo tee -a /etc/sysctl.conf
sudo echo 'kernel.msgmax=10485760' | sudo tee -a /etc/sysctl.conf
sudo echo 'net.core.rmem_max=50000000' | sudo tee -a /etc/sysctl.conf
sudo echo 'net.core.wmem_max=1048576' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
sudo cat /etc/sysctl.conf
echo "Please, check in previous lines that kernel.msgmnb, kernel.msgmax, net.core.rmem_max, net.core.wmem_max appears only one time in /etc/sysctl.conf file"
echo "If so, please, delete all except one of them for each entry"
echo "Please, Did you Check: (Y/N)"
read input_variable
echo "You entered: $input_variable" 
echo "If NO, please cancel this script (CTRL-C) and make 'sudo gedit /etc/sysctl.conf' to erase all previous lines and start that script again"


# Add Readline
sudo apt-get install libreadline6 libreadline6-dev
# Add fftw
sudo apt-get install fftw3-dev pkg-config
# Install gnuplot-qt
sudo apt-get install gnuplot-qt rlwrap

# Install JACK
# Ask for user name
user=$(whoami)
# Delete user lines
sudo grep -v "$user" /etc/security/limits.conf > /tmp/tmp2
sudo mv /tmp/tmp2 /etc/security/limits.conf
sudo grep -v "# End of file" /etc/security/limits.conf > /tmp/tmp2
sudo mv /tmp/tmp2 /etc/security/limits.conf
# ADD user lines
sudo echo "$user        -        rtprio        99" | sudo tee -a /etc/security/limits.conf
sudo echo "$user        -        memlock       unlimited" | sudo tee -a /etc/security/limits.conf
sudo echo "# End of file" | sudo tee -a /etc/security/limits.conf

#sudo grep -v "$user        -        memlock       unlimited" /etc/security/limits.conf
#sudo mv /tmp/tmp /etc/security/limits.conf
# Check Duplicated lines 
sudo cat /etc/security/limits.conf
echo "Please, check in previous lines that $user	-	rtprio	99 or  $user	-	memlock	unlimited appears only one time in /etc/security/limits.conf file"
echo "If so, please, delete all except one of them for each entry"
echo "Please, Did you Check: (Y/N)"
read input_variable
echo "You entered: $input_variable" 
echo "If NO, please cancel this script (CTRL-C) and make 'sudo gedit /etc/security/limits.conf' to erase all previous lines and start that script again"

sudo apt-get install qjackctl
sudo apt-get install libjack-jackd2-dev libjack-jackd2-0
sudo apt-get install libsamplerate0 libsamplerate0-dev
sudo apt-get install jackd2


# Install UHD
sudo apt-get install libusb-1.0-0-dev python-mako doxygen python-docutils cmake
# Alternative Commands
#-------------------------------------------------
#git clone git://github.com/EttusResearch/uhd.git
tar xvfz UHD.03.09.04.tar.gz
#-------------------------------------------------
cd UHD.03.09.04/host
mkdir build
cd build
cmake ../
make
make test
sudo make install
sudo ldconfig
cd ../../..
# Test UHD
uhd_usrp_probe --args="addr=192.168.10.2"






