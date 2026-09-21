/**
* \file HSM_Example.cpp
* This example allows to execute some operations with the STM32 HSM. \n
* 1.	Detect plugged-in HSM cards. \n
* 2.	Get the HSM Firmware ID, counter, State, type and version \n
* 3.	Get the HSM license after device connection \n
*.
* Go to the source code : \ref HSM_Example_Info
\example HSM_Example
* This example allows to execute some operations with the STM32 HSM. \n
* 1.	Detect plugged-in HSM cards. \n
* 2.	Get the HSM Firmware ID, counter, State, type and version \n
* 3.	Get the HSM license after device connection \n
* \code{.cpp}
**/


#include <stdio.h> 
#include <DisplayManager.h>
#include <CubeProgrammer_API.h>
#include <iostream>


constexpr int HSM_SLOT_INDEX = 1; /* HSM SLOT INDEX to select the plugged-in HSM */
constexpr int STLINK_INDEX = 0; /* STLINK INDEX to select the connected STM32 device */

int HSM_Example(void) {

    logMessage(Title, "\n+++ HSM Get Information +++\n\n");

    const char* fwIdentifier =  getHsmFirmwareID(HSM_SLOT_INDEX);
    if (fwIdentifier == nullptr)
        return -1;

    unsigned long counterValue = getHsmCounter(HSM_SLOT_INDEX);

    const char* hsmState = getHsmState(HSM_SLOT_INDEX);
    if (hsmState == nullptr)
        return -1;

    const char* hsmVersion = getHsmVersion(HSM_SLOT_INDEX);
    if (hsmVersion == nullptr)
        return -1;

    const char* hsmType = getHsmType(HSM_SLOT_INDEX);
    if (hsmType == nullptr)
        return -1;


    logMessage(Title, "\n+++ HSM Generate License +++\n\n");

    /* Connection to target should be established before getting license */
    debugConnectParameters* stLinkList;
    debugConnectParameters debugParameters;
    generalInf* genInfo;

    int getStlinkListNb = getStLinkList(&stLinkList, 0);
    debugParameters = stLinkList[STLINK_INDEX];
    debugParameters.connectionMode = HOTPLUG_MODE;
    debugParameters.shared = 0;

    /* Target connect */
    int connectStlinkFlag = connectStLink(debugParameters);
    if (connectStlinkFlag != CUBEPROGRAMMER_NO_ERROR) {
        logMessage(Error, "Establishing connection with the device failed\n");
        disconnect();
        return -1;
    }

    /* Display device information */
    genInfo = getDeviceGeneralInf();
    logMessage(Normal, "\nDevice name : %s ", genInfo->name);
    logMessage(Normal, "\nDevice type : %s ", genInfo->type);
    logMessage(Normal, "\nDevice CPU  : %s \n", genInfo->cpu);

    /* Get HSM license and save the output file in the location ../test file/licenseFile.bin */
    int ret = getHsmLicense(HSM_SLOT_INDEX, L"../test file/licenseFile.bin");
    if (ret != CUBEPROGRAMMER_NO_ERROR)
        return -1;

    return 0;

}

/** \endcode **/
