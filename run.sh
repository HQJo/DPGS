# ./main --input data/$1.txt --dataset $1 --turn 30 --seed 0 --b 8
# ratios=(0.50 0.25 0.12)
ratios=(0.8 0.6 0.4 0.2 0.1)
for r in ${ratios[@]}
do
    python main.py $1 /data/$1/adj.npz --ratio $r
done
