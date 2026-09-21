/**
* \file MPU_SSP.cpp
* This example allows to connect to a STM32 MPU based device through USB interface and launch the SSP process. \n
*		- Connect to the device via USB interface.
*		- Start the SSP programming
*		- Disconnect if an error occured.
*.
* Go to the source code : \ref USB_Bootloader
\example MPU_SSP
* This example allows to connect to a STM32 MPU based device through USB interface and launch the SSP process. \n
*		- Connect to the device via USB interface.
*		- Start the SSP programming
*		- Disconnect if an error occured.
*  \code{.cpp}
**/


#include <MPU_SSP.h>
#include <DisplayManager.h>
#include <CubeProgrammer_API.h>

int MPU_SSP(void) {

	logMessage(Title, "\n+++ MPU SSP +++\n\n");

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
		disconnect();
		deleteInterfaceList();
		return 0;
	}
	logMessage(Normal, "\nDevice name : %s ", genInfo->name);
	logMessage(Normal, "\nDevice type : %s ", genInfo->type);
	logMessage(Normal, "\nDevice CPU  : %s \n", genInfo->cpu);

	/* SSP Input binaries */
	const wchar_t* sspFilePath = L""; //Indicate the SSP image path here.
	const wchar_t* tfaSspFilePath = L""; //Indicate the tfa ssp path here.

	/* licenseFile  : is Empty since it is not required when using HSM.
	   hsmSlotId = 0: try to change the index value in accordance with the OS settings. */

	int sspFlag = programSsp(sspFilePath, L"", tfaSspFilePath, 0);
	if (sspFlag != 0)
	{
		disconnect();
		deleteInterfaceList();
		return 0;
	}

	/* Process successfully Done */
	disconnect();
	deleteInterfaceList();
	return 1;
}

/** \endcode **/
