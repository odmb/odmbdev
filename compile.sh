#!/bin/bash
current_path=`pwd -P`
if [[ $current_path == *TriDAS* ]]; then
    export BUILD_HOME=`echo $current_path | sed -r 's|(TriDAS).*$|\1|'`
elif [[ $current_path == *local/data/odmb_ucsb* ]]; then
    export BUILD_HOME=/home/cscme11/TriDAS
else
    export BUILD_HOME=`dirname ${current_path}`/TriDAS
fi
export XDAQ_ROOT=/opt/xdaq
make
make install
