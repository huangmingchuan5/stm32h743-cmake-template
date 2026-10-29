#!/usr/bin/env bash
# ============================================================
# 一键编译脚本（可选烧录）
#   ./run.sh          仅编译，生成 elf / bin / hex
#   ./run.sh flash    编译后立即用 OpenOCD 烧录
# ============================================================
set -e

cd "$(dirname "$0")"

# ---------- 工具链探测：PATH 里找不到时回退到本地安装目录 ----------
TOOLCHAIN_ROOT="/d/Embedded_Toolchain"
command -v arm-none-eabi-gcc >/dev/null 2>&1 || \
    export PATH="$TOOLCHAIN_ROOT/arm-none-eabi-gcc/15.3.rel2/bin:$PATH"
command -v cmake >/dev/null 2>&1 || \
    export PATH="$TOOLCHAIN_ROOT/cmake/4.4.5/bin:$PATH"
command -v ninja >/dev/null 2>&1 || \
    export PATH="$TOOLCHAIN_ROOT/ninja/1.13.2:$PATH"
command -v openocd >/dev/null 2>&1 || \
    export PATH="$TOOLCHAIN_ROOT/xpack-openocd/0.12.0-7/xpack-openocd-0.12.0-7/bin:$PATH"

ELF_NAME="Cmake_2_Projects_Template"
BUILD_DIR="build"

# ---------- 全新构建 ----------
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake \
      -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target all -- -j 8

# ---------- 生成 bin / hex ----------
arm-none-eabi-objcopy -O binary "$ELF_NAME.elf" "$ELF_NAME.bin"
arm-none-eabi-objcopy -O ihex   "$ELF_NAME.elf" "$ELF_NAME.hex"

# ---------- 兜底修正 compile_commands.json 的 MSYS 风格路径 ----------
# 本机用的是 Windows 原生 cmake，通常已是 c:/xxx，此步只是保险
sed -i "s|/c/|c:/|g; s|/d/|d:/|g; s|/e/|e:/|g; s|/f/|f:/|g;" compile_commands.json

cd ..
echo ""
echo "构建完成："
echo "  $BUILD_DIR/$ELF_NAME.elf"
echo "  $BUILD_DIR/$ELF_NAME.bin"
echo "  $BUILD_DIR/$ELF_NAME.hex"

# ---------- 可选烧录 ----------
if [ "$1" = "flash" ]; then
    echo ""
    echo "开始烧录 ..."
    openocd -f flash
fi
