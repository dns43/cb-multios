#conda activate cgc
python tester.py -c $1 --povs
cd results
mkdir $1
cp ../*.csv $1

