#!/bin/bash

function Help {
  echo "Usage: ./task.sh [OPTION] ..."
  echo "Скрипт конкатенирует файлы наименьшего размера до тех пор, пока суммарный размер не превысит размеров блока файловой системы"
  echo ""
  echo "Обязательные параметры:"
  echo -e "-p,  --path=[ФАЙЛ],            Указывает, в какой файл"
  echo -e "                               будет производиться конкатенация"
  echo ""
  echo "Необязательные параметры:"
  echo -e "-s,  --source-path=[КАТАЛОГ]   Указывает, из какого"
  echo -e "                               каталога будут браться файлы"
  echo -e "-c,  --clear,                  Удаляет файл-результат после работы скрипта"
}


function GetSize {
  if [ $# -ne 1 ] 
  then
    echo [DEBUG: func GetSize] Передан неверный параметр
    exit 2
  else
    echo $(wc -c $1 | cut -d" " -f1)
  fi
}

function Main {
  local size_of_block=`stat -f -c %s /`
  local cat_path=""
  local source_path="*"
  local clear=false

  while [ $# -gt 0 ] 
  do
    key=$1

    case $key in
      -p | --path)
        cat_path="$2"
	shift
	;;
      -c | --clear)
	clear=true
	;;
      -s | --source-path)
	source_path=$2
        shift
	;;
      *)
        Help
	exit 2
	;;
    esac
    shift
  done

  if [ -z $cat_path ]
  then
    Help
    exit 2
  fi
  
  # создаём пустой файл (очищаем, если он был полным)
  > $cat_path

  # конкатенируем, пока размер не превысит размер блока
  for file in `ls -rS $source_path`
  do
    if [ "$source_path" = "*" ]
    then
      path_file=$file
    else
      path_file=$source_path/$file
    fi
    echo -n "Добавлен $file размера $(GetSize $path_file), размер $cat_path: $(GetSize $cat_path) -> "
    cat $path_file >> $cat_path
    echo $(GetSize $cat_path)
    if [ $(GetSize $cat_path) -gt $size_of_block ]
    then
      break
    fi
  done

  # удаляем файл, если был введён флаг -c
  if [ -e $cat_path ] && [ $clear = "true" ]
  then
    echo Удаляю файл $cat_path размера $(GetSize $cat_path)...
    rm $cat_path
  fi
}


Main $@
