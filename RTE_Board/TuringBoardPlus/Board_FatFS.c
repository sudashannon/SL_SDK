#include "Board_FatFS.h"
#include "Board_SDCard.h"
Board_FatFS_Control_t FatFSHandle = {0};
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
  retSD = FATFS_LinkDriver(&SD_Driver, FatFSHandle.SDPath);
	/*##-1- Register the file system object to the FatFs module ##############*/
	retSD = f_mount(&FatFSHandle.SDFatFS, FatFSHandle.SDPath, 0);
	if(retSD)
	{
		RTE_Printf("%10s    mount error : %d \r\n","[SDCARD]",retSD);
		RTE_Assert(__FILE__, __LINE__);
	}
	RTE_Printf("%10s    Logic path:%s \r\n","[SDCARD]",FatFSHandle.SDPath);
	FATFS *fs;
	DWORD fre_clust;
  /* Get volume information and free clusters of drive */
  if ((retSD = f_getfree(FatFSHandle.SDPath, &fre_clust, &fs)) != FR_OK) {
		RTE_Printf("%10s    Read sd infor error : %d \r\n","[SDCARD]",retSD);
		RTE_Assert(__FILE__, __LINE__);
	}
	/* Get total sectors and free sectors */
	FatFSHandle.Total = (fs->n_fatent - 2) * fs->csize * 0.5;
	FatFSHandle.Free = fre_clust * fs->csize * 0.5;
	/* Format string */
	RTE_Printf("%10s    Total card size: %u kBytes\r\n","[SDCARD]", FatFSHandle.Total);
	/* Format string for free card size */
	RTE_Printf("%10s    Free card size:  %u kBytes\r\n","[SDCARD]", FatFSHandle.Free);
}
DWORD get_fattime(void)
{
  return RTE_RoundRobin_GetTick();
}
