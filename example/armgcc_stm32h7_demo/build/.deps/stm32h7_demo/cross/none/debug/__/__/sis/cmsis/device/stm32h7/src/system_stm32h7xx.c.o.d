{
    values = {
        [[C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.07\bin\arm-none-eabi-gcc]],
        {
            "-g",
            "-Wall",
            "-Werror",
            "-O0",
            "-std=gnu99",
            [[-I..\..\sis\cmsis\core\inc]],
            [[-I..\..\sis\cmsis\device\stm32h7\inc]],
            "-DSTM32H743xx",
            "-mcpu=cortex-m7",
            "-mthumb",
            "-mfpu=fpv5-d16",
            "-fdata-sections",
            "-ffunction-sections",
            "-Warray-bounds"
        }
    },
    depfiles_gcc = "build\\.objs\\stm32h7_demo\\cross\\none\\debug\\__\\__\\sis\\cmsis\\device\\stm32h7\\src\\system_stm32h7xx.c.o:  ..\\..\\sis\\cmsis\\device\\stm32h7\\src\\system_stm32h7xx.c  ..\\..\\sis\\cmsis\\device\\stm32h7\\inc/stm32h7xx.h  ..\\..\\sis\\cmsis\\device\\stm32h7\\inc/stm32h743xx.h  ..\\..\\sis\\cmsis\\core\\inc/core_cm7.h  ..\\..\\sis\\cmsis\\core\\inc/cmsis_version.h  ..\\..\\sis\\cmsis\\core\\inc/cmsis_compiler.h  ..\\..\\sis\\cmsis\\core\\inc/cmsis_gcc.h  ..\\..\\sis\\cmsis\\core\\inc/mpu_armv7.h  ..\\..\\sis\\cmsis\\device\\stm32h7\\inc/system_stm32h7xx.h\
",
    files = {
        [[..\..\sis\cmsis\device\stm32h7\src\system_stm32h7xx.c]]
    }
}