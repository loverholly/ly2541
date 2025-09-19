#!/bin/sh
APP=$(readlink -f "$(dirname "$0")/mainApp")   # 脚本同目录下的 mainapp
exec "$APP" "$@"                               # 把脚本所有参数原样传进去
