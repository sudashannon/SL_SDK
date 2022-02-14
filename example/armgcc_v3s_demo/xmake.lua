-- define toolchain
toolchain("armgcc")
    set_kind("standalone")
    set_sdkdir("C:\\UserProfession\\gcc-arm-10.3-2021.07-mingw-w64-i686-arm-none-linux-gnueabihf\\")
toolchain_end()

target("v3s_boot0")
    add_rules("mode.debug", "mode.release")
    set_toolchains("armgcc")
    set_kind("binary")
    -- set warning all as error
    set_warnings("all", "error")
    -- set language: gnu99, gnu++11
    set_languages("gnu99", "gnu++11")
    add_files("../../sis/casis/core/src/*.c",
                "../../sis/casis/core/src/*.S",
                "../../sis/casis/device/allwinner_v3s/src/GCC/startup_ARMCA7.c",
                "../../sis/casis/device/allwinner_v3s/src/system_ARMCA7.c")
    add_includedirs("./user_config",
                    "../../sis/casis/core/inc",
                    "../../sis/casis/device/allwinner_v3s/inc",
                    "../../rte/inc",
                    "../../hal/inc")
    add_cxflags("-march=armv7-a", "-mtune=cortex-a7", "-mfpu=vfpv4", "-mfloat-abi=hard",
                "-marm", "-mno-thumb-interwork", "-mno-unaligned-access",
                "-DCASIS_device_header=\"ARMCA7.h\"",
                "-O0",
                "-Wall", "-fdata-sections", "-ffunction-sections",
                {force = true})
    add_asflags("-march=armv7-a", "-mtune=cortex-a7", "-mfpu=vfpv4", "-mfloat-abi=hard",
                "-marm", "-mno-thumb-interwork", "-mno-unaligned-access",
                "-O0",
                "-Wall", "-fdata-sections", "-ffunction-sections",
                {force = true})
    -- set ld configurations
    add_ldflags("-Tdefault.ld",
                "-nostdlib",
                "-lgcc",
                "-Wl,--cref",
                "-Wl,--gc-sections",
                "-Wl,-Map=build/v3s_boot0.map",
                {force = true})
if is_arch("armv7-a") then
    -- before_build(
    --     function(general_link_file)
    --         os.exec("cp .\\formal.ld .\\formal.c")
    --         os.exec("arm-none-linux-gnueabihf-gcc.exe -E -P .\\formal.c -o .\\default.ld")
    --         os.exec("rm .\\formal.c")
    --     end
    -- )
    after_build(
        function(generate_bin)
            os.exec("arm-none-linux-gnueabihf-objcopy.exe -O binary .\\build\\cross\\armv7-a\\debug\\v3s_boot0 .\\v3s_boot0.bin")
            -- os.exec("rm .\\default.ld")
        end
    )
end
target_end()