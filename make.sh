#!/bin/bash

# 获取当前目录下的所有构建脚本
scripts=($(ls ./bash/build*.sh 2> /dev/null))

# 检查是否有脚本文件
if [ ${#scripts[@]} -eq 0 ]; then
    echo "没有找到任何构建脚本。"
    exit 1
fi

# 打印脚本列表
echo "请选择要运行的构建脚本："
for i in "${!scripts[@]}"; do
    echo "$((i+1)). ${scripts[i]}"
done

# 读取用户输入
read -p "请输入数字选择脚本: " choice

# 检查输入是否有效
if [[ ! "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -le 0 ] || [ "$choice" -gt ${#scripts[@]} ]; then
    echo "无效的选择。"
    exit 1
fi

# 获取选中的脚本
selected_script=${scripts[$((choice-1))]}

# 运行选中的脚本
echo "正在运行 $selected_script ..."
bash "$selected_script"
