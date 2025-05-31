#!/bin/sh
export  TZ="Asia/Shanghai"
echo "#ifndef __VERSION_H__" > version.h
echo "#define __VERSION_H__" >> version.h
echo "" >> version.h
date +"#define MAINCTRL_VERSION \"%F,%T\"" >> version.h
echo "#define GIT_VERSION \"$(git log -n 1 --pretty=format:"%h")\"" >> version.h
echo "" >> version.h
echo "void version_show(void);" >> version.h
echo "" >> version.h
echo "#endif /* __VERSION_H__ */" >> version.h
mv version.h include/
