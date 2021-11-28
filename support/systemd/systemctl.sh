if [ 1 -ne $# ]
then
    echo "You must specify argument : ./systemctl.sh  {start|stop|force-stop|restart|force-reload|status}"

    exit;
fi

systemctl $1 vepc-mmed
systemctl $1 vepc-sgwd
systemctl $1 vepc-pgwd
systemctl $1 vepc-hssd
systemctl $1 vepc-pcrfd
