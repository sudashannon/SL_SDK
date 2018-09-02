#include "BSP_Time.h"
const static char *g_ppcMonth[12] =
{
    "1-",
    "2-",
    "3-",
    "4-",
    "5-",
    "6-",
    "7-",
    "8-",
    "9-",
    "10-",
    "11-",
    "12-"
};
void BSP_Time_Init(void)
{
	// Enable the hibernate module.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
	// Check to see if Hibernation module is already active, which could mean
	// that the processor is waking from a hibernation.
	// Read the status bits to see what caused the wake.  Clear the wake
	// source so that the device can be put into hibernation again.
	uint32_t ui32Status = HibernateIntStatus(0);
	HibernateIntClear(ui32Status);
	// Configure Hibernate module clock.
	HibernateEnableExpClk(SystemCoreClock);
	// Configure the module clock source.
	HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);
	// Enable RTC mode.
	HibernateRTCEnable();
	// Configure the hibernate module counter to 24-hour calendar mode.
	HibernateCounterMode(HIBERNATE_COUNTER_24HR);
}
//*****************************************************************************
//
// This function reads the current date and time from the calendar logic of the
// hibernate module.  Return status indicates the validity of the data read.
// If the received data is valid, the 24-hour time format is converted to
// 12-hour format.
//
//*****************************************************************************
bool BSP_Time_Get(struct tm *sTime)
{
    //
    // Get the latest time.
    //
    HibernateCalendarGet(sTime);

    //
    // Is valid data read?
    //
    if(((sTime->tm_sec < 0) || (sTime->tm_sec > 59)) ||
       ((sTime->tm_min < 0) || (sTime->tm_min > 59)) ||
       ((sTime->tm_hour < 0) || (sTime->tm_hour > 23)) ||
       ((sTime->tm_mday < 1) || (sTime->tm_mday > 31)) ||
       ((sTime->tm_mon < 0) || (sTime->tm_mon > 11)) ||
       ((sTime->tm_year < 100) || (sTime->tm_year > 199)))
    {
        //
        // No - Let the application know the same by returning relevant
        // message.
        //
        return false;
    }

    //
    // Return that new data is available so that it can be displayed.
    //
    return true;
}

//*****************************************************************************
//
// This function formats valid new date and time to be displayed on the home
// screen in the format "MMM DD, YYYY  HH : MM : SS AM/PM".  Example of this
// format is Aug 01, 2013  08:15:30 AM.  It also indicates if valid new data
// is available or not.  If date and time is invalid, this function sets the
// date and time to default value.
//
//*****************************************************************************
bool BSP_Time_Display(void)
{
	static uint32_t ui32SecondsPrev = 0xFF;
	struct tm sTime;
	// Get the latest date and time and check the validity.
	if(BSP_Time_Get(&sTime) == false)
	{
			//
			// Invalid - Force set the date and time to default values and return
			// false to indicate no information to display.
			//
			return false;
	}
	// If date and time is valid, check if seconds have updated from previous
	// visit.
	if(ui32SecondsPrev == sTime.tm_sec)
	{
			//
			// No - Return false to indicate no information to display.
			//
			return false;
	}
	// If valid new date and time is available, update a local variable to keep
	// track of seconds to determine new data for next visit.
	ui32SecondsPrev = sTime.tm_sec;
	// Format the date and time into a user readable format.
	printf("[TIME]    系统时间：%s%02u %02u:%02u:%02u\n",g_ppcMonth[sTime.tm_mon], sTime.tm_mday,
						sTime.tm_hour, sTime.tm_min, sTime.tm_sec);
	// Return true to indicate new information to display.
	return true;
}
//*****************************************************************************
//
// This function writes the requested date and time to the calendar logic of
// hibernation module.
//
//*****************************************************************************
void BSP_Time_Set(uint32_t g_ui32HourIdx,uint32_t g_ui32MinIdx,uint32_t g_ui32MonthIdx,uint32_t g_ui32DayIdx
	,uint32_t g_ui32YearIdx)
{
    struct tm sTime;
    // Get the latest date and time.  This is done here so that unchanged
    // parts of date and time can be written back as is.
    HibernateCalendarGet(&sTime);
    // Set the date and time values that are to be updated.
    sTime.tm_hour = g_ui32HourIdx;
    sTime.tm_min = g_ui32MinIdx;
    sTime.tm_mon = g_ui32MonthIdx;
    sTime.tm_mday = g_ui32DayIdx;
    sTime.tm_year = 100 + g_ui32YearIdx;
    // Update the calendar logic of hibernation module with the requested data.
    HibernateCalendarSet(&sTime);
}

