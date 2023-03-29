#!/bin/sh

ALOE=aloe
ALOE2=aloe_
PACKAGES=packages
VERSION=$1

if ! test -n "$VERSION"
then
	echo "Need version as parameter"
	exit
fi

echo "Building distribution..."
make dist

mv $ALOE-$VERSION.tar.gz $ALOE2$VERSION.orig.tar.gz
tar xzvf $ALOE2$VERSION.orig.tar.gz
cd $ALOE-$VERSION

echo "Creating .deb..."
cp -fr ../debian ./
debuild -sd -us -uc
cd ..

rm -fr $ALOE-$VERSION

if ! test -d $PACKAGES
then
	mkdir $PACKAGES
fi
mv $ALOE2* $PACKAGES

echo "Converting to rpm..."
cd $PACKAGES
alien -r -c -k *.deb

echo "Comitting source package..."
mv $ALOE2$VERSION.orig.tar.gz $ALOE-$VERSION.tar.gz
svn add $ALOE-$VERSION.tar.gz
svn add *.deb
svn add *.rpm
svn ci
svn rm http://flexnets.upc.edu/svn/phal/tags/$ALOE-$VERSION.tar.gz 
svn cp http://flexnets.upc.edu/svn/phal/trunk/packages/$ALOE-$VERSION.tar.gz http://flexnets.upc.edu/svn/phal/tags

echo "Done."
