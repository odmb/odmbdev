#!/bin/bash

shopt -s nullglob

current_path=`pwd -P`
if [[ $current_path == *TriDAS* ]]; then
    export BUILD_HOME=`echo $current_path | sed -r 's|(TriDAS).*$|\1|'`
elif [[ $current_path == *local/data/odmb_ucsb* ]]; then
    export BUILD_HOME=/home/cscme11/TriDAS
else
    export BUILD_HOME=`dirname ${current_path}`/TriDAS
fi
export XDAQ_ROOT=/opt/xdaq
export XDAQ_OS=linux
export XDAQ_PLATFORM=x86_64_slc5

tmp_file="makefile_errors_and_warnings_temporary_collision_free_long_unambiguous_name.log"

make -j -k 2> >(tee $tmp_file >&2)

echo

if [[ -s $tmp_file ]] ; then
    echo
    echo "ERRORS AND WARNINGS:"
    cat $tmp_file >&2
    rm -rf $tmp_file
    exit 1
else
    make -j -k install 2> >(tee $tmp_file >&2)
    if [[ -s $tmp_file ]] ; then
	echo
	echo "ERRORS AND WARNING:"
	cat $tmp_file >&2
	rm -rf $tmp_file
	exit 2
    else
	echo
	echo "Compiled successfully without errors or warnings!"
	exit 0
    fi
fi
