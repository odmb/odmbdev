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
export XDAQ_PLATFORM=x86_64_slc6

echo
echo "Environment variables:"
echo "    BUILD_HOME         = $BUILD_HOME"
echo "    XDAQ_ROOT          = $XDAQ_ROOT"
echo "    XDAQ_OS            = $XDAQ_OS"
echo "    XDAQ_PLATFORM      = $XDAQ_PLATFORM"
echo "    XDAQ_DOCUMENT_ROOT = $XDAQ_DOCUMENT_ROOT"
echo "    ROOTSYS            = $ROOTSYS"
echo "    LD_LIBRARY_PATH    = $LD_LIBRARY_PATH"
echo



tmp_file_build="makefile_errors_and_warnings_temporary_collision_free_long_unambiguous_name_build.log"
tmp_file_install="makefile_errors_and_warnings_temporary_collision_free_long_unambiguous_name_install.log"

rm -f core.*
make -j -k 2> >(tee $tmp_file_build >&2)
make -j -k install 2> >(tee $tmp_file_install >&2)

echo

if [[ -s $tmp_file_build || -s $tmp_file_install ]] ; then
    echo
    echo "ERRORS AND WARNINGS:"
    if [[ -s $tmp_file_build ]] ; then
	cat $tmp_file_build >&2
    fi
    if [[ -s $tmp_file_install ]] ; then
	cat $tmp_file_install >&2
    fi
    rm -rf $tmp_file_build
    rm -rf $tmp_file_install
    echo
    exit 1
else
    echo
    echo "Compiled successfully without errors or warnings!"
    echo
    rm -rf $tmp_file_build
    rm -rf $tmp_file_install
    exit 0
fi
