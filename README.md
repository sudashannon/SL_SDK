# SL_SDK

## Introduction

SL_SDK is the software development kit for embedded devices, such as Cortex-M based MCU or RISC-V MCU. It provides varies of platform-related and cross-platform code to speed-up the development.

Till now, the SL_SDK is consist of following modules:

* RTE: Provides dynamic memory pool, concurrent data structure, log system and shell system.
* HAL: Provides united hardware devices instantiate and access api.
* OSAL: Provides united OS api to make cross-platform easier.
* SIS: Like CMSIS, try to provide united API to adapt different core like Cortex-M or RISC-V.
* bootloader: Provide an realization which is adapt for rtt's OTA

SL_SDK是一个用于嵌入式设备开发的软件框架，适配了包括Cortex-M、RISC-V等体系架构在内的MCU。SL_SDK提供了一系列体系架构适配和跨平台的代码以加速应用开发工作。

截至目前，SL_SDK包括以下模块：

* 运行时环境：提供了包括动态内存管理、并发数据结构（链表、哈希表、数组、环形缓冲区等）、日志系统和shell系统在内的组件
* 硬件抽象层：提供了一套统一的硬件设备实例化和访问接口
* 操作系统抽象层：提供了一套统一的OS接口以利于跨平台开发
* SOC独立软件包：给基于不同的体系架构而设计的SOC提供统一、完善的SOC级驱动和板级支持包
* bootloader：提供了兼容rtt OTA固件格式的bootloader实现

## 构建系统

SL_SDK使用xmake作为整个工程的构建系统，针对不同的体系架构，用户只需要指定自己的工具链即可。

## Runtime Environment

## Hardware Abstract Layer

## Operating System Abstract Layer

## Software Interface Standard
