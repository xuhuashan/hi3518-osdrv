#!/bin/sh

tc="v100"

if [ $# -gt 0 ]; then
  if [ "x${1}" = "xv100" -o "x${1}" = "xv200" ]; then
    tc=$1
    shift
  fi
fi

if [ "x${tc}" = "xv100" ]; then
  if ! which arm-hisiv100nptl-linux-gcc > /dev/null 2>&1; then
    tc_path=/opt/hisi-linux-nptl/arm-hisiv100-linux/target/bin
    export PATH=${PATH}:${tc_path}
  fi
  cross=arm-hisiv100nptl-linux
elif [ "x${tc}" = "xv200" ]; then
  if ! which arm-hisiv200-linux-gcc > /dev/null 2>&1; then
    tc_path=/opt/hisi-linux/x86-arm/arm-hisiv200-linux/target/bin
    export PATH=${PATH}:${tc_path}
  fi
  cross=arm-hisiv200-linux
fi


if [ $# -gt 0 ]; then
  target=$*
else
  target=all
fi

make OSDRV_CROSS=${cross} CHIP=hi3518a ${target}

