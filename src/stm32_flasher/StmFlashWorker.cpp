#include "StmFlashWorker.h"
#include "../../stm32_api/include/CubeProgrammer_API.h"
#include <QDebug>
#include <QMessageBox>

static StmFlashWorker* g_activeStmWorker = nullptr;

StmFlashWorker::StmFlashWorker(const QString &appPath, bool useDfu, QObject *parent)
    : QThread(parent), m_appPath(appPath), m_useDfu(useDfu)
{
}

StmFlashWorker::~StmFlashWorker()
{
}

void StmFlashWorker::staticLogMessage(int msgType, const wchar_t* str)
{
    if (g_activeStmWorker) {
        QString msg = QString::fromWCharArray(str);
        emit g_activeStmWorker->logMessage(msg.trimmed());
    }
}

void StmFlashWorker::staticLoadBar(int x, int n)
{
    if (g_activeStmWorker) {
        emit g_activeStmWorker->progress(x, n);
    }
}

void StmFlashWorker::run()
{
    g_activeStmWorker = this;

    displayCallBacks cbs;
    cbs.logMessage = staticLogMessage;
    cbs.loadBar = staticLoadBar;
    setDisplayCallbacks(cbs);

    int status = CUBEPROGRAMMER_ERROR_NOT_CONNECTED;

    if (m_useDfu) {
        emit logMessage("Connecting to STM32 via USB DFU...");
        char usbPort[] = "USB1";
        status = connectDfuBootloader(usbPort);
    } else {
        emit logMessage("Connecting to STM32 via SWD (ST-LINK)...");
        debugConnectParameters stlinkParams;
        memset(&stlinkParams, 0, sizeof(stlinkParams));
        stlinkParams.dbgPort = SWD;
        stlinkParams.connectionMode = NORMAL_MODE;
        stlinkParams.shared = 0;
        status = connectStLink(stlinkParams);
    }

    if (status != 0) {
        emit finishedWithError("Failed to connect to STM32 device. Error code: " + QString::number(status));
        g_activeStmWorker = nullptr;
        return;
    }

    emit logMessage("Connected. Flashing firmware...");
    
    // address = 0x08000000, skipErase = 0, verify = 1
    status = downloadFile(m_appPath.toStdWString().c_str(), 0x08000000, 0, 1, L"");

    disconnect();

    if (status == 0) {
        emit finishedWithSuccess();
    } else {
        emit finishedWithError("Flash operation failed. Error code: " + QString::number(status));
    }

    g_activeStmWorker = nullptr;
}
