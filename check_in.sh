#! /bin/bash -e
git pull
git commit -am "$1"
git push origin master
