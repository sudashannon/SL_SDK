# SL_RTE
一款基于RTX操作系统、Cotex-M内核下通用软件运行环境，加速嵌入式软件开发。
# 主要内容
RTE框架如下：

-RTE

--SL_APP

---包含内存管理、类shell交互、不同Core的文件、一个环形队列、一个状态机实现、一个软件定时器、字符串处理库等内容。
--SL_BSP
---包含任何Chip必有的三个部分：按键、串口、LED以方便用户对运行于SL_RTE下的代码进行调试。
--SL_ThirdParty
---包含使用到的第三方代码。
--Board_Collection
---针对具体使用的开发板所搜集或编写的代码目前包含以下部分：
---STM32F103：ADC、BH1750、CRC、DHT11、E2PROM、ESP8266、片内Flash模拟E2PROM、GSMA9、I2C、LCD、PM25、PWM
              RC522、IO模拟串口、SPI、SR501、GPIO。
   STM32F407：类似F1。
   IOTL475：移植官方驱动。
   STM32F767:待上传。
   STM32H743：待上传。
--Example
---KEIL环境下本RTE使用的DEMO。
--Docs
---包含TXT格式的版本历史纪录和DOC格式的用户手册。
# 版本历史
2017/09/22
新版本1.0release：
2017/09/23
完全剥离bsp和app 确保app使用时独立于硬件底层（SoftTimer除外 但针对coterxm内核mcu一致）
加入bget内存管理 将部分app变为动态内存分配机制（用于debug的串口缓存）
修改ringbuffer 使出列和入列变为动态内存分配
2017/11/06
结合tmlib开始2.0版本编写 基于F7以及HAL库
2017/11/12
2.0版本的除了spi其他都已经测试通过
为了配合mppt项目开始完成针对f1系列以及固件库的适配
修改app中led和key到bsp中去
2017/11/13
完成com key led对于f1的适配
修改了key和led使其适配于m3和m4系列内核
单独为m3内核建立comf1
确定新bsp撰写规范
攥写了bsp_timerbase
2018/04/17
新版本开始编写 3.0
app_mem 更换rtx自带的内存管理系统做为底层 方便管理多块内存空间。
增加app_stdio 替换printf
更换APP_Config和BSP_Config为适用于MDK开发环境，可勾选配置。
2018/05/30
基本完成F1板级支持库的开发；
开始做F4的板级支持库和bsp包；
2018/06/15
完成对bget的重写 使其支持多块内存 删除app_mem
重命名SL_LIB为SL-RTE
修改F1的串口接收为DMA工作模式
优化对MDK开发的适配
最新的RTE在STM32F103C8T平台上通过1ms循环 debug指令暴力测试
2018/06/23
决定开源，上传到github。
# 其他
等有空了再写。
