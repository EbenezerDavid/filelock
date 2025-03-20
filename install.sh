#!/bin/bash

# 脚本名称：install_encrypt.sh
# 用途：自动编译、安装 encrypt 程序，并配置为开机自启动和后台运行

# 检查是否以 root 权限运行
if [ "$EUID" -ne 0 ]; then
    echo "请以 root 权限运行此脚本：sudo ./install_encrypt.sh"
    exit 1
fi

# 检查必要的工具是否安装
command -v gcc >/dev/null 2>&1 || { echo "需要安装 gcc，请运行：apt-get install gcc 或 yum install gcc"; exit 1; }
command -v systemctl >/dev/null 2>&1 || { echo "需要 systemd 支持"; exit 1; }

# 编译程序
echo "正在编译程序..."
if ! gcc -I./include main.c encrypt.c -o encrypt; then
    echo "编译失败，请检查 main.c 和 encrypt.c 是否存在且无错误"
    exit 1
fi

# 安装程序
echo "正在安装程序到 /usr/local/bin/..."
mv encrypt /usr/local/bin/encrypt
chmod +x /usr/local/bin/encrypt

# 创建 systemd 服务文件
echo "正在创建 systemd 服务文件..."
cat > /etc/systemd/system/encrypt.service << EOF
[Unit]
Description=File Protection Service
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/encrypt
Restart=always
User=root
StandardOutput=journal
StandardError=journal
# 调整重启限制
RestartSec=10
StartLimitIntervalSec=60
StartLimitBurst=3

[Install]
WantedBy=multi-user.target
EOF

# 重新加载 systemd 配置
echo "正在配置开机自启动..."
systemctl daemon-reload

# 启用并启动服务
systemctl start encrypt
systemctl enable encrypt

# 检查服务状态
echo "检查服务状态..."
systemctl status encrypt --no-pager

echo "安装完成！"
echo "使用方法："
echo "  加密文件：sudo /usr/local/bin/encrypt -l <file_or_dir>"
echo "  交互模式：sudo /usr/local/bin/encrypt"