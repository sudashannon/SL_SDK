#include "Board_SDRAM.h"
Board_Sdram_Control_t SdarmHandle = {0};
void Board_SDRAM_Init(void (*MSPInitCallback)(void),void (*MSPDeInitCallback)(void))
{
	SdarmHandle.MSPInitCallback = MSPInitCallback;
	SdarmHandle.MSPDeInitCallback = MSPDeInitCallback;
  FMC_SDRAM_TimingTypeDef SdramTiming;
  /** Perform the SDRAM2 memory initialization sequence
  */
  SdarmHandle.SdramHalHandle.Instance = FMC_SDRAM_DEVICE;
  /* SdarmHandle.SdramHalHandle.Init */
  SdarmHandle.SdramHalHandle.Init.SDBank = FMC_SDRAM_BANK2;
  SdarmHandle.SdramHalHandle.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
  SdarmHandle.SdramHalHandle.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  SdarmHandle.SdramHalHandle.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  SdarmHandle.SdramHalHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  SdarmHandle.SdramHalHandle.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  SdarmHandle.SdramHalHandle.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  SdarmHandle.SdramHalHandle.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  SdarmHandle.SdramHalHandle.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  SdarmHandle.SdramHalHandle.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 2;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;
  /* SDRAM controller initialization */
  if(HAL_SDRAM_Init(&SdarmHandle.SdramHalHandle, &SdramTiming) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
  /* SDRAM initialization sequence */
  Board_SDRAM_Initialization_sequence(REFRESH_COUNT);
}
void Board_SDRAM_DeInit(void)
{
  if(HAL_SDRAM_DeInit(&SdarmHandle.SdramHalHandle) != HAL_OK)
  {
    RTE_Assert(__FILE__, __LINE__);
  }
}
void Board_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
  __IO uint32_t tmpmrd = 0;
  /* Step 1: Configure a clock configuration enable command */
  SdarmHandle.SdramCommandHandle.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  SdarmHandle.SdramCommandHandle.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
  SdarmHandle.SdramCommandHandle.AutoRefreshNumber      = 1;
  SdarmHandle.SdramCommandHandle.ModeRegisterDefinition = 0;
  /* Send the command */
  HAL_SDRAM_SendCommand(&SdarmHandle.SdramHalHandle, &SdarmHandle.SdramCommandHandle, HAL_MAX_DELAY);
  /* Step 2: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);
  /* Step 3: Configure a PALL (precharge all) command */
  SdarmHandle.SdramCommandHandle.CommandMode            = FMC_SDRAM_CMD_PALL;
  SdarmHandle.SdramCommandHandle.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
  SdarmHandle.SdramCommandHandle.AutoRefreshNumber      = 1;
  SdarmHandle.SdramCommandHandle.ModeRegisterDefinition = 0;
  /* Send the command */
  HAL_SDRAM_SendCommand(&SdarmHandle.SdramHalHandle, &SdarmHandle.SdramCommandHandle, HAL_MAX_DELAY);
  /* Step 4: Configure an Auto Refresh command */
  SdarmHandle.SdramCommandHandle.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  SdarmHandle.SdramCommandHandle.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
  SdarmHandle.SdramCommandHandle.AutoRefreshNumber      = 8;
  SdarmHandle.SdramCommandHandle.ModeRegisterDefinition = 0;
  /* Send the command */
  HAL_SDRAM_SendCommand(&SdarmHandle.SdramHalHandle, &SdarmHandle.SdramCommandHandle, HAL_MAX_DELAY);

  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
                     SDRAM_MODEREG_CAS_LATENCY_3           |\
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  SdarmHandle.SdramCommandHandle.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  SdarmHandle.SdramCommandHandle.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
  SdarmHandle.SdramCommandHandle.AutoRefreshNumber      = 1;
  SdarmHandle.SdramCommandHandle.ModeRegisterDefinition = tmpmrd;
  /* Send the command */
  HAL_SDRAM_SendCommand(&SdarmHandle.SdramHalHandle, &SdarmHandle.SdramCommandHandle, HAL_MAX_DELAY);
  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  HAL_SDRAM_ProgramRefreshRate(&SdarmHandle.SdramHalHandle, RefreshCount);
}
void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram)
{
	if(SdarmHandle.MSPInitCallback)
		SdarmHandle.MSPInitCallback();
	else
		 RTE_Assert(__FILE__, __LINE__);
}
void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram)
{
	if(SdarmHandle.MSPDeInitCallback)
		SdarmHandle.MSPDeInitCallback();
	else
		 RTE_Assert(__FILE__, __LINE__);
}
