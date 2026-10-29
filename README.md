# STM32H743 CMake Template

面向 **STM32H743XIHx** 的 CMake 工程模板：用 `arm-none-eabi-gcc + Ninja + OpenOCD` 替代 CubeMX 默认的 IDE 工程，开箱即可编译、烧录，并配好 VS Code 的代码提示（clangd）与断点调试（Cortex-Debug）。

工程由 STM32CubeMX 生成骨架，业务代码写在 `Core/` 的 `USER CODE` 区内，BSP 放在独立的 `User/` 目录，重新生成代码都不会丢失。

**复制到新目录即可当新工程用**——工程名、产物名、烧录路径全部自动跟随目录名，不需要改任何配置。

---

## 这个模板解决什么问题

| 常见痛点 | 本模板的做法 |
|---|---|
| CubeMX 只生成 IDE 工程，命令行编译要自己搭 | 提供完整的 CMake + Ninja 构建，`./run.sh` 一条命令出 `elf/bin/hex` |
| 交叉编译环境下 IDE 补全失效、满屏红波浪线 | 用 clangd + `compile_commands.json` + `--query-driver`，索引与编译完全一致 |
| 每做一个实验就复制一份工程，改名字改到漏 | 工程名自动取目录名，复制即用 |
| 烧录命令记不住、路径写死 | OpenOCD 配置文件自动匹配产物名 |
| 换芯片/加外设后各种链接错误 | 文档里集中列出 H7 特有的坑与判断方法 |

## 特性

- **CMake + Ninja** 构建，增量编译，不依赖任何 IDE
- **OpenOCD** 烧录，DAPLink 与 ST-Link 两套配置随时切换
- **clangd** 代码补全与跳转，不依赖微软 C/C++ 插件的交叉编译配置
- **Cortex-Debug** 断点调试，支持变量、寄存器、外设视图
- **`User/` 目录** 独立存放 BSP，新增 `.c` 自动纳入构建
- **自动命名**：工程名 / 产物名 / 烧录路径全部跟随目录名
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

安装完成后 armup 会把工具目录加入用户 `PATH`。`run.sh` 里也写了 PATH 回退逻辑，找不到工具时自动去 `D:/Embedded_Toolchain` 下找。

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
├── Drivers/                     # HAL 与 CMSIS（完整包含，含 DSP 与全部 H7 型号头文件）
├── cmake/
│   ├── gcc-arm-none-eabi.cmake  # 工具链文件：编译/链接参数集中在此
│   └── stm32cubemx/             # CubeMX 自动生成，切勿手改
├── tools/
│   └── new_project.sh           # 复制模板后的一键初始化脚本
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
./run.sh          # 编译，产出 build/<目录名>.elf / .bin / .hex
./run.sh flash    # 编译并烧录
```

手动方式（等价）：

```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug ..
ninja -j 8
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

## 从模板创建新工程

### 自动命名机制

不需要修改任何文件，工程名取自当前目录名：

| 位置 | 取值方式 |
|---|---|
| `CMakeLists.txt` | `get_filename_component(... NAME)` |
| `run.sh` | `ELF_NAME="$(basename "$(pwd)")"` |
| `flash` / `flash-reset` | Tcl `[file tail [pwd]]` |
| `.vscode/launch.json` | `${workspaceFolderBasename}` |
| 产物 | `build/<目录名>.elf` / `.bin` / `.hex` |

如需固定工程名，把 `CMakeLists.txt` 里那两行换成 `set(CMAKE_PROJECT_NAME 你的名字)`，并同步改 `run.sh` 与 `flash`。

### 初始化脚本

```bash
cp -r Cmake_2_Projects_Template Cmake_4_MY_PROJECT
cd Cmake_4_MY_PROJECT
./tools/new_project.sh
```

脚本会做三件事：

1. 删除 `build/`（其中的 `CMakeCache.txt` 记录着上一个工程的绝对路径）

它拒绝在模板目录本身执行，避免误删。

### 复制后仍需人工处理

- **`.ioc` 工程名**：在 CubeMX 里改 Project Name 并另存为 `<新名字>.ioc`。多个工程共用一个 ioc 会互相覆盖。
- **手动补的 HAL 源文件**：不同工程的外设配置不同，见下方「CubeMX 未启用的外设」，不能照抄。

## VS Code 配置

参考 [armup 的说明](https://github.com/ref42/armup)，核心只有三步：

1. **安装 `clangd` 扩展**（只装 clangd 就够）——它读取 CMake 导出的 `compile_commands.json`，无需手工配置 include 路径
2. **先跑一次 Debug 或 Release 构建**，生成 `elf` 与 `compile_commands.json`
3. **安装 `Cortex-Debug` 扩展**，`launch.json` 里已指向产物，直接按 F5 调试

本仓库已预置：

| 文件 | 作用 |
|---|---|
| `.vscode/settings.json` | clangd 参数、CMake Tools、自动保存 |
| `.vscode/tasks.json` | `Ctrl+Shift+B` 编译、flash 任务 |
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

验证 clangd 配置是否正确（不用打开 VS Code）：

```bash
clangd --check=Core/Src/main.c \
       --compile-commands-dir=build \
       --query-driver=<你的 gcc 绝对路径>
# 输出 All checks completed, 0 errors 即正常
```

### 把默认终端设为 Git Bash

本模板的脚本（`run.sh`、`tools/new_project.sh`）都是 Bash 脚本。Windows 下 VS Code 的集成终端默认是 **PowerShell**，直接执行会报错：

```
PS C:\...\stm32h743-cmake-template> ./run.sh
./run.sh : 无法将“./run.sh”项识别为 cmdlet、函数、脚本文件或可运行程序的名称。
```

需要把默认终端配置文件改成 **Git Bash**：

| 设置项 | 值 |
|---|---|
| `Terminal › Integrated › Default Profile: Windows` | `Git Bash` |

两种设置方式：

- **图形界面**：`Ctrl+,` 打开设置，搜索 `terminal.integrated.defaultProfile.windows`，下拉选择 `Git Bash`
- **JSON**：本仓库的 `.vscode/settings.json` 已预置该项，clone 后重开终端即生效

设置生效后，终端提示符会从 `PS>` 变成 `$`，此时 `./run.sh` 可以正常运行。

若 VS Code 提示该项只能在用户设置中修改（部分版本对此项有作用域限制），在用户设置里改一次即可，不影响仓库内的其它配置。

## 编写代码

### 业务代码放哪里

| 位置 | 用途 | 是否被 CubeMX 覆盖 |
|---|---|---|
| `Core/Src/main.c` 的 `USER CODE` 区 | 初始化调用、主循环 | 否，标记区内的内容保留 |
| `User/Src/`、`User/Inc/` | 自己的 BSP / 驱动 | 否，CubeMX 不知道这个目录 |
| 根 `CMakeLists.txt` | 构建配置、手动补的 HAL 源文件 | 否，只生成一次 |
| `cmake/stm32cubemx/` | CubeMX 的构建描述 | **是，每次 GENERATE CODE 整体重写** |

### 新增 BSP 文件

`User/Src/*.c` 由 `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` 自动收集，**新增文件不需要改 CMakeLists**。头文件放 `User/Inc/`，已在 `target_include_directories` 中。

首次新增文件后建议跑一次 `./run.sh`，让 `compile_commands.json` 更新，否则 clangd 看不到新文件。

若想改成显式列出（CMake 官方推荐，避免 GLOB 的隐式依赖），把根 `CMakeLists.txt` 里：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${USER_SOURCES})
```

换成逐个列出：

```cmake
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Src/bsp_led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Src/bsp_usart.c
)
```

## CubeMX 未启用的外设

如果 `User/` 下的 BSP 用到 CubeMX 里没配置的外设（例如串口、ADC），需要手动补两处：

1. `Core/Inc/stm32h7xx_hal_conf.h`：取消对应 `HAL_xxx_MODULE_ENABLED` 的注释
2. 根 `CMakeLists.txt` 的 `target_sources`：追加对应 `stm32h7xx_hal_xxx.c`

**添加源文件前必须先检查是否已存在**：

```bash
grep -oE "stm32h7xx_hal_[a-z_]+\.c" cmake/stm32cubemx/CMakeLists.txt
```

若 CubeMX 已经启用了该外设，这个列表里就会有对应文件；再手动加一次会产生 `multiple definition` 链接错误。

> 注意：`stm32h7xx_hal_conf.h` 也是 CubeMX 生成的文件，重新 GENERATE CODE 会被覆盖。更稳妥的做法是直接在 CubeMX 里启用该外设，让它自动生成配置。

## 关键设计与避坑

这一节记录的是几个「不看就会踩、看了解释就懂」的点。

### H7 的 DMA 不能访问 DTCM

CubeMX 生成的 H7 链接脚本默认把 `.bss` 放在 DTCM：

```ld
.bss (NOLOAD) : { ... } >DTCMRAM    /* 0x20000000 */
```

DTCM 是 CPU 私有 TCM，**DMA1/DMA2 挂在 D2 域、走 AHB/AXI 总线，物理上访问不到**。所以任何 DMA 缓冲区如果写成普通全局变量，会出现「外设正常转换 / 收发，DMA 也启动了，但 CPU 读回来永远是 0」。

判断方法：

```bash
arm-none-eabi-nm build/*.elf | grep <缓冲区名>
# 0x2000xxxx → 在 DTCM，DMA 到不了
```

解决：在链接脚本里新增一段映射到 D2 SRAM，变量指定段名：

```ld
.dma_buffer (NOLOAD) :
{
  . = ALIGN(4);
  *(.dma_buffer)
  *(.dma_buffer*)
  . = ALIGN(4);
} >RAM_D2
```

```c
__IO uint16_t adc_buf __attribute__((section(".dma_buffer")));   /* 地址应为 0x30000000 */
```

DMA 可访问的区域：D1 SRAM（0x24000000）、D2 SRAM（0x30000000）、D3 SRAM（0x38000000）。F4 系列同理，CCM（0x10000000）也不能被 DMA 访问。

> Keil 的 `__attribute__((section(".bss.ARM.__at_0x30000000")))` 是 ARMCC 专有语法，GCC 只当普通段名、不解析地址，**不要直接照抄 Keil 例程**。

### GCC 下重写 `fputc` 不能重定向 `printf`

| 工具链 | printf 调用链 |
|---|---|
| Keil / ARMCC | `printf → fputc()` |
| **GCC / newlib** | `printf → _write() → __io_putchar()` |

CubeMX 生成的 `syscalls.c` 里 `__io_putchar` 只有一个 weak 声明、从未定义，所以从 Keil 例程抄来的 `fputc` 在 GCC 下毫无作用，串口一个字都不会输出。

正确做法是在 BSP 里实现 `_write`（整块发送，效率更高），并返回原始长度：

```c
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&huart, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}
```

验证：`arm-none-eabi-nm` 查 `_write` 应为 `T`（强符号），若为 `W` 说明仍是 `syscalls.c` 的弱版本。

### 时基被覆盖时，`HAL_Init()` 不能省

如果工程用 DWT 替换 SysTick 做时基（如 `core_delay.c` 重定义 `HAL_InitTick` / `HAL_GetTick` 两个弱函数），那么 **`HAL_Init()` 是唯一触发 DWT 初始化的地方**。删掉它，`HAL_GetTick()` 恒返回 0，所有 `HAL_Delay()` 和 HAL 内部超时判断都会永久卡死。

判断时基是否被覆盖：

```bash
arm-none-eabi-nm build/*.elf | grep HAL_InitTick    # T 表示被工程覆盖
```

标准初始化顺序：

```c
HAL_Init();            /* 必须第一 */
SystemClock_Config();  /* 时钟树 */
MX_GPIO_Init();        /* 外设 */
```

### 无参宏与函数式宏的调用形式不同

```c
#define LEDR_ON    digitalLo(...)          /* 无参宏：调用写 LEDR_ON;    */
#define LEDR_ON()  HAL_GPIO_WritePin(...)  /* 函数式宏：调用写 LEDR_ON(); */
```

野火 BSP 大量使用无参宏（展开成 `{GPIOB->BSRR=...;}` 这样的语句块），写成 `LEDR_ON();` 会展开为 `{...}();` 而语法错误。判断方法：看头文件里宏名后面有没有 `()`。

### 工程名自动取目录名的原因

模板最常见的使用方式是「复制一份做新实验」。如果工程名硬编码在四个地方（`CMakeLists.txt`、`run.sh`、`flash`、`launch.json`），漏改一处就会出现 `objcopy: 'xxx.elf': No such file` 或烧录时找不到文件。改为跟随目录名后，复制即用。

## 常见问题

| 现象 | 原因与处理 |
|---|---|
| `Error: Can't find flash` | 不在工程根目录执行。`openocd -f flash` 只查当前目录，先 `cd` 回根目录 |
| `Error connecting DP: cannot read IDR` | 优先查目标板是否上电，其次 GND 共地、SWDIO/SWCLK 是否接反。100 kHz 低速仍失败即为物理层问题 |
| 代码全屏红波浪线 | clangd 未找到 `compile_commands.json`、缺 `--query-driver`、或索引缓存过期（`Ctrl+Shift+P → clangd: Restart Language Server`） |
| 改了代码但现象不变 | 函数未被调用时被 `--gc-sections` 丢弃。用 `arm-none-eabi-nm <elf> \| grep <符号>` 与 FLASH 占用是否变化来判断 |
| 新增 `.c` 没被编译 | GLOB 需要重新配置才收录。删 `build/` 重跑，或 `cmake ..` 强制重新配置 |
| `printf` 有调用但串口无输出 | GCC/newlib 走 `_write` 而非 `fputc`，见「关键设计与避坑」 |
| `multiple definition of 'HAL_xxx'` | 同一 HAL 源文件被 `cmake/stm32cubemx/CMakeLists.txt` 和根 `CMakeLists.txt` 各加了一次 |
| `objcopy: 'xxx.elf': No such file` | 工程名与产物名不一致。本模板已自动命名，若手动改过请检查四处 |
| 链接报 `unrecognized option '--gC-sections'` | 从网页粘贴 CMake 代码时字符被替换，检查是否写成 `--gc-sections`、`-lm`、`${CMAKE_...}` |

## 编辑器与磁盘文件同步

`.vscode/settings.json` 里开启了自动保存：

```jsonc
"files.autoSave": "afterDelay",
"files.autoSaveDelay": 1000
```

这不是随手加的配置，而是为了解决一类高频问题：**外部程序（脚本、AI 助手、`git checkout`）修改了磁盘文件，但编辑器里看到的还是旧内容，一保存就把外部改动整段覆盖回去。**

### 触发机制

| 编辑器状态 | 磁盘文件被外部修改时 |
|---|---|
| 无未保存修改（干净） | 自动重载，立即看到新内容 |
| **有未保存修改（dirty）** | **保留编辑器内容**，仅在标签页标一个圆点，绝不覆盖用户输入 |

第二条是保护机制，代价是：只要缓冲区处于 dirty 状态，外部改动就永远显示不出来；此时按 `Ctrl+S` 会把外部改动整段冲掉，表现为「明明改了，报错却还是旧的」。

### 典型症状

- `grep` 或命令行读到的内容，与编辑器里显示的不一致
- 编译报错指向某一行，但那行在编辑器里看起来已经改好了
- 编辑器显示的行号与磁盘真实行号相差若干行

### 处理方式

1. **日常**：依赖上面的 `autoSave`，缓冲区不会长期 dirty（该设置需重载窗口后生效）
2. **手动**：`Ctrl+Shift+P` → `File: Revert File`，丢弃本地未保存内容，加载磁盘版本
3. **确认**：`git status` 查看是否有意料之外的修改；提交前务必执行一次

## 示例代码

`User/` 下附带一组可运行的 BSP，用于演示 `User/` 目录的接入方式。**实际项目可以整体删除，不影响构建**。

| 模块 | 文件 | 资源占用 | 说明 |
|---|---|---|---|
| RGB LED | `bsp_led` | PB0 / PB1 / PA3 | 共阳接法，提供 `LED_RED` 等混色宏 |
| 串口 | `bsp_usart` | USART1，PA9 / PA10 | 115200-8-N-1，已重定向 `printf` |
| 按键 | `bsp_key` | PA0 / PC13 | 10 ms 消抖 |
| 延时 | `core_delay` | DWT CYCCNT | 重定义 `HAL_InitTick` / `HAL_GetTick` 到 DWT |
| ADC | `bsp_adc` | PC3 → ADC3_INP1 | DMA 循环采样，缓冲区在 D2 SRAM |
| MPU 配置 | `bsp_mpu` | — | 来自 H745 双核例程，H743 单核下冗余，可删 |

`Core/Src/main.c` 演示了调用方式（初始化各模块、串口打印、主循环闪灯 + 扫键）。写自己的项目时直接替换该文件即可。

## 已知限制

- **模板按 `STM32H743XIHx` 配置**。换芯片需同步修改：`.ioc`、`startup_stm32h7xx.s`、`STM32H7xx_FLASH.ld`、`cmake/stm32cubemx/CMakeLists.txt` 里的 `STM32H743xx` 宏，以及 `CMakeLists.txt` 的 `-DSTM32H743xx`
- **烧录会擦除 Flash 尾部**：OpenOCD 按 bank 边界对齐擦除，日志中的 `Adding extra erase range` 属正常行为
- **`.ioc` 与工程名不强制关联**：本仓库的 ioc 仍是 CubeMX 的原始文件名，复制模板后请自行在 CubeMX 中重命名
- **未验证项**：读保护（RDP）、双 Bank 选项字节、低功耗模式下的调试连接

## 许可证

本项目自研部分（`Core/` 中的用户代码、`User/`、CMake 配置、脚本与文档）采用 **MIT**，见 [LICENSE](LICENSE)。

`Drivers/`、`startup_stm32h743xx.s` 等第三方组件遵循各自许可证，MIT 不适用于这些目录。详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
