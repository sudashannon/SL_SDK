#include "quadspi.h"
#include "spi.h"
#include "driver_w25qxx.h"
#include "rte_include.h"
#include "hal_include.h"

/**< w25qxx handle */
w25qxx_handle_t ex_flash_handle;
w25qxx_handle_t db_flash_handle;

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_init(void)
{
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_deinit(void)
{
	if (HAL_SPI_DeInit(&hspi6) != HAL_OK) {
		RTE_LOGF("Deinit spi failed!");
	}
    return 0;
}

/**
 * @brief      interface spi qspi bus write read
 * @param[in]  instruction is the sent instruction
 * @param[in]  instruction_line is the instruction phy lines
 * @param[in]  address is the register address
 * @param[in]  address_line is the address phy lines
 * @param[in]  address_len is the address length
 * @param[in]  alternate is the register address
 * @param[in]  alternate_line is the alternate phy lines
 * @param[in]  alternate_len is the alternate length
 * @param[in]  dummy is the dummy cycle
 * @param[in]  *in_buf points to a input buffer
 * @param[in]  in_len is the input length
 * @param[out] *out_buf points to a output buffer
 * @param[in]  out_len is the output length
 * @param[in]  data_line is the data phy lines
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t w25qxx_interface_spi_write_read(uint8_t instruction, uint8_t instruction_line,
                                             uint32_t address, uint8_t address_line, uint8_t address_len,
                                             uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len,
                                             uint8_t dummy, uint8_t *in_buf, uint32_t in_len,
                                             uint8_t *out_buf, uint32_t out_len, uint8_t data_line)
{
    if (instruction_line || address_line || alternate_line || dummy || (data_line != 1))
    {
        return 1;
    }

    volatile uint8_t res;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET);

    if (in_len) {
        res = hal_device_write_async("spi_0", in_buf, in_len, 1000);
        if (res) {
            RTE_LOGI("async_error: %d", res);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
            return 1;
        }
    }
    if (out_len) {
        res = hal_device_read_async("spi_0", out_buf, &out_len, 1000);
        if (res) {
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
            return 1;
        }
    }

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
    return 0;
}

/**
 * @brief  interface qspi bus init
 * @return status code
 *         - 0 success
 *         - 1 qspi init failed
 * @note   none
 */
uint8_t w25qxx_interface_qspi_init(void)
{
    return 0;
}

/**
 * @brief  interface qspi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 qspi deinit failed
 * @note   none
 */
uint8_t w25qxx_interface_qspi_deinit(void)
{
	if (HAL_QSPI_DeInit(&hqspi) != HAL_OK) {
		RTE_LOGF("Deinit qspi failed!");
	}
    return 0;
}

/**
 * @brief      interface qspi bus write read
 * @param[in]  instruction is the sent instruction
 * @param[in]  instruction_line is the instruction phy lines
 * @param[in]  address is the register address
 * @param[in]  address_line is the address phy lines
 * @param[in]  address_len is the address length
 * @param[in]  alternate is the register address
 * @param[in]  alternate_line is the alternate phy lines
 * @param[in]  alternate_len is the alternate length
 * @param[in]  dummy is the dummy cycle
 * @param[in]  *in_buf points to a input buffer
 * @param[in]  in_len is the input length
 * @param[out] *out_buf points to a output buffer
 * @param[in]  out_len is the output length
 * @param[in]  data_line is the data phy lines
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t w25qxx_interface_qspi_write_read(uint8_t instruction, uint8_t instruction_line,
                                             uint32_t address, uint8_t address_line, uint8_t address_len,
                                             uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len,
                                             uint8_t dummy, uint8_t *in_buf, uint32_t in_len,
                                             uint8_t *out_buf, uint32_t out_len, uint8_t data_line)
{
    QSPI_CommandTypeDef cmd_handler;

    if (in_len && out_len)
    {
        return 2;
    }

    cmd_handler.Instruction = instruction;
    if (instruction_line == 0)
    {
        cmd_handler.InstructionMode = QSPI_INSTRUCTION_NONE;
    }
    else if (instruction_line == 1)
    {
        cmd_handler.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    }
    else if (instruction_line == 2)
    {
        cmd_handler.InstructionMode = QSPI_INSTRUCTION_2_LINES;
    }
    else if (instruction_line == 4)
    {
        cmd_handler.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    }
    else
    {
        return 2;
    }

    if (address_line == 0)
    {
        cmd_handler.AddressMode = QSPI_ADDRESS_NONE;
    }
    else if (address_line == 1)
    {
        cmd_handler.AddressMode = QSPI_ADDRESS_1_LINE;
    }
    else if (address_line == 2)
    {
        cmd_handler.AddressMode = QSPI_ADDRESS_2_LINES;
    }
    else if (address_line == 4)
    {
        cmd_handler.AddressMode = QSPI_ADDRESS_4_LINES;
    }
    else
    {
        return 2;
    }
    if (address_len == 0)
    {
        cmd_handler.Address = address & 0xFF;
        cmd_handler.AddressSize = QSPI_ADDRESS_8_BITS;
    }
    else if (address_len == 1)
    {
        cmd_handler.Address = address & 0xFF;
        cmd_handler.AddressSize = QSPI_ADDRESS_8_BITS;
    }
    else if (address_len == 2)
    {
        cmd_handler.Address = address & 0xFFFF;
        cmd_handler.AddressSize = QSPI_ADDRESS_16_BITS;
    }
    else if (address_len == 3)
    {
        cmd_handler.Address = address & 0xFFFFFF;
        cmd_handler.AddressSize = QSPI_ADDRESS_24_BITS;
    }
    else if (address_len == 4)
    {
        cmd_handler.Address = address & 0xFFFFFFFF;
        cmd_handler.AddressSize = QSPI_ADDRESS_32_BITS;
    }
    else
    {
        return 2;
    }

    cmd_handler.AlternateBytes = alternate;
    if (alternate_line == 0)
    {
        cmd_handler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    }
    else if (alternate_line == 1)
    {
        cmd_handler.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;
    }
    else if (alternate_line == 2)
    {
        cmd_handler.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;
    }
    else if (alternate_line == 4)
    {
        cmd_handler.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    }
    else
    {
        return 2;
    }
    if (alternate_len == 0)
    {
        cmd_handler.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    }
    else if (alternate_len == 1)
    {
        cmd_handler.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    }
    else if (alternate_len == 2)
    {
        cmd_handler.AlternateBytesSize = QSPI_ALTERNATE_BYTES_16_BITS;
    }
    else if (alternate_len == 3)
    {
        cmd_handler.AlternateBytesSize = QSPI_ALTERNATE_BYTES_24_BITS;
    }
    else if (alternate_len == 4)
    {
        cmd_handler.AlternateBytesSize = QSPI_ALTERNATE_BYTES_32_BITS;
    }
    else
    {
        return 2;
    }

    cmd_handler.DummyCycles = dummy;

    if (data_line == 0)
    {
        cmd_handler.DataMode = QSPI_DATA_NONE;
    }
    else if (data_line == 1)
    {
        cmd_handler.DataMode = QSPI_DATA_1_LINE;
    }
    else if (data_line == 2)
    {
        cmd_handler.DataMode = QSPI_DATA_2_LINES;
    }
    else if (data_line == 4)
    {
        cmd_handler.DataMode = QSPI_DATA_4_LINES;
    }
    else
    {
        return 2;
    }

    cmd_handler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd_handler.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd_handler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;

    if (HAL_QSPI_Command(&hqspi, &cmd_handler, 1000))
    {
        return 1;
    }

    if (in_len)
    {
        hqspi.Instance->DLR = in_len - 1;
        if (HAL_QSPI_Transmit(&hqspi, in_buf, 1000))
        {
            return 1;
        }
    }

    if (out_len)
    {
        hqspi.Instance->DLR = out_len - 1;
        if (HAL_QSPI_Receive(&hqspi, out_buf, 1000))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void w25qxx_interface_delay_ms(uint32_t ms)
{
    rte_delay_ms(ms);
}

/**
 * @brief     interface delay us
 * @param[in] us
 * @note      none
 */
extern void rte_delay_us(volatile uint32_t micros);
void w25qxx_interface_delay_us(uint32_t us)
{
    rte_delay_us(us);
}

/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @return    length of the send data
 * @note      none
 */
uint16_t w25qxx_interface_debug_print(char *format, ...)
{
    va_list args;
    va_start(args, format);
    log_vprintf(format, args);
    va_end(args);
    return 0;
}

void driver_w25qxx_init(void)
{
    /* link interface function */
    DRIVER_W25QXX_LINK_INIT(&ex_flash_handle, w25qxx_handle_t);
    DRIVER_W25QXX_LINK_SPI_QSPI_INIT(&ex_flash_handle, w25qxx_interface_qspi_init);
    DRIVER_W25QXX_LINK_SPI_QSPI_DEINIT(&ex_flash_handle, w25qxx_interface_qspi_deinit);
    DRIVER_W25QXX_LINK_SPI_QSPI_WRITE_READ(&ex_flash_handle, w25qxx_interface_qspi_write_read);
    DRIVER_W25QXX_LINK_DELAY_MS(&ex_flash_handle, w25qxx_interface_delay_ms);
    DRIVER_W25QXX_LINK_DELAY_US(&ex_flash_handle, w25qxx_interface_delay_us);
    DRIVER_W25QXX_LINK_DEBUG_PRINT(&ex_flash_handle, w25qxx_interface_debug_print);
    /* link interface function */
    DRIVER_W25QXX_LINK_INIT(&db_flash_handle, w25qxx_handle_t);
    DRIVER_W25QXX_LINK_SPI_QSPI_INIT(&db_flash_handle, w25qxx_interface_spi_init);
    DRIVER_W25QXX_LINK_SPI_QSPI_DEINIT(&db_flash_handle, w25qxx_interface_spi_deinit);
    DRIVER_W25QXX_LINK_SPI_QSPI_WRITE_READ(&db_flash_handle, w25qxx_interface_spi_write_read);
    DRIVER_W25QXX_LINK_DELAY_MS(&db_flash_handle, w25qxx_interface_delay_ms);
    DRIVER_W25QXX_LINK_DELAY_US(&db_flash_handle, w25qxx_interface_delay_us);
    DRIVER_W25QXX_LINK_DEBUG_PRINT(&db_flash_handle, w25qxx_interface_debug_print);
}

#if RTE_SHELL_ENABLE == 1
static uint8_t gs_buffer_input[600];         /**< input buffer */
static uint8_t gs_buffer_output[600];        /**< output buffer */
static const uint32_t gsc_size[] = {0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000};        /**< flash size */

/**
 * @brief     qspi test
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t w25qxx_qspi_test(void)
{
    volatile uint8_t res;
    w25qxx_type_t type = W25Q64;
    /* start read test */
    w25qxx_interface_debug_print("w25qxx: start read test.\r\n");
    volatile uint32_t size;
    volatile uint32_t addr, step, j;

    size = gsc_size[type - W25Q80];
    step = size / 16;

    /* w25qxx_write/w25qxx_read test */
    w25qxx_interface_debug_print("w25qxx: w25qxx_write/w25qxx_read test.\r\n");

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* w25qxx_sector_erase_4k test */
    addr = (rand() % 10) * 4 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_sector_erase_4k test with address 0x%X.\r\n", addr);
    res = w25qxx_sector_erase_4k(&ex_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 4k failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    /* w25qxx_block_erase_32k test */
    addr = (rand() % 10) * 32 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_32k test with address 0x%X.\r\n", addr);
    res = w25qxx_block_erase_32k(&ex_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 32k failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    /* w25qxx_block_erase_64k test */
    addr = (rand() % 10) * 64 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_64k test with address 0x%X.\r\n", addr);
    res = w25qxx_block_erase_64k(&ex_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 64k failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&ex_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    #if (W25QXX_ENABLE_ERASE_READ_TEST == 1)
        /* start chip erasing */
        w25qxx_interface_debug_print("w25qxx: start chip erasing.\r\n");

        /* chip erase */
        w25qxx_interface_debug_print("w25qxx: w25qxx_chip_erase test.\r\n");
        res = w25qxx_chip_erase(&ex_flash_handle);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: chip erase failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        w25qxx_interface_debug_print("w25qxx: chip erase successful.\r\n");
    #endif

    if (type >= W25Q256)
    {
        /* set address mode 4 byte */
        w25qxx_interface_debug_print("w25qxx: set address mode 4 byte.\r\n");

        /* set address mode 4 byte */
        res = w25qxx_set_address_mode(&ex_flash_handle, W25QXX_ADDRESS_MODE_4_BYTE);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: set address mode failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }

        /* w25qxx_write/w25qxx_read test */
        w25qxx_interface_debug_print("w25qxx: w25qxx_write/w25qxx_read test.\r\n");

        for (addr = 0; addr < size; addr += step)
        {
            for (j = 0; j < 600; j++)
            {
                gs_buffer_input[j] = rand() %256;
            }
            res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
            if (res)
            {
                w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
            res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
            if (res)
            {
                w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
            for (j = 0; j < 600; j++)
            {
                if (gs_buffer_input[j] != gs_buffer_output[j])
                {
                    w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                    w25qxx_deinit(&ex_flash_handle);

                    return 1;
                }
            }
            w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
        }

        /* w25qxx_sector_erase_4k test */
        addr = (rand() % 10) * 4 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_sector_erase_4k test with address 0x%X.\r\n", addr);
        res = w25qxx_sector_erase_4k(&ex_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 4k failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* w25qxx_block_erase_32k test */
        addr = (rand() % 10) * 32 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_32k test with address 0x%X.\r\n", addr);
        res = w25qxx_block_erase_32k(&ex_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 32k failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* w25qxx_block_erase_64k test */
        addr = (rand() % 10) * 64 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_64k test with address 0x%X.\r\n", addr);
        res = w25qxx_block_erase_64k(&ex_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 64k failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&ex_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&ex_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* set address mode 3 byte */
        w25qxx_interface_debug_print("w25qxx: set address mode 3 byte.\r\n");

        /* set address mode 3 byte */
        res = w25qxx_set_address_mode(&ex_flash_handle, W25QXX_ADDRESS_MODE_3_BYTE);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: set address mode failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
    }

    /* w25qxx_set_read_parameters test */
    w25qxx_interface_debug_print("w25qxx: w25qxx_set_read_parameters test.\r\n");

    /* 8 dummy max 80MHz test */
    w25qxx_interface_debug_print("w25qxx: set 8 dummy max 80MHz test.\r\n");

    res = w25qxx_set_read_parameters(&ex_flash_handle, W25QXX_QSPI_READ_DUMMY_8_80MHZ, W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: set read parameters.\r\n");

        return 1;
    }

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* 6 dummy max 80MHz test */
    w25qxx_interface_debug_print("w25qxx: set 6 dummy max 80MHz test.\r\n");

    res = w25qxx_set_read_parameters(&ex_flash_handle, W25QXX_QSPI_READ_DUMMY_6_80MHZ, W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: set read parameters.\r\n");

        return 1;
    }

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* 4 dummy max 55MHz test */
    w25qxx_interface_debug_print("w25qxx: set 4 dummy max 55MHz test.\r\n");

    res = w25qxx_set_read_parameters(&ex_flash_handle, W25QXX_QSPI_READ_DUMMY_4_55MHZ, W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: set read parameters.\r\n");

        return 1;
    }

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* 2 dummy max 33MHz test */
    w25qxx_interface_debug_print("w25qxx: set 2 dummy max 33MHz test.\r\n");

    res = w25qxx_set_read_parameters(&ex_flash_handle, W25QXX_QSPI_READ_DUMMY_2_33MHZ, W25QXX_QSPI_READ_WRAP_LENGTH_8_BYTE);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: set read parameters.\r\n");

        return 1;
    }

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&ex_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        res = w25qxx_read(&ex_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&ex_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&ex_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* finish read test */
    w25qxx_interface_debug_print("w25qxx: finish read test.\r\n");
    w25qxx_deinit(&ex_flash_handle);

    return 0;
}

/**
 * @brief     spi test
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t w25qxx_spi_test(void)
{
    volatile uint8_t res;
    w25qxx_type_t type = W25Q64;
    volatile uint32_t size;
    volatile uint32_t addr, step, j;

    size = gsc_size[type - W25Q80];
    step = size / 16;

    /* w25qxx_write/w25qxx_read test */
    w25qxx_interface_debug_print("w25qxx: w25qxx_write/w25qxx_read test.\r\n");

    for (addr = 0; addr < size; addr += step)
    {
        for (j = 0; j < 600; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_write(&db_flash_handle, addr, gs_buffer_input, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        res = w25qxx_read(&db_flash_handle, addr, gs_buffer_output, 600);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 600; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: 0x%08X/0x%08X successful.\r\n", addr, size);
    }

    /* w25qxx_sector_erase_4k test */
    addr = (rand() % 10) * 4 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_sector_erase_4k test with address 0x%X.\r\n", addr);
    res = w25qxx_sector_erase_4k(&db_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 4k failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }

    /* w25qxx_only_spi_read */
    res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    /* w25qxx_block_erase_32k test */
    addr = (rand() % 10) * 32 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_32k test with address 0x%X.\r\n", addr);
    res = w25qxx_block_erase_32k(&db_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 32k failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }

    /* w25qxx_only_spi_read */
    res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    /* w25qxx_block_erase_64k test */
    addr = (rand() % 10) * 64 * 1024;
    w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_64k test with address 0x%X.\r\n", addr);
    res = w25qxx_block_erase_64k(&db_flash_handle, addr);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: sector erase 64k failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }

    /* w25qxx_page_program */
    res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }

    /* w25qxx_only_spi_read */
    res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

    /* w25qxx_fast_read */
    res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

    /* get sfdp */
    w25qxx_interface_debug_print("w25qxx: get sfdp.\r\n");
    memset(gs_buffer_output, 0, sizeof(uint8_t) * 256);
    res = w25qxx_get_sfdp(&db_flash_handle, (uint8_t *)gs_buffer_output);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: get sfdp failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j += 8)
    {
        w25qxx_interface_debug_print("w25qxx: sdfp[%d-%d] is 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X.\r\n",
                                     j , j + 7,
                                     gs_buffer_output[j + 0], gs_buffer_output[j + 1], gs_buffer_output[j + 2],
                                     gs_buffer_output[j + 3], gs_buffer_output[j + 4], gs_buffer_output[j + 5],
                                     gs_buffer_output[j + 6], gs_buffer_output[j + 7]);
    }

    /*  security register1 write and read test */
    w25qxx_interface_debug_print("w25qxx: security register1 write and read test.\r\n");

    /* security register1 write and read test */
    res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }
    res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1, gs_buffer_input);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1, gs_buffer_output);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: security register1 check passed.\r\n");

    /*  security register2 write and read test */
    w25qxx_interface_debug_print("w25qxx: security register2 write and read test.\r\n");

    /* security register2 write and read test */
    res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }
    res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2, gs_buffer_input);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2, gs_buffer_output);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: security register2 check passed.\r\n");

    /*  security register3 write and read test */
    w25qxx_interface_debug_print("w25qxx: security register3 write and read test.\r\n");

    /* security register3 write and read test */
    res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        gs_buffer_input[j] = rand() %256;
    }
    res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3, gs_buffer_input);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3, gs_buffer_output);
    if (res)
    {
        w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
        w25qxx_deinit(&db_flash_handle);

        return 1;
    }
    for (j = 0; j < 256; j++)
    {
        if (gs_buffer_input[j] != gs_buffer_output[j])
        {
            w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }
    w25qxx_interface_debug_print("w25qxx: security register3 check passed.\r\n");

    #if (W25QXX_ENABLE_ERASE_READ_TEST == 1)
        /* start chip erasing */
        w25qxx_interface_debug_print("w25qxx: start chip erasing.\r\n");

        /* chip erase */
        w25qxx_interface_debug_print("w25qxx: w25qxx_chip_erase test.\r\n");
        res = w25qxx_chip_erase(&db_flash_handle);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: chip erase failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        w25qxx_interface_debug_print("w25qxx: chip erase successful.\r\n");
    #endif

    if (type >= W25Q256)
    {
        /* set address mode 4 byte */
        w25qxx_interface_debug_print("w25qxx: set address mode 4 byte.\r\n");

        /* set address mode 4 byte */
        res = w25qxx_set_address_mode(&db_flash_handle, W25QXX_ADDRESS_MODE_4_BYTE);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: set address mode failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }

        /* w25qxx_write/w25qxx_read test */
        w25qxx_interface_debug_print("w25qxx: w25qxx_write/w25qxx_read test.\r\n");

        for (addr = 0; addr < size; addr += step)
        {
            for (j = 0; j < 600; j++)
            {
                gs_buffer_input[j] = rand() %256;
            }
            res = w25qxx_write(&db_flash_handle, addr, gs_buffer_input, 600);
            if (res)
            {
                w25qxx_interface_debug_print("w25qxx: write failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
            res = w25qxx_read(&db_flash_handle, addr, gs_buffer_output, 600);
            if (res)
            {
                w25qxx_interface_debug_print("w25qxx: read failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
            for (j = 0; j < 600; j++)
            {
                if (gs_buffer_input[j] != gs_buffer_output[j])
                {
                    w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                    w25qxx_deinit(&db_flash_handle);

                    return 1;
                }
            }
            w25qxx_interface_debug_print("w25qxx: %d/%d successful.\r\n", addr, size);
        }

        /* w25qxx_sector_erase_4k test */
        addr = (rand() % 10) * 4 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_sector_erase_4k test with address 0x%X.\r\n", addr);
        res = w25qxx_sector_erase_4k(&db_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 4k failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }

        /* w25qxx_only_spi_read */
        res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* w25qxx_block_erase_32k test */
        addr = (rand() % 10) * 32 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_32k test with address 0x%X.\r\n", addr);
        res = w25qxx_block_erase_32k(&db_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 32k failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }

        /* w25qxx_only_spi_read */
        res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* w25qxx_block_erase_64k test */
        addr = (rand() % 10) * 64 * 1024;
        w25qxx_interface_debug_print("w25qxx: w25qxx_block_erase_64k test with address 0x%X.\r\n", addr);
        res = w25qxx_block_erase_64k(&db_flash_handle, addr);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: sector erase 64k failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }

        /* w25qxx_page_program */
        res = w25qxx_page_program(&db_flash_handle, addr, gs_buffer_input, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: page program failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }

        /* w25qxx_only_spi_read */
        res = w25qxx_only_spi_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: only spi read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: only spi read test passed.\r\n");

        /* w25qxx_fast_read */
        res = w25qxx_fast_read(&db_flash_handle, addr, gs_buffer_output, 256);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: fast read failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: fast read test passed.\r\n");

        /* get sfdp */
        w25qxx_interface_debug_print("w25qxx: get sfdp.\r\n");
        memset(gs_buffer_output, 0, sizeof(uint8_t) * 256);
        res = w25qxx_get_sfdp(&db_flash_handle, (uint8_t *)gs_buffer_output);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: get sfdp failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j += 8)
        {
            w25qxx_interface_debug_print("w25qxx: sdfp[%d-%d] is 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X.\r\n",
                                         j , j + 7,
                                         gs_buffer_output[j + 0], gs_buffer_output[j + 1], gs_buffer_output[j + 2],
                                         gs_buffer_output[j + 3], gs_buffer_output[j + 4], gs_buffer_output[j + 5],
                                         gs_buffer_output[j + 6], gs_buffer_output[j + 7]);
        }

        /*  security register1 write and read test */
        w25qxx_interface_debug_print("w25qxx: security register1 write and read test.\r\n");

        /* security register1 write and read test */
        res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1, gs_buffer_input);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_1, gs_buffer_output);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: security register1 check passed.\r\n");

        /*  security register2 write and read test */
        w25qxx_interface_debug_print("w25qxx: security register2 write and read test.\r\n");

        /* security register2 write and read test */
        res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2, gs_buffer_input);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_2, gs_buffer_output);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: security register2 check passed.\r\n");

        /*  security register3 write and read test */
        w25qxx_interface_debug_print("w25qxx: security register3 write and read test.\r\n");

        /* security register3 write and read test */
        res = w25qxx_erase_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: erase security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            gs_buffer_input[j] = rand() %256;
        }
        res = w25qxx_program_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3, gs_buffer_input);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: program security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        res = w25qxx_read_security_register(&db_flash_handle, W25QXX_SECURITY_REGISTER_3, gs_buffer_output);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: read security register failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
        for (j = 0; j < 256; j++)
        {
            if (gs_buffer_input[j] != gs_buffer_output[j])
            {
                w25qxx_interface_debug_print("w25qxx: write read check failed.\r\n");
                w25qxx_deinit(&db_flash_handle);

                return 1;
            }
        }
        w25qxx_interface_debug_print("w25qxx: security register3 check passed.\r\n");

        /* set address mode 3 byte */
        w25qxx_interface_debug_print("w25qxx: set address mode 3 byte.\r\n");

        /* set address mode 3 byte */
        res = w25qxx_set_address_mode(&db_flash_handle, W25QXX_ADDRESS_MODE_3_BYTE);
        if (res)
        {
            w25qxx_interface_debug_print("w25qxx: set address mode failed.\r\n");
            w25qxx_deinit(&db_flash_handle);

            return 1;
        }
    }

    /* finish read test */
    w25qxx_interface_debug_print("w25qxx: finish read test.\r\n");
    w25qxx_deinit(&db_flash_handle);
    return 0;
}

int shell_cmd_qspitest(int argc, char *const argv[])
{
    if (argc == 1) {
        w25qxx_qspi_test();
        return 0;
    } else {
        return -1;
    }
}

SHELL_ADD_CMD(qspitest, shell_cmd_qspitest,
                    "test w25qxx exe flash with qspi interface.","\r\r\n");

int shell_cmd_spitest(int argc, char *const argv[])
{
    if (argc == 1) {
        w25qxx_spi_test();
        return 0;
    } else {
        return -1;
    }
}

SHELL_ADD_CMD(spitest, shell_cmd_spitest,
                    "test w25qxx database flash with qspi interface.","\r\r\n");

#endif
