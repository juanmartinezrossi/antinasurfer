#!/bin/bash

SOURCE0=module_template3
printf 'SOURCE0=%s\n' "$SOURCE0"
pwd
rm -r $SOURCE0.ALOE1.6SK15/
ls -al module_template3.ALOE1.6SK15
pwd
cp -fr ../modules/$SOURCE0/ $SOURCE0.ALOE1.6SK15/
ls -al module_template3.ALOE1.6SK15


#Clean Reference Template
pwd
#rm module_template.ALOE1.6/debug_make/Makefile
rm $SOURCE0.ALOE1.6SK15/test/libmodule_template3.a
rm $SOURCE0.ALOE1.6SK15/test/module_template3
rm $SOURCE0.ALOE1.6SK15/test/*.o
rm $SOURCE0.ALOE1.6SK15/test/.deps/*.Po
rm $SOURCE0.ALOE1.6SK15/test/*~
rm $SOURCE0.ALOE1.6SK15/test/*.*~

rm $SOURCE0.ALOE1.6SK15/lnx_make/Makefile
rm $SOURCE0.ALOE1.6SK15/lnx_make/Makefile.in
rm $SOURCE0.ALOE1.6SK15/lnx_make/Makefile.am~

rm $SOURCE0.ALOE1.6SK15/lnx_make/bin/module_template3
#rm $SOURCE0.ALOE1.6SK15/matlab/module_template3.mexglx
#rm $SOURCE0.ALOE1.6SK15/matlab/module_template3.mex
rm $SOURCE0.ALOE1.6SK15/matlab/*.*~

rm $SOURCE0.ALOE1.6SK15/src/*.om
rm $SOURCE0.ALOE1.6SK15/src/*.o
rm $SOURCE0.ALOE1.6SK15/src/.deps/*.Po
rm $SOURCE0.ALOE1.6SK15/src/*.*~

SOURCE1=$SOURCE0.ALOE1.6SK15
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

STR=s/module_template3/$1/g
#STR2=s/$SOURCE1/$1/g
printf 'STR=%s\n' "$STR"
printf 'STR2=%s\n' "$STR2"

sed -i $STR test/Makefile
sed -i $STR test/test_generate.c
sed -i $STR lnx_make/Makefile.am
sed -i $STR src/*.c
sed -i $STR src/*.h
sed -i $STR matlab/module_template3.c
sed -i $STR matlab/verify.m

STR3=s/$SOURCE0/$1/g
printf 'STR3=%s\n' "$STR3"
sed -i $STR3 .settings/*
sed -i $STR3 .cproject
sed -i $STR3 .project

#Change Files Names
mv matlab/module_template3.c matlab/$1.c
mv matlab/module_template3.mex matlab/$1.mex
mv matlab/module_template3.mexglx matlab/$1.mexglx

mv params/module_template3.params params/$1.params

mv src/module_template3.c src/$1.c
mv src/module_template3.h src/$1.h
mv src/module_template3_functions.c src/$1_functions.c
mv src/module_template3_functions.h src/$1_functions.h
mv src/module_template3_params.h src/$1_params.h
mv src/module_template3_stats.h src/$1_stats.h

rm src/*.OLD
rm src/*.*~

rm test/test_generate.o
rm test/module_template3
rm test/*~

pwd
ls -al
cd ../..
pwd

#Update modules Makefiles
#./scripts/update_modules.pl
#make clean
#autoconf
#./configure LIBS='-lpthread -lboost_system'


