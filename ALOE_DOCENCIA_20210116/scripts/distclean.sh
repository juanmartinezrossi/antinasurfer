make clean
find -name '*.log' | xargs rm -f -
find -name '*.params' | xargs rm -f -
find -name '.deps' | xargs rm -rf -
find -name '.dirstamp' | xargs rm -rf -
