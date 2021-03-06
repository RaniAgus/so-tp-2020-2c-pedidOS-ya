BIN=""
OPTION=""
MODULE=""
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
SHARED_LIB_DIR=$DIR"/cshared/"

if [ $# -eq 0 ];then
    echo "Expected:[OPTION] [MODULE] [PARAMS]"
    echo "Enter one module. Allowed:"
    echo "      *app"
    echo "      *cliente"
    echo "      *comanda"
    echo "      *restaurante"
    echo "      *sindicato"
    echo "      *tests"
    echo ""
    echo "For each module, you can add one of the follow options:"
    echo "      valgrind - Execute valgrind (memcheck mode)"
    echo "      helgrind - Execute valgrind (helgrind mode)"
    echo "      gdb      - Execute gdb"
    echo "      release  - Execute the release version of the target"
else
    if [ "$1" == "valgrind" ];then
        OPTION="valgrind --leak-check=full --log-file=valgrind.log"
        shift;
    elif [ "$1" == "helgrind" ];then
        OPTION="valgrind --tool=helgrind --log-file=helgrind.log"
        shift;
    elif [ "$1" == "gdb" ];then
        OPTION="gdb --args"
        shift;
    elif [ "$1" == "release" ];then
        BIN=""
        shift;
    fi

    MODULE="$1"
    shift;

    cd $MODULE && make && LD_LIBRARY_PATH=$SHARED_LIB_DIR $OPTION ./$BIN$MODULE $@
fi
