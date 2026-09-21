/**
* \file TSV_Flashing.cpp
* This example allows to connect to a STM32 MPU based device through USB interface and start the flashing service. \n
*		- Connect to the device via USB interface.
*		- TSV file programming.
*		- Disconnect
*.
* Go to the source code : \ref USB_Bootloader
\example TSV_Flashing
* This example allows to connect to a STM32 MPU based device through USB interface and start the flashing service. \n
*		- Connect to the device via USB interface.
*		- TSV file programming.
*		- Disconnect
*  \code{.cpp}
**/


#include <TSV_Flashing.h>
#include <DisplayManager.h>
#include <CubeProgrammer_API.h>

int TSV_Flashing(void) {

	logMessage(Title, "\n+++ TSV Flashing service [STM32MP15] +++\n\n");

	generalInf* genInfo;
	dfuDeviceInfo* dfuList;

	int getDfuListNb = getDfuDeviceList(&dfuList, 0xdf11, 0x0483);

	if (getDfuListNb == 0)
	{
		logMessage(Error, "No USB DFU available\n");
		return 0;
	}
	else {
		logMessage(Title, "\n------------- USB DFU List --------------\n");
		for (int i = 0; i < getDfuListNb; i++)
		{
			logMessage(Normal, "USB Port %d \n", i);
			logMessage(Info, "	USB index   : %s \n", dfuList[i].usbIndex);
			logMessage(Info, "	USB SN      : %s \n", dfuList[i].serialNumber);
			logMessage(Info, "	DFU version : 0x%02X ", dfuList[i].dfuVersion);
		}
		logMessage(Title, "\n-----------------------------------------\n\n");
	}

	/* Target connect, choose the adequate USB port by indicating its index that is already mentioned in USB DFU List above */
	int usbConnectFlag = connectDfuBootloader(dfuList[0].usbIndex);
	if (usbConnectFlag != 0)
	{
		logMessage(Error, "Failed to establish connection !\n");
		disconnect();
		return 0;
	}
	else {
		logMessage(GreenInfo, "\n--- Device Connected --- \n");
	}

	/* Display device information */
	genInfo = getDeviceGeneralInf();
	if (genInfo == nullptr)
	{
		logMessage(Error, "Failed to get device information");
	}
	logMessage(Normal, "\nDevice name : %s ", genInfo->name);
	logMessage(Normal, "\nDevice type : %s ", genInfo->type);
	logMessage(Normal, "\nDevice CPU  : %s \n", genInfo->cpu);

	/* Download binaries */
#ifdef _WIN32
	const wchar_t* tsvFilePath = L"../test file/STM32MP/FlashLayout_sdcard_stm32mp157c-dk2-trusted.tsv";
#else
	const wchar_t* tsvFilePath = L"../api/test file/STM32MP/FlashLayout_sdcard_stm32mp157c-dk2-trusted.tsv";
#endif

#ifdef _WIN32
	const wchar_t* binFilePath = L"../test file/STM32MP/";
#else
	const wchar_t* binFilePath = L"../api/test file/STM32MP/";
#endif

	unsigned int isVerify = 0;
	unsigned int isSkipErase = 1; 
	int downloadFileFlag = downloadFile(tsvFilePath, 0, 0, 0, binFilePath);
	if (downloadFileFlag != 0)
	{
		disconnect();
		return 0;
	}

	/* Process successfully Done */
	disconnect();
	deleteInterfaceList();
	return 1;
}

/** \endcode **/
