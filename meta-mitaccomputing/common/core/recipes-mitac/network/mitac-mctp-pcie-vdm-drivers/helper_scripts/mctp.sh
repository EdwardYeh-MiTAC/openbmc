mctp link set mctppci0 up
mctp link set mctppci0 mtu 68
mctp address add 8 dev mctppci0
mctp route add 29 via mctppci0 mtu 68
mctp neigh add 29 dev mctppci0 lladdr 18:0
