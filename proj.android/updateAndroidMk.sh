#!/bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

cd $DIR

./updateAndroidMk.py -o jni/Android.mk -t Android.mk.template -b jni ../Classes

