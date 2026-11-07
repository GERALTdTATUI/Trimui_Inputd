#!/bin/bash

# Mensagem de ajuda (Help)
ajuda() {
    echo "Uso: $0 [-u USER] [-p PASSWORD] [-h HOST_IP]"
    echo "  -u        Sets console user to send files (defaults root)"
    echo "  -p        Console user password (defaults user)"
    echo "  -h        Console ip address"
    echo "  -i        Install the new binary into the system"
    exit 1
}

# O ":" antes das letras ativa o modo silencioso de erro.
# O ":" DEPOIS de uma letra (como u: e p:) indica que essa flag EXIGE um argumento.
while getopts ":u:p:h:i" opt; do
  case ${opt} in
    u ) u="${OPTARG}" ;;
    p ) p="${OPTARG}" ;;
    h ) h="${OPTARG}" ;;
    b ) b="${OPTARG}" ;;
    i ) INSTALL=true ;;
    * ) echo "Opção inválida"; ajuda ;;
  esac
done

TRIMUI_USER="${u:-root}"
TRIMUI_PASS="${p:-tina}"
TRIMUI_HOST="${h:-10.19.0.111}"
TRIMUI_SSHS="$TRIMUI_USER@$TRIMUI_HOST"

TRIMUI_INPD="/root/trimui_inputd"
TRIMUI_BIND="${b:-build/trimui_inputd/bin/trimui_inputd}"

if [ $INSTALL ]; then
  TRIMUI_INPD="/usr/trimui/bin/trimui_inputd"
fi



echo "Compiling a bew binary"
docker run --privileged --rm tonistiigi/binfmt --install arm64
docker compose up --build

echo "Stopping the input damon"
sshpass -f <(printf '%s\n' $TRIMUI_PASS) ssh $TRIMUI_SSHS killall trimui_inputd 2>&1 > /dev/null; sleep 2

echo "Pushing binary to the device"
sshpass -f <(printf '%s\n' $TRIMUI_PASS) ssh $TRIMUI_SSHS rm "$TRIMUI_INPD.bak" 2>&1 > /dev/null;
sshpass -f <(printf '%s\n' $TRIMUI_PASS) ssh $TRIMUI_SSHS mv $TRIMUI_INPD "$TRIMUI_INPD.bak" 2>&1 > /dev/null;
sshpass -f <(printf '%s\n' $TRIMUI_PASS) scp $TRIMUI_BIND $TRIMUI_SSHS:$TRIMUI_INPD 2>&1 > /dev/null;

echo "Restarting input daemon"
sshpass -f <(printf '%s\n' $TRIMUI_PASS) ssh $TRIMUI_SSHS $TRIMUI_INPD &
sleep 2

echo "Finished"
echo "Try testing your device inputs with a hardware testing app"


exit 0