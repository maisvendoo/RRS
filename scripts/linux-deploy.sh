#!/bin/bash

# Создаем дерево каталогов
export RRS_DEV_ROOT=../../RRS-1.6.0

mkdir -p ${RRS_DEV_ROOT}
mkdir -p ${RRS_DEV_ROOT}/bin
mkdir -p ${RRS_DEV_ROOT}/lib
mkdir -p ${RRS_DEV_ROOT}/modules
mkdir -p ${RRS_DEV_ROOT}/plugins

mkdir -p ${RRS_DEV_ROOT}/cfg
mkdir -p ${RRS_DEV_ROOT}/data
mkdir -p ${RRS_DEV_ROOT}/routes
mkdir -p ${RRS_DEV_ROOT}/fonts
mkdir -p ${RRS_DEV_ROOT}/themes

mkdir -p ${RRS_DEV_ROOT}/sdk

mkdir -p ${RRS_DEV_ROOT}/logs
mkdir -p ${RRS_DEV_ROOT}/screenshots

# Копируем исполняемые файлы
cp ../../bin/* ${RRS_DEV_ROOT}/bin
cp ../../lib/* ${RRS_DEV_ROOT}/lib
cp -r ../../modules/* ${RRS_DEV_ROOT}/modules
cp ../../plugins/* ${RRS_DEV_ROOT}/plugins
cp linux-start.sh ${RRS_DEV_ROOT}/bin
chmod a+x ${RRS_DEV_ROOT}/bin/linux-start.sh

# Копируем конфиги
cp -r ../cfg/* ${RRS_DEV_ROOT}/cfg

# Копируем данные
cp -r ../data/* ${RRS_DEV_ROOT}/data
cp -r ../themes/* ${RRS_DEV_ROOT}/themes
cp -r ../fonts/* ${RRS_DEV_ROOT}/fonts
cp -r ../routes/* ${RRS_DEV_ROOT}/routes

# Копируем лицензию
cp -r ../LICENSE ${RRS_DEV_ROOT}/
cp -r ../LICENSE-Russian ${RRS_DEV_ROOT}/
