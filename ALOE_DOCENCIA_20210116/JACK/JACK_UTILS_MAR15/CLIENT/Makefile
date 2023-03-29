TARGET = jack_client
LIBS = -lrt -ldl -lpthread -lm -ljack -lsamplerate #-lasound 
CC = gcc
CFLAGS = -g -Wall

.PHONY: clean all default

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)






#http://jackaudio.org/files/docs/html/index.html
#SETUP JACK
#http://blog.desdelinux.net/introduccion-a-jack-audio-connection-kit/
#Download JACK2
#Follow README 
#####Install libsamplerate: Donwload from http://www.linuxfromscratch.org/blfs/view/svn/multimedia/libsamplerate.html
#./configure --prefix=/usr --disable-static && make
#sudo make htmldocdir=/usr/share/doc/libsamplerate-0.1.8 install
#####Install alsa
#Synaptics: add alsaplayer-jack and multimedia-jack

############################3http://dis-dot-dat.net/index.cgi?item=jacktuts/starting/invsimpleclient

#####Compile jack2
#./waf configure --prefix /usr
#./waf
#./waf install

#pkg-config --cflags jack
#pkg-config --libs jack

#INFO: execute jack
#jackd -d alsa
#jack_simple_client



#####Instala via synaptics libjack-jackd2-dev i libjack-jackd2-0




