#!/bin/bash

SOURCE0=module_template2
printf 'SOURCE0=%s\n' "$SOURCE0"
pwd
rm -r $SOURCE0.ALOE1.6NOV15/
cp -fr ../modules/$SOURCE0/ $SOURCE0.ALOE1.6NOV15/

#Clean Reference Template
pwd
#rm module_template.ALOE1.6/debug_make/Makefile
rm $SOURCE0.ALOE1.6/debug_make/libmodule_template2.a
rm $SOURCE0.ALOE1.6/debug_make/module_template2
rm $SOURCE0.ALOE1.6/lnx_make/Makefile
rm $SOURCE0.ALOE1.6/lnx_make/Makefile.in
rm $SOURCE0.ALOE1.6/aloe/skeleton/*.o
rm $SOURCE0.ALOE1.6/aloe/standalone/*.o
rm $SOURCE0.ALOE1.6/aloe/extern/*.o
rm $SOURCE0.ALOE1.6/aloe/gnuplot/*.o
rm $SOURCE0.ALOE1.6/lnx_make/bin/module_template2
rm $SOURCE0.ALOE1.6/matlab/module_template2.mexglx
rm $SOURCE0.ALOE1.6/matlab/module_template2.mex
rm $SOURCE0.ALOE1.6/src/*.om
rm $SOURCE0.ALOE1.6/src/*.o
rm $SOURCE0.ALOE1.6/standalone/*.o


SOURCE1=$SOURCE0.ALOE1.6
printf 'SOURCE1=%s\n' "$SOURCE1"

if [ $# -ne 1 ] 
then
	echo "Need modulename as first argument"
	exit
fi

if [ -d "$1" ]; then
	echo "Directory" $1 "already exist."
	exit
fi

cp -fr $SOURCE1 ../modules/$1

if [ ! -d "../modules/$1" ]; then
	echo "Could not create directory "$1 "."
	exit
fi


cd ../modules/$1
pwd

STR=s/module_template2/$1/g
#STR2=s/$SOURCE1/$1/g
printf 'STR=%s\n' "$STR"
printf 'STR2=%s\n' "$STR2"

#sed -i $STR debug_make/Makefile
sed -i $STR debug_make/Makefile
sed -i $STR lnx_make/Makefile.am
sed -i $STR src/*.c
sed -i $STR src/*.h
sed -i $STR standalone/test_generate.c
sed -i $STR matlab/module_template2.c
sed -i $STR aloe/skeleton/*.c
sed -i $STR aloe/skeleton/*.h
sed -i $STR aloe/standalone/*.c

STR3=s/$SOURCE0/$1/g
printf 'STR3=%s\n' "$STR3"
sed -i $STR3 .settings/*
sed -i $STR3 .cproject
sed -i $STR3 .project

#Change Files Names
mv matlab/module_template2.c matlab/$1.c
mv src/module_template2.c src/$1.c
mv src/module_template2.h src/$1.h
mv src/module_template2_interfaces.h src/$1_interfaces.h
mv src/module_template2_params.h src/$1_params.h
mv src/module_template2_functions.c src/$1_functions.c
mv src/module_template2_functions.h src/$1_functions.h
rm src/module_template2*.*
rm matlab/module_template2*.*
pwd
ls -al
cd ../..
pwd

#Update modules Makefiles
./scripts/update_modules.pl
make clean
autoconf
./configure LIBS='-lpthread -lboost_system'


