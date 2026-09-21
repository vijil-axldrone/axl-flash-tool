#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialworker.h"

#include <QSerialPortInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QTextCursor>
#include <QScrollBar>

#include "esp_flasher/EspFlashWorker.h"
#include "stm32_flasher/StmFlashWorker.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    activeProcess(nullptr),
    activeWorker(nullptr),
    activeReply(nullptr),
    activeEspWorker(nullptr)
{
    ui->setupUi(this);

    // Initial state
    ui->confirmButton->setEnabled(false);
    ui->flashingGroup->setEnabled(false);

    connect(ui->selectDirButton, &QPushButton::clicked, this, &MainWindow::selectDirectory);
    connect(ui->confirmButton, &QPushButton::clicked, this, &MainWindow::confirmSetup);
    
    // Changing product invalidates the selection
    connect(ui->productComboBox, &QComboBox::currentTextChanged, this, [this](const QString &) {
        ui->confirmButton->setEnabled(false);
        ui->confirmButton->setText("Confirm (Please Validate First)");
        m_validatedDirPath.clear();
        ui->releaseNotesTextEdit->clear();
        appendLog("Product selection changed. Please select the firmware directory again to validate.");
    });

    connect(ui->clearLogButton, &QPushButton::clicked, ui->logTextEdit, &QTextEdit::clear);
    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(ui->flashEspButton, &QPushButton::clicked, this, &MainWindow::flashEsp);
    connect(ui->flashStmButton, &QPushButton::clicked, this, &MainWindow::flashStm);
    connect(ui->updateOtaButton, &QPushButton::clicked, this, &MainWindow::updateOta);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MainWindow::cancelOperation);

    refreshPorts();
}

MainWindow::~MainWindow()
{
    if (activeEspWorker) {
        activeEspWorker->terminate();
        activeEspWorker->wait();
    }
    if (activeProcess) {
        if (activeProcess->state() == QProcess::Running) {
            activeProcess->kill();
            activeProcess->waitForFinished();
        }
        delete activeProcess;
    }
    delete ui;
}

void MainWindow::appendLog(const QString &message)
{
    ui->logTextEdit->append(message);
}

void MainWindow::selectDirectory()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Firmware Directory");
    if (!dirPath.isEmpty()) {
        validateDirectory(dirPath);
    }
}

void MainWindow::validateDirectory(const QString &dirPath)
{
    QString product = ui->productComboBox->currentText();
    QString espPrefix;
    QString stmFileName;

    if (product == "AXL FLAT") {
        espPrefix = "FLAT";
        stmFileName = "axlflat.bin";
    } else if (product == "AXL BIN") {
        espPrefix = "BIN";
        stmFileName = "axlbin.bin";
    } else if (product == "AXL GATE (SEC)") {
        espPrefix = "GATE";
        stmFileName = "axlgate.bin";
    } else {
        appendLog("Error: Unknown product selected.");
        return;
    }

    appendLog(QString("Validating directory for %1...").arg(product));

    QString changelogPath = QDir(dirPath).filePath("changelog.txt");
    if (!QFile::exists(changelogPath)) {
        appendLog("Validation failed: changelog.txt not found!");
        ui->confirmButton->setEnabled(false);
        return;
    }

    QString espPartitionDir = QDir(dirPath).filePath("esp_partition");
    if (!QDir(espPartitionDir).exists()) {
        appendLog("Validation failed: esp_partition directory not found!");
        ui->confirmButton->setEnabled(false);
        return;
    }

    QString stmApp = QDir(dirPath).filePath(stmFileName);
    QString espApp = QDir(dirPath).filePath(QString("%1-NW-Manager.ino.bin").arg(espPrefix));
    QString espBootloader = QDir(espPartitionDir).filePath(QString("%1-NW-Manager.ino.bootloader.bin").arg(espPrefix));
    QString espPartitions = QDir(espPartitionDir).filePath(QString("%1-NW-Manager.ino.partitions.bin").arg(espPrefix));
    QString espBootApp = QDir(espPartitionDir).filePath("boot_app0.bin");

    bool allValid = true;
    if (!QFile::exists(stmApp)) { appendLog("Missing: " + stmApp); allValid = false; }
    if (!QFile::exists(espApp)) { appendLog("Missing: " + espApp); allValid = false; }
    if (!QFile::exists(espBootloader)) { appendLog("Missing: " + espBootloader); allValid = false; }
    if (!QFile::exists(espPartitions)) { appendLog("Missing: " + espPartitions); allValid = false; }
    if (!QFile::exists(espBootApp)) { appendLog("Missing: " + espBootApp); allValid = false; }

    if (!allValid) {
        appendLog("Validation failed due to missing files.");
        ui->confirmButton->setEnabled(false);
        return;
    }

    // Save valid paths
    m_validatedDirPath = dirPath;
    m_stmAppPath = stmApp;
    m_espAppPath = espApp;
    m_espBootloaderPath = espBootloader;
    m_espPartitionsPath = espPartitions;
    m_espBootAppPath = espBootApp;

    appendLog("Validation successful! All files present.");

    // Print changelog to release notes UI
    QFile changelogFile(changelogPath);
    if (changelogFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&changelogFile);
        QString content = in.readAll();
        ui->releaseNotesTextEdit->setPlainText(content);
        changelogFile.close();
    } else {
        ui->releaseNotesTextEdit->setPlainText("Could not open changelog.txt.");
    }

    ui->confirmButton->setText("Confirm");
    ui->confirmButton->setEnabled(true);
}

void MainWindow::confirmSetup()
{
    if (m_validatedDirPath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please validate a directory first.");
        return;
    }

    // Disable setup section, enable flashing section
    ui->setupGroup->setEnabled(false);
    ui->flashingGroup->setEnabled(true);
    appendLog("Setup confirmed. You can now flash the device.");
}

void MainWindow::refreshPorts()
{
    ui->portComboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        ui->portComboBox->addItem(info.portName() + " - " + info.description(), info.systemLocation());
    }
    if (ports.isEmpty()) {
        appendLog("No serial ports found.");
    }
}

QString MainWindow::getSelectedPort()
{
    return ui->portComboBox->currentData().toString();
}

void MainWindow::flashEsp()
{
    if (activeEspWorker || activeProcess || activeWorker || activeReply) {
        QMessageBox::warning(this, "Busy", "An operation is already running.");
        return;
    }

    QString port = getSelectedPort();
    if (port.isEmpty()) {
        QMessageBox::warning(this, "Error", "No port selected!");
        return;
    }

    activeEspWorker = new EspFlashWorker(port, m_espBootloaderPath, m_espPartitionsPath, m_espBootAppPath, m_espAppPath, this);

    connect(activeEspWorker, &EspFlashWorker::logMessage, this, &MainWindow::appendLog);
    connect(activeEspWorker, &EspFlashWorker::progress, this, [this](int value, int maximum) {
        ui->progressBar->setMaximum(maximum);
        ui->progressBar->setValue(value);
    });
    connect(activeEspWorker, &EspFlashWorker::finishedWithSuccess, this, [this]() {
        appendLog("ESP32 Flashing successful!");
        QMessageBox::information(this, "Success", "ESP32 Flashing completed successfully!");
        activeEspWorker->deleteLater();
        activeEspWorker = nullptr;
    });
    connect(activeEspWorker, &EspFlashWorker::finishedWithError, this, [this](const QString &err) {
        appendLog(err);
        QMessageBox::critical(this, "Error", "ESP32 Flashing failed:\n" + err);
        activeEspWorker->deleteLater();
        activeEspWorker = nullptr;
    });

    ui->progressBar->setValue(0);
    activeEspWorker->start();
}

void MainWindow::flashStm()
{
    if (activeEspWorker || activeProcess || activeWorker || activeReply) {
        QMessageBox::warning(this, "Busy", "An operation is already running.");
        return;
    }

    activeProcess = new QProcess(this);
    connect(activeProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::handleProcessOutput);
    connect(activeProcess, &QProcess::readyReadStandardError, this, &MainWindow::handleProcessError);
    connect(activeProcess, &QProcess::finished, this, &MainWindow::handleProcessFinished);

    QString portArg = ui->radioDfu->isChecked() ? "USB1" : "SWD";
    QStringList args;
    if(ui->radioDfu->isChecked())
        args << "-c" << "port=USB1" << "mode=UR" << "-w" << m_stmAppPath << "0x08000000" << "-v";
    else
        args << "-c" << "port=SWD" << "freq=1000" << "mode=UR" << "-w" << m_stmAppPath << "0x08000000" << "-v";

    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat("Connecting... %p%");
    appendLog(QString("Starting STM32_Programmer_CLI (Mode: %1)...").arg(portArg));
    activeProcess->start("STM32_Programmer_CLI", args);
}

void MainWindow::handleProcessOutput()
{
    if (activeProcess) {
        QString out = QString::fromUtf8(activeProcess->readAllStandardOutput());
        out.remove(QRegularExpression("\\x1B\\[[0-9;]*[a-zA-Z]"));
        
        if (out.contains("Download in Progress", Qt::CaseInsensitive) || out.contains("Downloading", Qt::CaseInsensitive)) {
            ui->progressBar->setFormat("Downloading... %p%");
        } else if (out.contains("Verifying", Qt::CaseInsensitive) || out.contains("Verification", Qt::CaseInsensitive)) {
            ui->progressBar->setFormat("Verifying... %p%");
            ui->progressBar->setValue(0);
        }

        QRegularExpression regex("(\\d+)\\s*%");
        QRegularExpressionMatchIterator it = regex.globalMatch(out);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int percent = match.captured(1).toInt();
            ui->progressBar->setValue(percent);
        }

        ui->logTextEdit->moveCursor(QTextCursor::End);
        ui->logTextEdit->insertPlainText(out);
        ui->logTextEdit->ensureCursorVisible();
    }
}

void MainWindow::handleProcessError()
{
    if (activeProcess) {
        QString err = QString::fromUtf8(activeProcess->readAllStandardError());
        err.remove(QRegularExpression("\\x1B\\[[0-9;]*[a-zA-Z]"));
        ui->logTextEdit->moveCursor(QTextCursor::End);
        ui->logTextEdit->insertPlainText(err);
        ui->logTextEdit->ensureCursorVisible();
    }
}

void MainWindow::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit) {
        appendLog("Process crashed.");
        QMessageBox::critical(this, "Error", "STM32 Flashing process crashed.");
    } else {
        appendLog(QString("Process finished with code %1").arg(exitCode));
        if (exitCode == 0) {
            QMessageBox::information(this, "Success", "STM32 Flashing completed successfully!");
        } else {
            QMessageBox::critical(this, "Error", QString("STM32 Flashing failed (Code %1). Check logs for details.").arg(exitCode));
        }
    }
    ui->progressBar->setFormat("%p%"); // Reset format
    activeProcess->deleteLater();
    activeProcess = nullptr;
}

// End of flash functions

void MainWindow::updateOta()
{
    QString port = getSelectedPort();
    if (port.isEmpty()) {
        QMessageBox::warning(this, "Error", "No port selected!");
        return;
    }

    appendLog("Starting OTA procedure on port " + port);
    startOtaSequence(port);
}

void MainWindow::startOtaSequence(const QString &portName)
{
    if (activeWorker || activeEspWorker || activeProcess || activeReply) {
        QMessageBox::warning(this, "Busy", "An operation is already running.");
        return;
    }

    activeWorker = new SerialWorker(portName, this);
    
    connect(activeWorker, &SerialWorker::logMessage, this, &MainWindow::appendLog);
    connect(activeWorker, &SerialWorker::errorOccurred, this, [this](const QString &err){
        QMessageBox::critical(this, "OTA Error", err);
    });
    
    QString *ipAddress = new QString();
    connect(activeWorker, &SerialWorker::otaIpAddressFound, this, [ipAddress](const QString &ip){
        *ipAddress = ip;
    });

    connect(activeWorker, &QThread::finished, this, [this, ipAddress]() {
        activeWorker->deleteLater();
        activeWorker = nullptr;

        if (!ipAddress->isEmpty()) {
            appendLog("Ready to upload OTA to " + *ipAddress);

            QFile *file = new QFile(m_espAppPath);
            if (!file->open(QIODevice::ReadOnly)) {
                appendLog("Failed to open firmware file: " + m_espAppPath);
                delete file;
                return;
            }

            QUrl url(QString("http://%1/update").arg(*ipAddress));
            QNetworkRequest request(url);
            request.setRawHeader("X-Password", "axl@#$admin");

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"update\"; filename=\"firmware.bin\""));
            filePart.setBodyDevice(file);
            file->setParent(multiPart); // we cannot delete file now, let multipart own it
            multiPart->append(filePart);

            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            activeReply = manager->post(request, multiPart);
            multiPart->setParent(activeReply); // delete multipart with reply

            connect(activeReply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesSent, qint64 bytesTotal){
                if (bytesTotal > 0) {
                    ui->progressBar->setMaximum(bytesTotal);
                    ui->progressBar->setValue(bytesSent);
                }
            });

            connect(activeReply, &QNetworkReply::finished, this, [this, manager](){
                bool success = (activeReply->error() == QNetworkReply::NoError);
                if (success) {
                    appendLog("OTA Upload Successful!");
                    QMessageBox::information(this, "Success", "ESP32 OTA completed successfully! Automatically starting STM32 DFU flash...");
                } else {
                    appendLog("OTA Upload failed: " + activeReply->errorString());
                    QMessageBox::critical(this, "Error", "ESP32 OTA failed:\n" + activeReply->errorString());
                }
                
                activeReply->deleteLater();
                activeReply = nullptr;
                manager->deleteLater();

                if (success) {
                    ui->radioDfu->setChecked(true); // Ensure DFU mode
                    flashStm(); // Start STM32 flash
                }
            });
        }
        delete ipAddress;
    });

    activeWorker->start();
}

void MainWindow::cancelOperation()
{
    bool cancelled = false;
    if (activeEspWorker) {
        activeEspWorker->terminate(); // Brute force stop
        activeEspWorker->wait();
        activeEspWorker->deleteLater();
        activeEspWorker = nullptr;
        cancelled = true;
    }
    if (activeProcess) {
        activeProcess->kill();
        cancelled = true;
    }
    if (activeWorker) {
        activeWorker->requestInterruption();
        cancelled = true;
    }
    if (activeReply) {
        activeReply->abort();
        cancelled = true;
    }

    if (cancelled) {
        appendLog("\n[Operation cancelled by user]");
        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("%p%");
    }
}
