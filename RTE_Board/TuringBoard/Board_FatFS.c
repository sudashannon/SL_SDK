#include "Board_FatFS.h"
#include "Board_SDCard.h"
char SDPath[4];   /* SD logical drive path */
FIL SDFile;       /* File object for SD */
FATFS SDFatFS;    /* File system object for SD logical drive */
static volatile DSTATUS Stat = STA_NOINIT;
static DSTATUS SD_CheckStatus(BYTE lun);
static DSTATUS SD_initialize (BYTE);
static DSTATUS SD_status (BYTE);
static DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
static DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
static DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */
const Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};
static DSTATUS SD_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;
  if(Board_SD_GetCardState() == 0x00)
  {
    Stat &= ~STA_NOINIT;
  }
  return Stat;
}
DSTATUS SD_initialize(BYTE lun)
{
  Stat = STA_NOINIT;
	Stat = SD_CheckStatus(lun);
  return Stat;
}
DSTATUS SD_status(BYTE lun)
{
  return SD_CheckStatus(lun);
}
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;

  if(Board_SD_ReadBlocks((uint32_t*)buff,
                       (uint32_t) (sector),
                       count, SDMMC_DATATIMEOUT) == 0x00)
  {
    /* wait until the read operation is finished */
    while(Board_SD_GetCardState()!= 0x00)
    {
    }
    res = RES_OK;
  }

  return res;
}
#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;

  if(Board_SD_WriteBlocks((uint32_t*)buff,
                        (uint32_t)(sector),
                        count, SDMMC_DATATIMEOUT) == 0x00)
  {
	/* wait until the Write operation is finished */
    while(Board_SD_GetCardState() != 0x00)
    {
    }
    res = RES_OK;
  }

  return res;
}
#endif

#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  HAL_SD_CardInfoTypeDef CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    Board_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    Board_SD_GetCardInfo(&CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    Board_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / 512;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */
void Board_FATFS_Init(void) 
{
	uint8_t retSD;    /* Return value for SD */
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
	/*##-1- Register the file system object to the FatFs module ##############*/
	retSD = f_mount(&SDFatFS, SDPath, 0);
	if(retSD)
	{
		RTE_Printf("[SDCard]    mount error : %d \r\n",retSD);
		RTE_Assert(__FILE__, __LINE__);
	}
	RTE_Printf("[SDCard]    Logic path:%s \r\n",SDPath);
}
DWORD get_fattime(void)
{
  return 0;
}
