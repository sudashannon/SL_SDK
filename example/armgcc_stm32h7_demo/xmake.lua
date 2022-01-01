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
    add_files("main.c")
    add_files("../../sis/cmsis/device/stm32h7/src/gcc/startup_stm32h750xx.s",
                "../../sis/cmsis/device/stm32h7/src/system_stm32h7xx.c")
    add_includedirs("../../sis/cmsis/core/inc",
                    "../../sis/cmsis/device/stm32h7/inc")
    add_cxflags("-DSTM32H743xx")
    -- set ld configurations
    add_ldflags("-Tdefault.ld",
                "-specs=nano.specs",
                "-nostartfiles",
                "-Wl,--gc-sections",
                "-Wl,-Map=build/stm32h7_demo.map",
                {force = true})
    -- output architecture configurations
    add_cxflags("-mcpu=cortex-m7", "-mthumb", "-mfpu=fpv5-d16")
    -- general define
    add_cxflags("-fdata-sections", "-ffunction-sections", "-Warray-bounds")

    after_build(
        function(objdump)
            os.exec("arm-none-eabi-objcopy.exe -O binary .\\build\\cross\\none\\debug\\stm32h7_demo .\\stm32h7_demo.bin")
        end
    )
target_end()