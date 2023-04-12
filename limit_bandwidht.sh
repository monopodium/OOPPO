tc qdisc add dev lo root handle 1: htb

tc class add dev lo parent 1: classid 1:20 htb rate 512mibit
tc qdisc add dev lo parent 1:20 handle 20: sfq perturb 10
tc filter add dev lo parent 1: protocol ip prio 1 basic match 'cmp(u16 at 0 layer transport eq 18000)' flowid 1:20


tc class add dev lo parent 1: classid 1:30 htb rate 512mibit
tc qdisc add dev lo parent 1:30 handle 30: sfq perturb 10
tc filter add dev lo parent 1: protocol ip prio 1 basic match 'cmp(u16 at 0 layer transport eq 18000)' flowid 1:30