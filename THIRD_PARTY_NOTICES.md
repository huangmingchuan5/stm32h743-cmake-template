# 第三方组件声明 / Third-Party Notices

## 许可证适用范围

根目录 `LICENSE`（MIT）**仅覆盖本仓库的原创部分**：`Core/` 中 USER CODE 区内的用户代码、`User/` 下的 BSP、CMake 配置、shell 脚本、OpenOCD 配置以及文档。

它**不覆盖**下列第三方组件，这些组件遵循各自的许可证。

本仓库包含 STMicroelectronics 与 Arm 提供的嵌入式软件组件。它们**不受**本仓库根目录 `LICENSE`（MIT）约束，而是遵循各自的许可证。下表列出每个组件的位置、版权方、许可证类型，以及仓库内可查证的依据文件。

| 组件 | 仓库内位置 | 版权方 | 许可证 | 依据文件 |
|---|---|---|---|---|
| STM32H7 HAL 驱动 | `Drivers/STM32H7xx_HAL_Driver/` | STMicroelectronics | **BSD-3-Clause** | `Drivers/STM32H7xx_HAL_Driver/LICENSE.txt` |
| CMSIS Core / Device | `Drivers/CMSIS/` | Arm Limited | **Apache-2.0** | `Drivers/CMSIS/LICENSE.txt` |
| 启动文件 | `startup_stm32h743xx.s` | STMicroelectronics | **BSD-3-Clause** | 文件头版权声明；无独立 LICENSE 文件时按 ST 的 BSD-3-Clause 条款 |
| 链接脚本 | `STM32H743XX_FLASH.ld` | STMicroelectronics | **BSD-3-Clause** | 同上，随 CubeMX 生成 |
| 延时实现 | `User/Src/core_delay.c`、`User/Inc/core_delay.h` | 野火（fire） | 教学示例代码，版权归原作者 | 文件头 `@author fire` 及论坛/淘宝链接 |
| MPU 配置 | `User/Src/bsp_mpu.c`、`User/Inc/bsp_mpu.h` | 野火（fire） | 教学示例代码，版权归原作者 | 文件头注明实验平台为野火 H745 开发板 |

> `User/` 下的 `core_delay` 与 `bsp_mpu` 参考野火教程例程整理而来，版权归野火所有，仅作学习参考。若需商用或再分发，请自行替换为独立实现。`bsp_led`、`bsp_usart`、`bsp_key`、`bsp_adc` 的接口沿用了野火教程风格，但实现为独立编写。

## 许可证全文

- BSD-3-Clause: <https://opensource.org/licenses/BSD-3-Clause>
- Apache-2.0: <https://www.apache.org/licenses/LICENSE-2.0>

`Drivers/STM32H7xx_HAL_Driver/LICENSE.txt` 的原文明确写道：

> If you received this software component outside of a package or without
> applicable license terms, the terms of the BSD-3-Clause license shall apply.

因此在本仓库（独立分发、非完整 STM32Cube 包）的场景下，HAL 驱动按 BSD-3-Clause 处理。

## 关于裁剪

为控制仓库体积，`.gitignore` 排除了部分未被本工程引用的 CMSIS 组件（DSP、NN、RTOS、RTOS2、Core、Core_A）以及除 STM32H743 之外的型号头文件。被排除的文件仍属上述第三方组件，同样遵循各自的许可证，只是不在本仓库内分发。

## 合规提示

再分发本仓库或其中包含的第三方组件时：

1. 保留各目录下的 `LICENSE.txt` 与源文件中的版权声明，不要删除或修改
2. 在衍生产品的文档中保留本声明
3. Apache-2.0 组件（CMSIS）如被修改，需在文件中标注修改说明
4. 不得暗示 STMicroelectronics 或 Arm 对本项目提供背书

---

# Third-Party Notices (English)

This repository bundles embedded software components from STMicroelectronics and Arm. They are **not** covered by the MIT license at the repository root and remain under their own licenses:

- `Drivers/STM32H7xx_HAL_Driver/` — BSD-3-Clause, © STMicroelectronics
- `Drivers/CMSIS/` — Apache-2.0, © Arm Limited
- `startup_stm32h743xx.s`, `STM32H743XX_FLASH.ld` — BSD-3-Clause, © STMicroelectronics

License texts are available in the respective `LICENSE.txt` files and at the URLs listed above. When redistributing, keep all copyright notices and license files intact.
