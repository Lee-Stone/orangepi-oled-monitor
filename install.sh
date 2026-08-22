#!/usr/bin/env bash
set -e

# ===== 检查是否以 root 运行 =====
if [ "$(id -u)" -ne 0 ]; then
    echo "错误：请使用 sudo 运行："
    echo "  sudo $0 -enable"
    echo "  sudo $0 -disable"
    exit 1
fi

# ===== 获取脚本所在目录 =====
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MAIN="$SCRIPT_DIR/main"
SERVICE_NAME="oled-monitor"
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

usage() {
    echo "用法: sudo $0 {-enable|-disable}"
    exit 1
}

[ $# -eq 1 ] || usage

case "$1" in
    -enable|-disable) ;;
    *) usage ;;
esac

# ===== 杀掉正在运行的 OLED 进程 =====
kill_oled() {
    pkill -f "$MAIN" 2>/dev/null || true
    sleep 1
}

if [ "$1" = "-enable" ]; then
    echo "[1/4] 编译 OLED 工程..."
    cd "$SCRIPT_DIR"
    make clean >/dev/null 2>&1 || true
    make || { echo "编译失败！"; exit 1; }

    echo "[2/4] 停止已存在的 OLED 进程..."
    kill_oled

    echo "[3/4] 创建开机自启动服务..."
    cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=Orange Pi OLED Terminal Monitor
After=network.target

[Service]
Type=simple
ExecStart=$MAIN
WorkingDirectory=$SCRIPT_DIR
Restart=always
RestartSec=5
User=root

[Install]
WantedBy=multi-user.target
EOF

    systemctl daemon-reload
    systemctl enable "$SERVICE_NAME" >/dev/null 2>&1
    systemctl start "$SERVICE_NAME"

    echo "[4/4] OLED 监视器已启动，并已设置开机自启动"
    systemctl is-active "$SERVICE_NAME" && echo "完成！"

else
    echo "[1/3] 停止 OLED 进程..."
    kill_oled

    echo "[2/3] 关闭开机自启动服务..."
    if [ -f "$SERVICE_FILE" ]; then
        systemctl stop "$SERVICE_NAME" 2>/dev/null || true
        systemctl disable "$SERVICE_NAME" 2>/dev/null || true
    fi

    echo "[3/3] 删除开机自启动配置..."
    rm -f "$SERVICE_FILE"
    systemctl daemon-reload

    echo "完成！OLED 监视器已停止，开机自启动已删除"
fi
