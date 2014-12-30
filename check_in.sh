#! /bin/bash -e
git commit -am "$1"
git pull
git push origin master
