BIN="/home/utnso/logs/$2"
mkdir -p $BIN
[ -f cliente/*.log ] && mv -v cliente/*.log $BIN
[ -f app/*.log ] && mv -v app/*.log $BIN
[ -f restaurante/*.log ] && mv -v restaurante/*.log $BIN
[ -f comanda/*.log ] && mv -v comanda/*.log $BIN
[ -f sindicato/*.log ] && mv -v sindicato/*.log $BIN