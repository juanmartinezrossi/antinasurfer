#!/bin/bash

jack_control start
#sudo schedtool -R -p 20 `pidof jackdbus`
jack_control eps realtime true
jack_control ds alsa
jack_control dps device hw:0
jack_control dps rate 48000
jack_control dps nperiods 2
jack_control dps period 1024	#1024

#jackd -dalsa -dhw:1	
#http://www.2uzhan.com/jack-will-not-start/
