#StartEID:   8, Binding: 0x02, Addr: 0x00:0x13
#StartEID:   9, Binding: 0x02, Addr: 0x00:0x12
#StartEID:  10, Binding: 0x02, Addr: 0x00:0x10
#StartEID:  11, Binding: 0x02, Addr: 0x00:0x87

mctp route add 8 via mctppci0
mctp neigh add 8 dev mctppci0 lladdr 0x00:0x13

mctp route add 9 via mctppci0
mctp neigh add 9 dev mctppci0 lladdr 0x00:0x12

mctp route add 10 via mctppci0
mctp neigh add 10 dev mctppci0 lladdr 0x00:0x10

mctp route add 11 via mctppci0
mctp neigh add 11 dev mctppci0 lladdr 0x00:0x87
