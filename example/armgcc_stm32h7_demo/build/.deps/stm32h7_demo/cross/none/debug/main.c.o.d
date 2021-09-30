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
    depfiles_gcc = "build\\.objs\\stm32h7_demo\\cross\\none\\debug\\main.c.o: main.c\
",
    files = {
        "main.c"
    }
}