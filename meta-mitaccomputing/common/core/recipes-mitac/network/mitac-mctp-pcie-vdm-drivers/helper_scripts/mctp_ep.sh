mctp route add $1 via mctppci0
mctp neigh add $1 dev mctppci0 lladdr $2
