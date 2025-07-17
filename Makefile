VERSION	?= $(shell date +%Y%m%d)-snapshot

# 系统类型
OS_TYPE := LINUX

#头文件
INC_PATH := -I ./include

# 排除的不需要编译的目录及其子目录
EXCLUDE_DIR := SI* .git* include* siPro* build

# 编译选项
#CROSS_COMPILE ?= arm-linux-gnueabihf-
#CROSS_COMPILE ?= arm-linux-gnueabi-
CFLAGS += $(EXTRA_CFLAGS)

ifndef CROSS_COMPILE
EXCLUDE_DIR := common/zlog
endif

ifdef CROSS_COMPILE
#外部调用的库
#LDFLAGS += -L./lib/zlog -lzlog
endif

ifdef TEST_UNIT
CFLAGS += -DTEST_UNIT=1
endif

export VERSION OS_TYPE CROSS_COMPILE

# 指定生成文件类型，可执行文件、动态库、静态库
SHARE_LIB :=
PRO_APP := mainApp
# STATIC_LIB


# 编译类型
# BUILD_TYPE ?= DEBUG
BUILD_TYPE ?= RELEASE

# shell command
PWD := $(shell pwd)

# 基本目录
TOP_DIR  := $(PWD)
SRC_TREE := $(PWD)

# 中间文件
OBJ_NAME := obj
OBJ_PATH := $(TOP_DIR)/$(OBJ_NAME)/

# 目标文件
BIN_NAME := bin
BIN_PATH := $(TOP_DIR)/$(BIN_NAME)/

# 排除编译中间文件和目标文件
EXCLUDE_DIR += $(OBJ_NAME)* $(BIN_NAME)* test .git/*

# 安装目录
INSTALL_PATH :=

# 指定编译类型
ifdef SHARE_LIB
BUILD_SHARED_LIB := $(BIN_PATH)$(SHARE_LIB)
endif

ifdef PRO_APP
BUILD_APP := $(BIN_PATH)$(PRO_APP)
endif

ifdef STATIC_LIB
BULID_STATIC_LIB := $(BIN_PATH)$(STATIC_LIB)
endif

# 获取编译子目录和当前目录, 只搜索1级目录
DIRS := $(shell find . -maxdepth 4 -type d)
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS += .
export DIRS

# 删除排除不编译的目录
EXCLUDE_PATH_NAME := $(patsubst %*, %%, $(EXCLUDE_DIR))

# 不要编译源代码的目录
SRC_DIRS := $(filter-out $(EXCLUDE_PATH_NAME), $(DIRS))

# 自动添加头文件路径
INC_PATH += $(foreach dir, $(SRC_DIRS), $(addprefix -I$(SRC_TREE)/, $(dir)))

# 源文件
C_SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
CXX_SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.cpp))

export  C_SRCS CXX_SRCS EXCLUDE_PATH_NAME BUILD_TYPE TOP_DIR SRC_TREE BIN_PATH \
        INSTALL_PATH OBJ_PATH BUILD_APP BULID_STATIC_LIB CFLAGS

include ./Makefile.template

CFLAGS += $(INC_PATH)

#add by cq
#CFLAGS += -std=c99

.SUFFIXES: .cpp .c
%.o: %.cpp
	@echo "$(CXX) compiling $<..."
	@echo "$(CFLAGS)"
	@test -d $(dir $(OBJ_PATH)$@) || mkdir -p $(dir $(OBJ_PATH)$@)
	@$(CXX) $(CFLAGS) -c -o $(OBJ_PATH)$@ $(OBJ_PATH)$<

%.o: %.c
	@echo "$(CC) compiling $<..."
	@echo "$(CFLAGS)"
	@test -d $(dir $(OBJ_PATH)$@) || mkdir -p $(dir $(OBJ_PATH)$@)
	@$(CC) $(CFLAGS) -c -o $(OBJ_PATH)$@ $<

.PHONY: all version
all: version $(BUILD_APP)  $(OBJS)

GIT_VERSION = $(shell git log -n 1 --pretty=format:"%h")
RELEASE_VERSION = $(shell git describe --tags --abbrev=0)

version:
	@echo "#ifndef __VERSION_H__" > version.h
	@echo "#define __VERSION_H__" >> version.h
	@echo "" >> version.h
	@date +"#define MAINCTRL_VERSION \"%F,%T\"" >> version.h
	@echo "#define GIT_VERSION \"$(GIT_VERSION)\"" >> version.h
	@echo "#define RELEASE_VERSION \"$(RELEASE_VERSION)\"" >> version.h
	@echo "" >> version.h
	@echo "void version_show(void);" >> version.h
	@echo "" >> version.h
	@echo "#endif /* __VERSION_H__ */" >> version.h
	@mv version.h include/

$(BUILD_APP) : $(OBJS)
	@echo "$(CXX) link target $@"
	@test -d $(dir $(BIN_PATH)) || mkdir -p $(dir $(BIN_PATH))
	@$(CC) $(CFLAGS) -o $(BUILD_APP) $(foreach obj, $(OBJS), $(OBJ_PATH)$(obj)) $(LDFLAGS)

$(BUILD_SHARED_LIB) : $(OBJS)
	@echo "$(CXX) link target $@"
	@test -d $(dir $(BIN_PATH)) || mkdir -p $(dir $(BIN_PATH))
	@$(CC) $(CFLAGS) -shared -fPIC -o $(BUILD_SHARED_LIB) $(foreach obj, $(OBJS), $(OBJ_PATH)$(obj))

$(BULID_STATIC_LIB) : $(OBJS)
	@echo "$(CXX) linking target $@"
	@test -d $(dir $(BIN_PATH)) || mkdir -p $(dir $(BIN_PATH))
	@$(AR) rcs $(BULID_STATIC_LIB) $(foreach obj, $(OBJS), $(OBJ_PATH)$(obj))

.PHONY:clean
clean:
	rm -f $(BUILD_SHARED_LIB) $(BULID_STATIC_LIB)
	rm -rf $(OBJ_PATH)/*
	rm -rf test/build
	rm -rf bin
