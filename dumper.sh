#!/bin/sh

rm -rf ../dump/*
../criu/criu/criu dump -v4 -t $(pgrep tcp_server) --images-dir ../dump/ --tcp-established -o dump.log

