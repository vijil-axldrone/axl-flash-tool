#include "EspFlashWorker.h"
#include <QFile>
#include <QSerialPort>
#include <QThread>

extern "C" {
#include "esp_loader.h"
#include "esp_loader_io.h"
}

// Defined in esp_loader_port.cpp
extern esp_loader_port_t* get_esp_loader_port(QSerialPort *serial, EspFlashWorker *worker);

EspFlashWorker::EspFlashWorker(const QString &portName, const QString &bootloader, const QString &partitions, const QString &bootApp, const QString &app, QObject *parent)
    : QThread(parent), m_portName(portName), m_bootloader(bootloader), m_partitions(partitions), m_bootApp(bootApp), m_app(app)
{
}

EspFlashWorker::~EspFlashWorker()
{
}

static bool flash_binary(EspFlashWorker *worker, esp_loader_t *loader, const QString& filePath, uint32_t address) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (worker) emit worker->logMessage("Failed to open " + filePath);
        return false;
    }

    QByteArray data = file.readAll();
    uint32_t size = data.size();

    if (worker) emit worker->logMessage(QString("Erasing flash (Address: 0x%1, Size: %2)...").arg(address, 0, 16).arg(size));

    esp_loader_flash_cfg_t flash_cfg = {};
    flash_cfg.offset = address;
    flash_cfg.image_size = size;
    flash_cfg.block_size = size;
    flash_cfg.skip_verify = false;

    esp_loader_error_t err = esp_loader_flash_start(loader, &flash_cfg);
    if (err != ESP_LOADER_SUCCESS) {
        if (worker) emit worker->logMessage("Failed to start flash (Erase)");
        return false;
    }

    if (worker) emit worker->logMessage("Flashing...");

    uint32_t written = 0;
    const uint32_t chunk_size = 4096;
    while (written < size) {
        uint32_t to_write = std::min(chunk_size, size - written);
        err = esp_loader_flash_write(loader, &flash_cfg, (uint8_t*)data.constData() + written, to_write);
        if (err != ESP_LOADER_SUCCESS) {
            if (worker) emit worker->logMessage("Failed to write chunk.");
            return false;
        }
        written += to_write;
        if (worker) emit worker->progress(written, size);
    }

    err = esp_loader_flash_finish(loader, &flash_cfg);
    if (err != ESP_LOADER_SUCCESS) {
        if (worker) emit worker->logMessage("Failed to finish flash operation (MD5 check failed).");
        return false;
    }

    return true;
}

void EspFlashWorker::run()
{
    QSerialPort serial;
    serial.setPortName(m_portName);
    serial.setBaudRate(115200);
    if (!serial.open(QIODevice::ReadWrite)) {
        emit finishedWithError("Failed to open serial port: " + serial.errorString());
        return;
    }

    emit logMessage("Connecting to ESP32 on " + m_portName + "...");

    esp_loader_t loader;
    esp_loader_port_t* port = get_esp_loader_port(&serial, this);
    esp_loader_error_t err = esp_loader_init_serial(&loader, port);
    if (err != ESP_LOADER_SUCCESS) {
        emit finishedWithError("Failed to init serial loader.");
        return;
    }

    esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
    err = esp_loader_connect_with_stub(&loader, &connect_config);
    if (err != ESP_LOADER_SUCCESS) {
        emit finishedWithError("Failed to connect to ESP32.");
        return;
    }

    emit logMessage("Changing baud rate to 921600...");
    err = esp_loader_change_transmission_rate(&loader, 921600);
    if (err == ESP_LOADER_SUCCESS) {
        emit logMessage("Baud rate changed.");
    } else {
        emit logMessage("Failed to change baud rate, continuing at 115200.");
    }

    emit logMessage("Connected! Flashing binaries...");

    // Flash bootloader
    if (!flash_binary(this, &loader, m_bootloader, 0x1000)) goto error;
    // Flash partitions
    if (!flash_binary(this, &loader, m_partitions, 0x8000)) goto error;
    // Flash boot app
    if (!flash_binary(this, &loader, m_bootApp, 0xe000)) goto error;
    // Flash app
    if (!flash_binary(this, &loader, m_app, 0x10000)) goto error;

    esp_loader_reset_target(&loader); // Reboot
    
    serial.close();
    emit finishedWithSuccess();
    return;

error:
    serial.close();
    emit finishedWithError("Flashing failed.");
}
