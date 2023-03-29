#!/bin/bash

schedtool -R -p 20 `pidof jackdbus`
jack_control eps realtime true
jack_control ds alsa
jack_control dps device hw:0
jack_control dps rate 48000
jack_control dps nperiods 2
jack_control dps period 2048
dbus-launch jack_control start

#jackd -dalsa -dhw:1	
#http://www.2uzhan.com/jack-will-not-start/
