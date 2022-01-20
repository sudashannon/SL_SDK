-- define toolchain
toolchain("riscvgcc")
    set_kind("standalone")
    set_sdkdir("C:\\UserProfession\\riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-w64-mingw32")
toolchain_end()

toolchain("riscvgcc_ubuntu")
    set_kind("standalone")
    set_sdkdir("~/.local/bin/riscv_64bit_toolchain")
toolchain_end()

add_rules("mode.debug", "mode.release")
-- Set target
target("xdemo_riscv64")
if is_host("windows") then
    set_toolchains("riscvgcc")
else
    set_toolchains("riscvgcc_ubuntu")
end
    set_kind("binary")
    add_files("../../sis/rsis/core/src/*.S")
    add_files("../../sis/rsis/core/src/*.c")
    add_files("../../sis/rsis/device/k210/src/*.c")
    add_files("../../sis/rsis/drivers/k210/src/*.c")
    -- User files
    add_files("*.c")
    add_files("lcd/*.c")
    -- Include path
    add_includedirs(".")
    add_includedirs("lcd")
    add_includedirs("../../rte/inc");
    add_includedirs("../../sis/rsis/core/inc");
    add_includedirs("../../sis/rsis/device/k210/inc");
    add_includedirs("../../sis/rsis/drivers/k210/inc");

    set_warnings("all", "error")
    add_defines("CONFIG_LOG_LEVEL=LOG_VERBOSE",
                "CONFIG_LOG_ENABLE",
                "CONFIG_LOG_COLORS",
                "LOG_KERNEL",
                "DEBUG_ON_QEMU=0",
                "RTE_USE_SUGAR_KERNEL=0")
    add_cxflags("-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany",
                "-ffunction-sections", "-fdata-sections" ,"-fno-common",
                "-fno-common", "-ffunction-sections", "-fdata-sections",
                "-fstrict-volatile-bitfields", "-fno-zero-initialized-in-bss",
                "-ffast-math", "-fno-math-errno", "-fsingle-precision-constant",
                "-Os", "-Wno-pointer-to-int-cast", "-Wno-strict-aliasing")
    add_asflags("-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany",
                "-ffunction-sections", "-fdata-sections" ,"-fno-common")
    add_ldflags("-Tkendryte.ld",
                "-Wl,-Map,$(buildir)/r64_memory.map",
                "-Wl,--gc-sections",
                "-Wl,--check-sections", "-nostartfiles",
                "-static", "-Wl,-static",
                "-Wl,--start-group", "-Wl,--whole-archive",
                "-Wl,--no-whole-archive", "-Wl,--end-group",
                "-Wl,-EL", "-Wl,--no-relax",
                {force = true})
target_end()
