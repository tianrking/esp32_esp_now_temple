#!/bin/bash

# 检查是否以 root 权限运行
if [ "$EUID" -ne 0 ]; then
    echo "请以 root 权限运行此脚本"
    exit 1
fi

# 检查参数数量
if [ $# -ne 2 ]; then
    echo "用法: $0 <interface> <mode>"
    echo "mode: 0 - monitor, 1 - managed"
    exit 1
fi

INTERFACE=$1
MODE=$2

# 切换模式的函数
switch_mode() {
    local new_mode=$1
    echo "切换到 $new_mode 模式..."
    ifconfig $INTERFACE down
    iwconfig $INTERFACE mode $new_mode
    ifconfig $INTERFACE up
    if [ "$new_mode" = "managed" ]; then
        systemctl restart NetworkManager
    else
        systemctl stop NetworkManager
    fi
    echo "已切换到 $new_mode 模式"
}

# 根据输入参数切换模式
case $MODE in
    0)
        switch_mode monitor
        ;;
    1)
        switch_mode managed
        ;;
    *)
        echo "无效的模式参数。请使用 0 表示 monitor 模式，1 表示 managed 模式。"
        exit 1
        ;;
esac

# 显示新的模式
NEW_MODE=$(iwconfig $INTERFACE | grep Mode | awk '{print $4}' | cut -d':' -f2)
echo "当前模式: $NEW_MODE"
