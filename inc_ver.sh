#!/bin/bash
# author Sergey Volkhin
# date 02.07.2017

majv=`grep 'set(DUALIT_VERSION_MAJOR "[0-9]\{1,5\}")' Version.cmake | grep -o '[0-9]\{1,5\}'`
minv=`grep 'set(DUALIT_VERSION_MINOR "[0-9]\{1,5\}")' Version.cmake | grep -o '[0-9]\{1,5\}'`
patv=`grep 'set(DUALIT_VERSION_PATCH "[0-9]\{1,5\}")' Version.cmake | grep -o '[0-9]\{1,5\}'`

if [ -z "$majv" ]; then
  echo Failed to determine major version
  exit -1
fi

if [ -z "$minv" ]; then
  echo Failed to determine minor version
  exit -2
fi

if [ -z "$patv" ]; then
  echo Failed to determine patch version
  exit -3
fi

echo Old version: $majv.$minv.$patv
let patv++
echo New version: $majv.$minv.$patv

sed -i 's/set(DUALIT_VERSION_PATCH "[0-9]\{1,5\}")/set(DUALIT_VERSION_PATCH "'$patv'")/' Version.cmake
git add Version.cmake
git ci -m '[c*] Deb '$majv.$minv.$patv.

