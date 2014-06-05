#!/bin/bash

#
# Script to prepare the environment for the software configuration db
#
#

setup_env () {
    . SetupProject.sh --silent Tools --use Ariadne

    if [ "$SetupProjectStatus" != 0 ] ; then
        echo "ERROR: Cannot prepare the environment to use Ariadne."
        exit 1
    fi
}

command -v cern-get-sso-cookie >/dev/null 2>&1 || { echo >&2 "I need 'cern-get-sso-cookie' package to run but it's not installed. Aborting."; exit 1; }
export cookie_file=/tmp/$USER/ssocookie-ariadne.txt
cern-get-sso-cookie --krb -u https://ariadne-lhcb.cern.ch/ -o $cookie_file
if [ $? != 0 ] ; then
    printf "WARNING: Error encountered while acquiring CERN SSO cookie for Ariadne. Attempting to connect with no SSO..\n\n"
fi    

setup_env