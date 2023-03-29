#!/bin/bash


#sudo schedtool -R -p 20 `pidof jackdbus`
jack_control eps realtime true
jack_control ds alsa
jack_control dps device hw:0
jack_control dps rate 48000
jack_control dps nperiods 3
jack_control dps period 1024

jack_control start

#jackd -dalsa -dhw:1	
#http://www.2uzhan.com/jack-will-not-start/
