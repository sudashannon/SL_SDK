-- define toolchain
toolchain("armgcc")
    set_kind("standalone")
    set_sdkdir("C:\\UserProfession\\GNU Arm Embedded Toolchain\\10 2021.10")
toolchain_end()

target("stm32h7_demo")
    add_rules("mode.debug", "mode.release")
    set_toolchains("armgcc")
    set_kind("binary")
    -- set warning all as error
    set_warnings("all", "error")
    -- set language: gnu99, gnu++11
    set_languages("gnu99", "gnu++11")
    add_files("../rte_adapter_metal.c")
    add_files("./cubemx_driver/*.c")
    add_files("../../rte/src/data_structure/*.c",
                "../../rte/src/middleware/*.c",
                "../../rte/src/sugar/*.c",
                "../../hal/src/hal.c",
                "../../hal/src/hal_gpio_stm32h7.c",
                "../../hal/src/hal_com_stm32h7.c",
                "../../rte/utils/segger_hardfault_handle/SEGGER_HardFaultHandler.c",
                "../../rte/utils/segger_hardfault_handle/HardFaultHandler.S")
    if is_arch("armv7-m") then
        add_files("../../rte/src/sugar/arch/arm_v7m/*.c")
        add_files("../../rte/src/sugar/arch/arm_v7m/*.S")
    end
    add_files("../../sis/cmsis/device/stm32h7/src/gcc/startup_stm32h750xx.s",
                "../../sis/cmsis/device/stm32h7/src/system_stm32h7xx.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_dma.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_dma_ex.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_rcc.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_rcc_ex.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_pwr.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_pwr_ex.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_gpio.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_mdma.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_qspi.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_uart.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_uart_ex.c",
                "../../sis/cmsis/drivers/stm32h7/src/stm32h7xx_hal_cortex.c")
    add_includedirs("./user_config",
                    "./cubemx_driver",
                    "../../sis/cmsis/core/inc",
                    "../../sis/cmsis/device/stm32h7/inc",
                    "../../sis/cmsis/drivers/stm32h7/inc",
                    "../../rte/inc",
                    "../../hal/inc")
    add_cxflags("-DSTM32H750xx", "-DUSE_HAL_DRIVER",
                "-mcpu=cortex-m7", "-mthumb", "-mfpu=fpv5-d16",
                "-O0",
                "-Wall", "-fdata-sections", "-ffunction-sections")
    add_asflags("-D__NEWLIB__",
                "-mcpu=cortex-m7", "-mthumb", "-mfpu=fpv5-d16",
                "-O0",
                "-Wall", "-fdata-sections", "-ffunction-sections")
    -- set ld configurations
    add_ldflags("-mcpu=cortex-m7", "-mthumb", "-mfpu=fpv5-d16",
                "-Tdefault.ld",
                "-specs=nano.specs",
                "-specs=nosys.specs",
                "-Wl,--cref",
                "-Wl,--gc-sections",
                "-Wl,-Map=build/stm32h7_demo.map",
                {force = true})
if is_arch("armv7-m") then
    after_build(
        function(objdump)
            os.exec("arm-none-eabi-objcopy.exe -O binary .\\build\\cross\\armv7-m\\debug\\stm32h7_demo .\\stm32h7_demo.bin")
        end
    )
end
target_end()