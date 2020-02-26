

cd build
echo '[DEBUG] making target '$1
make $1
echo '[DEBUG] making patched version of target '$1
make $1_patched

echo '[DEBUG] making all povs for target '$1
cd ../pov
for f in `ls ../challenges/$1/ | grep pov_`
do
    make $1_$f
done

cd ..

echo '[DEBUG] copying all povs for target '$1 'from pov/ to build/'
for f in `ls pov/challenges/$1/ | grep pov`
do
    cp pov/challenges/$1/$f build/challenges/$1
done

echo '[DEBUG] DONE'