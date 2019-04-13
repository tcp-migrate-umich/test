#!/bin/sh

../criu/criu/criu restore -v4 --images-dir ../dump/ --tcp-established -o restore.log
