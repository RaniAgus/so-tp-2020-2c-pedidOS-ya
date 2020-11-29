if [ "$1" == "save" ];then
    BIN="/home/utnso/logs/$2"
    mkdir -p $BIN
    if [ -e cliente/*.log ];then
        cp -v cliente/*.log $BIN
    fi
    if [ -e app/*.log ];then
        cp -v app/*.log $BIN
    fi
    if [ -e restaurante/*.log ];then
        cp -v restaurante/*.log $BIN
    fi
    if [ -e comanda/*.log ];then
        cp -v comanda/*.log $BIN
    fi
    if [ -e sindicato/*.log ];then
        cp -v sindicato/*.log $BIN
    fi
fi

rm -fv cliente/*.log
rm -fv app/*.log
rm -fv restaurante/*.log
rm -fv comanda/*.log
rm -fv sindicato/*.log