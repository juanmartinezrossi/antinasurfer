#!/bin/bash

SOURCE0=module_templateSK18
printf 'SOURCE0=%s\n' "$SOURCE0"

rm -r $SOURCE0.ALOE1.6_SK18/
cp -fr ../modules/$SOURCE0/ $SOURCE0.ALOE1.6_SK18/

#Clean Reference Template
pwd
			#rm $SOURCE0.ALOE1.6/debug_make/libmodulename.a
rm $SOURCE0.ALOE1.6_SK18/lnx_make/Makefile
rm $SOURCE0.ALOE1.6_SK18/lnx_make/Makefile.in
rm $SOURCE0.ALOE1.6_SK18/lnx_make/bin/module_templateSK18
			#rm $SOURCE0.ALOE1.6_SK18/matlab/modulename.mexglx
			#rm $SOURCE0.ALOE1.6_SK18/matlab/modulename.mex
rm $SOURCE0.ALOE1.6_SK18/src/*.om
rm $SOURCE0.ALOE1.6_SK18/src/*.o


SOURCE1=$SOURCE0.ALOE1.6_SK18
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

STR=s/module_templateSK18/$1/g
#STR2=s/$SOURCE1/$1/g
printf 'STR=%s\n' "$STR"
#printf 'STR2=%s\n' "$STR2"

sed -i $STR lnx_make/Makefile.am
sed -i $STR src/*.c
sed -i $STR src/*.h

STR3=s/$SOURCE0/$1/g
printf 'STR3=%s\n' "$STR3"
sed -i $STR3 .settings/*
sed -i $STR3 .cproject
sed -i $STR3 .project

#Change Files Names
mv src/module_templateSK18.c src/$1.c
#mv src/module_templateSK18.h src/$1.h
#mv src/module_templateSK18_interfaces.h src/$1_interfaces.h
#mv src/module_templateSK18_params.h src/$1_params.h
mv src/module_templateSK18_functions.c src/$1_functions.c
mv src/module_templateSK18_functions.h src/$1_functions.h
#rm src/module_templateSK18*.*
pwd
ls -al
cd ../..
pwd
./scripts/update_modules.pl





