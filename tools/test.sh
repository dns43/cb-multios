# Just a quick script to auto test all challenges
if [ "$1" == 'all' ]; then
    for f in `ls ../challenges/`
    do
        rm -rf results/$f
        mkdir -p results/$f
        echo $f
        python tester.py -c $f --povs    
        echo 'Done'
    done
elif [ "$1" == 'test' ]; then
    for f in `ls ../challenges`
    do
        echo $f "asd "
    done
else
    rm -rf results/$1
    mkdir -p results/$1
    python tester.py -c $1 --povs    
    
    # note that we do not copy final_labels.csv here
    # because every invocation of tester.py may generate multiple final_labels.csv
    # instead we adjust tester.py itself
    # cp ../*.csv $1
fi
