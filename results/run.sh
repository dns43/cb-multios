
echo $1
if [ "$1" = "" ] #then
#if [ -z $1 ]
then
	echo "you need to specify which challenge";
	exit
fi

cd ..
var=$(pwd)
#echo $var
cd $var/results/$1




#if [ -f sample.input ];
if [[ $(ls sample.input) = 'sample.input' ]]
then
	mkdir manual/
	cd manual/
#	$var/build/challenges/$1/$1 < $var/results/$1/sample.input
	cd ..
else
	mkdir pov/
	cd pov
	python2 $var/tools/tester.py --povs -c $1
fi

