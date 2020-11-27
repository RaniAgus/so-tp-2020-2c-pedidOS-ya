#!/bin/bash

bold=$(tput bold)
normal=$(tput sgr0)
underline=`tput smul`
nounderline=`tput rmul`

if [[ "$@" =~ (-h|-H|--help) ]]; then
  echo "
${bold}NAME${normal}
    ${bold}deploy.sh${normal} - script to deploy sisoputnfrba's TP.
${bold}SYNOPSIS${normal}
    ${bold}deploy.sh${normal} [${bold}--lib${normal}=${underline}library${nounderline}] [${bold}--dependency${normal}=${underline}dependency${nounderline}] [${bold}--project${normal}=${underline}project${nounderline}] ${underline}repository${nounderline}
${bold}DESCRIPTION${normal}
    The ${bold}deploy.sh${normal} utility is to ease the deploy process.
    The options are as follows:
    ${bold}-t | --target${normal}       Changes the directory where the script is executed. By default it will be the current directory.
    ${bold}-l | --lib${normal}          Adds an external dependency to build and install.
    ${bold}-d | --dependency${normal}   Adds an internal dependency to build and install from the repository.
    ${bold}-p | --project${normal}      Adds a project to build from the repository.
${bold}COMPATIBILITY${normal}
    The project must have the following scructure:
      project1/
       ╰─ makefile
      project2/
       ╰─ makefile
      sharedlibrary/
       ╰─ makefile
${bold}EXAMPLE${normal}
      ${bold}deploy.sh${normal} ${bold}-l${normal}=sisoputnfrba/ansisop-parser ${bold}-d${normal}=sockets ${bold}-p${normal}=consola ${bold}-p${normal}=kernel ${bold}-p${normal}=memoria ${underline}tp-2017-1C-exampleRepo${nounderline}
  "
  exit
fi


length=$(($#-1))
OPTIONS=${@:1:$length}
REPONAME="${!#}"
CWD=$PWD

for i in "${@:2}"
do
  case $i in
    -t=*|--target=*)
      CWD="${i#*=}"
    ;;
    *)
    ;;
  esac
done

LIBRARIES=()
DEPENDENCIES=()
PROYECTS=()

echo -e "\n\nInstalling commons libraries...\n\n"

COMMONS="so-commons-library"
git clone "https://github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD


for i in $OPTIONS
do
    case $i in
        -l=*|--lib=*)
          LIBRARIES+=("${i#*=}")
        ;;
        -d=*|--dependency=*)
          DEPENDENCIES+=("${i#*=}")
        ;;
        -p=*|--project=*)
          PROYECTS+=("${i#*=}")
        ;;
        *)
        ;;
    esac
done


echo -e "\n\nCloning external libraries\n\n"


for i in "${LIBRARIES[@]}"
do
  git clone "https://github.com/${i}.git" $i
  make
  cd $CWD
done

git clone "https://github.com/sisoputnfrba/${REPONAME}.git"
cd $REPONAME
PROJECTROOT=$PWD

echo -e "\n\nBuilding dependencies\n\n"

for i in "${DEPENDENCIES[@]}"
do
  echo -e "Building ${i}"
  cd $i
  make install
  cd $PROJECTROOT 
done

echo -e "\n\nBuilding projects...\n\n"

for i in "${PROYECTS[@]}"
do
  cd $i
  make
  cd $PROJECTROOT
done

echo -e "\n\nDeploy done!\n\n"