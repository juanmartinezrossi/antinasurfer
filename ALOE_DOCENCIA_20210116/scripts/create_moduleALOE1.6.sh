#!/bin/bash

SOURCE0=module_template
printf 'SOURCE0=%s\n' "$SOURCE0"

rm -r $SOURCE0.ALOE1.6/
cp -fr ../modules/$SOURCE0/ $SOURCE0.ALOE1.6/

#Clean Reference Template
pwd
#rm module_template.ALOE1.6/debug_make/Makefile
rm $SOURCE0.ALOE1.6/debug_make/libmodulename.a
rm $SOURCE0.ALOE1.6/lnx_make/Makefile
rm $SOURCE0.ALOE1.6/lnx_make/Makefile.in
rm $SOURCE0.ALOE1.6/aloe/skeleton/*.o
rm $SOURCE0.ALOE1.6/aloe/standalone/*.o
rm $SOURCE0.ALOE1.6/aloe/extern/*.o
rm $SOURCE0.ALOE1.6/aloe/gnuplot/*.o
rm $SOURCE0.ALOE1.6/debug_make/modulename
rm $SOURCE0.ALOE1.6/lnx_make/bin/modulename
rm $SOURCE0.ALOE1.6/matlab/modulename.mexglx
rm $SOURCE0.ALOE1.6/matlab/modulename.mex
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

STR=s/modulename/$1/g
#STR2=s/$SOURCE1/$1/g
printf 'STR=%s\n' "$STR"
#printf 'STR2=%s\n' "$STR2"

#sed -i $STR debug_make/Makefile
sed -i $STR debug_make/Makefile
sed -i $STR lnx_make/Makefile.am
sed -i $STR src/*.c
sed -i $STR src/*.h
sed -i $STR standalone/test_generate.c
sed -i $STR matlab/modulename.c
sed -i $STR aloe/skeleton/*.c
sed -i $STR aloe/skeleton/*.h
sed -i $STR aloe/standalone/*.c

STR3=s/$SOURCE0/$1/g
printf 'STR3=%s\n' "$STR3"
sed -i $STR3 .settings/*
sed -i $STR3 .cproject
sed -i $STR3 .project

#Change Files Names
mv matlab/modulename.c matlab/$1.c
mv src/modulename.c src/$1.c
mv src/modulename.h src/$1.h
mv src/modulename_interfaces.h src/$1_interfaces.h
mv src/modulename_params.h src/$1_params.h
mv src/modulename_functions.c src/$1_functions.c
mv src/modulename_functions.h src/$1_functions.h
rm src/modulename*.*
rm matlab/modulename*.*
pwd
ls -al
cd ../..
pwd
./scripts/update_modules.pl





