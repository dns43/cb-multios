POVs=`ls challenges/$1/ | grep pov_`
echo $POVs

for f in $POVs
do
	echo `ls challenges/$1/*$f`
	echo $1$f
done

#for f in pov/challenges/$1/Sensr_pov_*
#do
#	echo $f
#done
