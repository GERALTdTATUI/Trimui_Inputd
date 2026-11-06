#!/bin/bash

HOST="root@10.19.0.111"
PASS="tina"
BIN="./build/trimui_inputd/bin/trimui_inputd"

echo "Compiling a bew binary"
docker run --privileged --rm tonistiigi/binfmt --install arm64
docker compose up --build


echo "Pushing binary to the device"
sshpass -f <(printf '%s\n' $PASS) ssh $HOST rm /root/trimui_inputd;
sshpass -f <(printf '%s\n' $PASS) scp $BIN $HOST:/root/trimui_inputd;


echo "Restarting input daemon"
sshpass -f <(printf '%s\n' $PASS) ssh $HOST killall trimui_inputd; sleep 2
sshpass -f <(printf '%s\n' $PASS) ssh $HOST /root/trimui_inputd &

echo "Finished"


exit 0