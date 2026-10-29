# STM32H743 CMake Template

面向 **STM32H743XIHx** 的 CMake 工程模板：用 `arm-none-eabi-gcc + Ninja + OpenOCD` 替代 CubeMX 默认的 IDE 工程，开箱即可编译、烧录，并配好 VS Code 的代码提示（clangd）与断点调试（Cortex-Debug）。

工程由 STM32CubeMX 生成骨架，用户在 `User/` 下独立维护 BSP，业务代码写在 `Core/` 的 `USER CODE` 区内，重新生成代码不会丢失。

---

## 特性

- **CMake + Ninja** 构建，增量编译，不依赖任何 IDE
- **OpenOCD** 烧录，DAPLink 与 ST-Link 两条配置随时切换
- **clangd** 代码补全与跳转，不再依赖微软 C/C++ 插件的交叉编译配置
- **Cortex-Debug** 断点调试，支持变量、寄存器、外设视图
- **`User/` 目录** 独立存放 BSP，新增 `.c` 自动纳入构建，无需改 CMake
- **一键脚本** `run.sh`：编译产出 `elf/bin/hex`，加参数直接烧录
- 已开启 `printf` 浮点输出、`--gc-sections` 死代码消除、`--print-memory-usage`

## 环境要求

### 工具链

使用 [armup](https://github.com/ref42/armup) 一键安装（Windows）：

```powershell
irm https://github.com/ref42/armup/releases/latest/download/install.ps1 | iex
armup install -a --root D:\Embedded_Toolchain -j 24
```

安装内容与本文档验证过的版本：

| 工具 | 版本 |
|---|---|
| arm-none-eabi-gcc | 15.3.rel2 |
| cmake | 4.4.5 |
| ninja | 1.13.2 |
| xpack-openocd | 0.12.0 |
| clangd | 23.1.0 |
| probe-rs | 0.32.0 |

安装完成后 armup 会把工具目录加入用户 `PATH`。

### 硬件

- STM32H743 开发板（本模板按 `STM32H743XIHx` 配置）
- DAPLink（CMSIS-DAP）或 ST-Link
- 目标板独立供电，且与调试器共地

## 目录结构

```
.
├── Core/                        # CubeMX 生成的应用代码（业务逻辑写在 USER CODE 区）
│   ├── Inc/                     # main.h / HAL 配置 / 中断头
│   └── Src/                     # main.c / msp / it / syscalls
├── User/                        # 用户 BSP，独立于 CubeMX
│   ├── Inc/                     # bsp_xxx.h
│   └── Src/                     # bsp_xxx.c
├── Drivers/                     # HAL 与 CMSIS（已裁剪未使用组件，见「已知限制」）
├── cmake/
│   ├── gcc-arm-none-eabi.cmake  # 工具链文件：编译/链接参数集中在此
│   └── stm32cubemx/             # CubeMX 自动生成，切勿手改
├── .vscode/                     # clangd / CMake Tools / 构建任务 / 调试配置
├── CMakeLists.txt               # 根构建脚本（只生成一次，可放心修改）
├── flash                        # OpenOCD 烧录配置（DAPLink）
├── flash-reset                  # connect-under-reset 备用配置
├── run.sh                       # 一键编译 / 烧录
├── startup_stm32h743xx.s        # 启动文件
└── STM32H743XX_FLASH.ld         # 链接脚本
```

## 快速开始

```bash
# 编译：产出 build/Cmake_2_Projects_Template.elf / .bin / .hex
./run.sh

# 编译并烧录
./run.sh flash
```

手动方式（等价）：

```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug ..
ninja -j 8

arm-none-eabi-objcopy -O binary Cmake_2_Projects_Template.elf Cmake_2_Projects_Template.bin
arm-none-eabi-objcopy -O ihex   Cmake_2_Projects_Template.elf Cmake_2_Projects_Template.hex
```

烧录（**必须在工程根目录执行**）：

```bash
openocd -f flash
```

成功标志：

```
** Programming Started **
** Programming Finished **
** Verified OK **
** Resetting Target **
```

## VS Code 配置

参考 [armup 的说明](https://github.com/ref42/armup)，核心只有三步：

1. **安装 `clangd` 扩展**（只装 clangd 就够）——它读取 CMake 导出的 `compile_commands.json`，无需手工配置 include 路径
2. **先跑一次 Debug 或 Release 构建**，生成 `elf` 与 `compile_commands.json`
3. **安装 `Cortex-Debug` 扩展**，在 `launch.json` 里指向该 `elf` 即可断点调试

本仓库已预置以下文件，clone 后只需改路径：

| 文件 | 作用 |
|---|---|
| `.vscode/settings.json` | clangd 参数、CMake Tools、自动保存 |
| `.vscode/tasks.json` | `Ctrl+Shift+B` 编译、`flash` 任务 |
| `.vscode/launch.json` | DAPLink / ST-Link 两套调试配置 |

**必须按自己的安装位置修改的两处**（在 `.vscode/settings.json`）：

```jsonc
"clangd.path": "D:/Embedded_Toolchain/clangd/23.1.0/bin/clangd.exe",
"clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/build",
    "--query-driver=D:/Embedded_Toolchain/arm-none-eabi-gcc/15.3.rel2/bin/arm-none-eabi-gcc.exe"
]
```

同样的路径也出现在 `.vscode/launch.json` 的 `openocdPath`、`searchDir`、`armToolchainPath` 中。

`--query-driver` 是关键：clangd 需要执行一次交叉编译器来获取 newlib 的头文件搜索路径，缺少它会导致 `#include <stdint.h>` 报「找不到文件」，表现为满屏红波浪线。

验证 clangd 是否配置正确（不用打开 VS Code）：

```bash
clangd --check=Core/Src/main.c \
       --compile-commands-dir=build \
       --query-driver=<你的 gcc 绝对路径>
# 输出 All checks completed, 0 errors 即正常
```

## 新增 BSP 文件

`User/Src/*.c` 由 `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` 自动收集，**新增文件不需要改 CMakeLists**。头文件放在 `User/Inc/`，已在 `target_include_directories` 中。

若想改成显式列出（CMake 官方推荐做法），把根 `CMakeLists.txt` 里的 `target_sources(... ${USER_SOURCES})` 换成：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Src/bsp_led.c
)
```

> 注意：`cmake/stm32cubemx/CMakeLists.txt` 每次 CubeMX GENERATE CODE 都会被整体重写，**不要在那里加用户源文件**。根目录的 `CMakeLists.txt` 只生成一次（文件头注释已说明），可放心修改。

## 常见问题

| 现象 | 原因与处理 |
|---|---|
| `Error: Can't find flash` | 不在工程根目录执行。`openocd -f flash` 只查当前目录，先 `cd` 回根目录 |
| `Error connecting DP: cannot read IDR` | 优先查**目标板是否上电**，其次 GND 共地、SWDIO/SWCLK 是否接反。100 kHz 低速仍失败即为物理层问题 |
| 代码全屏红波浪线 | clangd 未找到 `compile_commands.json`、缺 `--query-driver`、或索引缓存过期（`Ctrl+Shift+P → clangd: Restart Language Server`） |
| 改了代码但现象不变 | 函数未被调用时被 `--gc-sections` 丢弃。用 `arm-none-eabi-nm <elf> \| grep <符号>` 与 FLASH 占用是否变化来判断 |
| `LEDR_ON();` 报语法错误 | 野火风格 BSP 的 `LEDR_ON` 是**无参宏**，应写 `LEDR_ON;` 不带括号 |
| 链接报 `unrecognized option '--gC-sections'` | 从网页粘贴 CMake 代码时字符被替换，检查是否写成 `--gc-sections`、`-lm`、`${CMAKE_...}` |
| `objcopy: 'xxx.elf': No such file` | 改过 CubeMX 工程名，但 `run.sh` / `flash` 里的 elf 名未同步 |

## 已知限制

- **Drivers 已裁剪**：为控制仓库体积，`.gitignore` 排除了 `CMSIS/DSP`（179 MB）、`CMSIS/NN`、`RTOS`、`RTOS2`、`Device/ST/STM32H7xx/Source`，以及除 H743 外的 21 个型号头文件。克隆后可直接编译；若需切换到其它 H7 型号或使用 DSP 库，请从 CubeMX 固件包补回对应文件。
- **烧录会擦除 Flash 尾部**：OpenOCD 按 bank 边界对齐擦除，日志中的 `Adding extra erase range` 属正常行为。若在 Flash 尾部存放参数，需要自行调整链接脚本或烧录范围。
- **工程名与仓库名不一致**：CubeMX 工程名仍是 `Cmake_2_Projects_Template`，产物为 `build/Cmake_2_Projects_Template.elf`。改名需同步 `run.sh`、`flash`、`flash-reset`、`launch.json` 四处。
- **未验证项**：读保护（RDP）、双 Bank 选项字节、低功耗模式下的调试连接，均未做实测。

## 许可证

本项目自研部分（`Core/` 中的用户代码、`User/`、CMake 配置、脚本与文档）采用 **MIT**，见 [LICENSE](LICENSE)。

`Drivers/`、`startup_stm32h743xx.s` 等第三方组件遵循各自许可证，MIT 不适用于这些目录。详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
