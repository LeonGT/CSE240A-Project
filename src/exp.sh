for trace in ../traces/*
do
    echo "Running static on $trace"
    bunzip2 -kc "$trace" | ./predictor --static | sed '$!d'
done
echo ""
for trace in ../traces/*
do
    echo "Running gshare:13 on $trace"
    bunzip2 -kc "$trace" | ./predictor --gshare:13 | sed '$!d'
done
echo ""
for trace in ../traces/*
do
    echo "Running tournament:9:10:10 on $trace"
    bunzip2 -kc "$trace" | ./predictor --tournament:9:10:10 | sed '$!d'
done
