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

## 内置 BSP

`User/` 下包含一组可直接使用的基础模块，接口沿用野火教程风格（无参宏、`xxx_Config()`），实现为独立编写：

| 模块 | 文件 | 资源占用 | 说明 |
|---|---|---|---|
| RGB LED | `bsp_led.h/.c` | PB0(红) / PB1(绿) / PA3(蓝) | 共阳接法，低电平点亮；提供 `LED_RED`、`LED_GREEN`、`LED_BLUE`、`LED_YELLOW` 等混色宏 |
| 串口 | `bsp_usart.h/.c` | USART1，PA9(TX) / PA10(RX) | 115200-8-N-1，已把 `printf` / `scanf` 重定向到串口 |
| 按键 | `bsp_key.h/.c` | KEY1=PA0，KEY2=PC13 | 10 ms 软件消抖，`Key_Scan()` 内部等待按键释放 |
| 延时 | `core_delay.h/.c` | DWT CYCCNT | 参考野火实现：重定义 `HAL_InitTick` / `HAL_GetTick` 到 DWT，系统时基不再依赖 SysTick |
| ADC | `bsp_adc.h/.c` | PC3 → ADC3_INP1（双焊盘引脚，需开 SYSCFG 模拟开关） | DMA1_Stream1 循环搬运，缓冲区放在 **D2 SRAM**（`.dma_buffer` 段），main 中已启用并打印电压 |
| MPU 配置 | `bsp_mpu.h/.c` | — | 来自野火 **H745 双核**例程，H743 单核下与 CubeMX 生成的 `MPU_Config()` 功能重复，可删除 |

`main()` 当前行为：

1. 依次调用 `LED_GPIO_Config()`、`UARTx_Config()`、`Key_GPIO_Config()`
2. 串口输出 `STM32H743 CMake template started.`
3. 循环显示 红 → 绿 → 蓝 → 黄（各 500 ms），同时扫描 KEY1，按下时打印 `KEY1 pressed`

> 时基说明：`core_delay.c` 覆盖了 HAL 的两个弱函数，`HAL_Init()` 时即完成 DWT 初始化，所以 `HAL_Delay()` 仍然可用，无需额外初始化。
>
> `bsp_mpu.c` 未在 `main()` 中调用，其符号被 `--gc-sections` 丢弃，不占用 Flash。
>
> **ADC 的 DMA 缓冲区必须放在 DMA 可访问的 SRAM**。本工程链接脚本把 `.bss` 放在 `DTCMRAM`（0x20000000），而 DMA1/DMA2 无法访问 DTCM/CCM，若把缓冲区当普通全局变量定义，DMA 写入无效、CPU 读回恒为 0。因此 `STM32H743XX_FLASH.ld` 新增了 `.dma_buffer` 段并映射到 `RAM_D2`（0x30000000），`ADC_ConvertedValue` 通过 `__attribute__((section(".dma_buffer")))` 放入该段，可用 `arm-none-eabi-nm` 验证地址确为 `0x30000000`。

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
| 代码全屏红波浪线 | clangd 未找到 `compile_commands.json`、缺 `--query-driver`、或索引缓存过期（`Ctrl+Shift+P → clangd: Restart Language Server`） |
| 改了代码但现象不变 | 函数未被调用时被 `--gc-sections` 丢弃。用 `arm-none-eabi-nm <elf> \| grep <符号>` 与 FLASH 占用是否变化来判断 |
| `printf` 有调用但串口无输出 | GCC/newlib 下 `printf` 走 `_write` 而非 `fputc`。需实现 `_write` 覆盖 `syscalls.c` 的弱符号，`nm` 查应为 `T _write` |
| 外部改动的文件在编辑器里看不到 | 编辑器有未保存修改时不会自动重载磁盘内容，详见 [编辑器与磁盘文件同步](#编辑器与磁盘文件同步) |
| 链接报 `unrecognized option '--gC-sections'` | 从网页粘贴 CMake 代码时字符被替换，检查是否写成 `--gc-sections`、`-lm`、`${CMAKE_...}` |
| `objcopy: 'xxx.elf': No such file` | 改过 CubeMX 工程名，但 `run.sh` / `flash` 里的 elf 名未同步 |

## 编辑器与磁盘文件同步

`.vscode/settings.json` 里开启了自动保存：

```jsonc
"files.autoSave": "afterDelay",
"files.autoSaveDelay": 1000
```

这不是随手加的配置，而是为了解决一类高频问题：**外部程序（脚本、AI 助手、git checkout）修改了磁盘文件，但编辑器里看到的还是旧内容，一保存就把外部改动整段覆盖回去。**

### 触发机制

VS Code 对磁盘变化的处理规则：

| 编辑器状态 | 磁盘文件被外部修改时 |
|---|---|
| 无未保存修改（干净） | 自动重载，立即看到新内容 |
| **有未保存修改（dirty）** | **保留编辑器内容**，仅在标签页标一个圆点，绝不覆盖用户输入 |

第二条是保护机制，代价是：只要缓冲区处于 dirty 状态，外部改动就永远显示不出来；此时按 `Ctrl+S`，会把外部改动整段冲掉。双方来回覆盖，表现为「明明改了，报错却还是旧的」。

### 典型症状

- `grep` 或命令行读到的内容，与编辑器里显示的不一致
- 编译报错指向某一行，但那行在编辑器里看起来已经改好了
- 编辑器显示的行号与磁盘真实行号相差若干行

### 处理方式

1. **日常**：依赖上面的 `autoSave`，缓冲区不会长期 dirty（该设置需重载窗口后生效）
2. **手动**：`Ctrl+Shift+P` → `File: Revert File`，丢弃本地未保存内容，加载磁盘版本
3. **确认**：`git status` 查看是否有意料之外的修改；提交前务必执行一次

## 已知限制

- **UART 是手动接入的**：CubeMX 里没有配置 USART，因此本模板手动做了三件事——在 `Core/Inc/stm32h7xx_hal_conf.h` 取消注释 `HAL_UART_MODULE_ENABLED`、在根 `CMakeLists.txt` 追加 `stm32h7xx_hal_uart.c`、在 `User/` 下提供 `bsp_usart`。若之后在 CubeMX 中重新生成代码，`stm32h7xx_hal_conf.h` 会被覆盖导致 UART 重新失效，建议直接在 CubeMX 里启用 USART1 再生成，之后删除根 `CMakeLists.txt` 里那行手动补充的 `hal_uart.c`。
- **Drivers 已裁剪**：为控制仓库体积，`.gitignore` 排除了 `CMSIS/DSP`（179 MB）、`CMSIS/NN`、`RTOS`、`RTOS2`、`Device/ST/STM32H7xx/Source`，以及除 H743 外的 21 个型号头文件。克隆后可直接编译；若需切换到其它 H7 型号或使用 DSP 库，请从 CubeMX 固件包补回对应文件。
- **烧录会擦除 Flash 尾部**：OpenOCD 按 bank 边界对齐擦除，日志中的 `Adding extra erase range` 属正常行为。若在 Flash 尾部存放参数，需要自行调整链接脚本或烧录范围。
- **工程名与仓库名不一致**：CubeMX 工程名仍是 `Cmake_2_Projects_Template`，产物为 `build/Cmake_2_Projects_Template.elf`。改名需同步 `run.sh`、`flash`、`flash-reset`、`launch.json` 四处。
- **未验证项**：读保护（RDP）、双 Bank 选项字节、低功耗模式下的调试连接，均未做实测。

## 许可证

本项目自研部分（`Core/` 中的用户代码、`User/`、CMake 配置、脚本与文档）采用 **MIT**，见 [LICENSE](LICENSE)。

`Drivers/`、`startup_stm32h743xx.s` 等第三方组件遵循各自许可证，MIT 不适用于这些目录。详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
