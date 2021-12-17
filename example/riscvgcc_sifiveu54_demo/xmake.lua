-- define toolchain
toolchain("riscvgcc")
    set_kind("standalone")
    set_sdkdir("C:\\UserProfession\\riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-w64-mingw32")
toolchain_end()

add_rules("mode.debug", "mode.release")

target("riscvgcc_sifiveu54_demo")
    set_toolchains("riscvgcc")
    set_kind("binary")
    add_files("*.c")
    add_files("../../sis/rsis/core/src/*.S", "../../sis/rsis/core/src/*.c")
    add_files("../../sis/rsis/device/sifiveu/src/system_sifiveu.c")
    -- Include path
    add_includedirs("../../hal/inc");
    add_includedirs("../../rte/inc");
    add_includedirs("../../osal/inc");
    -- Configuration
    set_warnings("all", "error")
    add_cxflags("-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany",
                "-ffunction-sections", "-fdata-sections" ,"-fno-common")
    add_asflags("-march=rv64gc", "-mabi=lp64d", "-mcmodel=medany",
                "-ffunction-sections", "-fdata-sections" ,"-fno-common")
    add_ldflags("-Tsifiveu54.lds",
                "-Wl,-Map,$(buildir)/r64_memory.map",
                "-Wl,--gc-sections",
                "-Wl,--check-sections", "-nostartfiles",
                {force = true})
--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
