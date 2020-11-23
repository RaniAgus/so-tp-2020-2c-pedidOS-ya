./config.sh com serv 5001
./config.sh app com localhost 5001
./config.sh app serv 5004
./config.sh app cpu 4
./config.sh app gm 1
./config.sh app fifo
./config.sh app rep "[1|1,7|8,1|4]" [50,50,90] [2,2,1]
./config.sh app pl [Milanesa]
./config.sh app pos 5 5
./config.sh cli localhost 5004