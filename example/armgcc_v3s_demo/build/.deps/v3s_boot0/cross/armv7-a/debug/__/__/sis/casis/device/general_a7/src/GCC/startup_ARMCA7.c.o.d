{
    depfiles_gcc = "build\\.objs\\v3s_boot0\\cross\\armv7-a\\debug\\__\\__\\sis\\casis\\device\\general_a7\\src\\GCC\\startup_ARMCA7.c.o:  ..\\..\\sis\\casis\\device\\general_a7\\src\\GCC\\startup_ARMCA7.c  ..\\..\\sis\\casis\\device\\general_a7\\inc/ARMCA7.h  ..\\..\\sis\\casis\\core\\inc/core_ca.h  ..\\..\\sis\\casis\\core\\inc/cmsis_compiler.h  ..\\..\\sis\\casis\\core\\inc/cmsis_gcc.h  ..\\..\\sis\\casis\\core\\inc/cmsis_cp15.h\
",
    values = {
        [[C:\UserProfession\gcc-arm-10.3-2021.07-mingw-w64-i686-arm-none-linux-gnueabihf\bin\arm-none-linux-gnueabihf-gcc]],
        {
            "-g",
            "-Werror",
            "-std=gnu99",
            "-Iuser_config",
            [[-I..\..\sis\casis\core\inc]],
            [[-I..\..\sis\casis\device\general_a7\inc]],
            [[-I..\..\rte\inc]],
            [[-I..\..\hal\inc]],
            "-march=armv7-a",
            "-mtune=cortex-a7",
            "-mfpu=vfpv4",
            "-mfloat-abi=hard",
            "-marm",
            "-mno-thumb-interwork",
            "-mno-unaligned-access",
            "-DCASIS_device_header=\"ARMCA7.h\"",
            "-O0",
            "-Wall",
            "-fdata-sections",
            "-ffunction-sections"
        }
    },
    files = {
        [[..\..\sis\casis\device\general_a7\src\GCC\startup_ARMCA7.c]]
    }
}