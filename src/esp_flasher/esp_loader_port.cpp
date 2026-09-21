#include <QSerialPort>
#include <QThread>
#include <QElapsedTimer>
#include <QDebug>
#include "EspFlashWorker.h"

extern "C" {
#include "esp_loader.h"
#include "esp_loader_io.h"
}

typedef struct {
    esp_loader_port_t base;
    QSerialPort *serial;
    EspFlashWorker *worker;
    QElapsedTimer timer;
    uint32_t timeout_ms;
} qt_port_t;

static qt_port_t g_qt_port;

static esp_loader_error_t qt_init(esp_loader_port_t *port) {
    return ESP_LOADER_SUCCESS;
}

static void qt_deinit(esp_loader_port_t *port) {
}

static void qt_enter_bootloader(esp_loader_port_t *port) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    p->serial->setDataTerminalReady(false); // DTR = 1
    p->serial->setRequestToSend(true);      // RTS = 0
    QThread::msleep(50);
    p->serial->setDataTerminalReady(true);  // DTR = 0
    p->serial->setRequestToSend(false);     // RTS = 1
    QThread::msleep(50);
}

static void qt_reset_target(esp_loader_port_t *port) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    p->serial->setDataTerminalReady(false); // DTR = 1
    p->serial->setRequestToSend(false);     // RTS = 1
    QThread::msleep(50);
}

static void qt_start_timer(esp_loader_port_t *port, uint32_t ms) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    p->timeout_ms = ms;
    p->timer.start();
}

static uint32_t qt_remaining_time(esp_loader_port_t *port) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    qint64 elapsed = p->timer.elapsed();
    if (elapsed >= p->timeout_ms) return 0;
    return p->timeout_ms - elapsed;
}

static void qt_delay_ms(esp_loader_port_t *port, uint32_t ms) {
    QThread::msleep(ms);
}

static void qt_log(esp_loader_port_t *port, esp_loader_log_level_t level,
                   const char *fmt, va_list args) {
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    qt_port_t *p = container_of(port, qt_port_t, base);
    if (p->worker) emit p->worker->logMessage(QString::fromUtf8(buf).trimmed());
}

static esp_loader_error_t qt_change_transmission_rate(esp_loader_port_t *port, uint32_t rate) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    p->serial->setBaudRate(rate);
    return ESP_LOADER_SUCCESS;
}

static esp_loader_error_t qt_write(esp_loader_port_t *port, const uint8_t *data, uint16_t size, uint32_t timeout) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    p->serial->write((const char*)data, size);
    if (!p->serial->waitForBytesWritten(timeout)) {
        return ESP_LOADER_ERROR_TIMEOUT;
    }
    return ESP_LOADER_SUCCESS;
}

static esp_loader_error_t qt_read(esp_loader_port_t *port, uint8_t *data, uint16_t size, uint32_t timeout) {
    qt_port_t *p = container_of(port, qt_port_t, base);
    uint16_t read_bytes = 0;
    QElapsedTimer t;
    t.start();
    
    while (read_bytes < size) {
        qint64 remaining = timeout - t.elapsed();
        if (remaining <= 0) return ESP_LOADER_ERROR_TIMEOUT;
        
        if (p->serial->bytesAvailable() == 0) {
            p->serial->waitForReadyRead(remaining);
        }
        qint64 r = p->serial->read((char*)data + read_bytes, size - read_bytes);
        if (r > 0) read_bytes += r;
    }
    return ESP_LOADER_SUCCESS;
}

static const esp_loader_port_ops_t qt_ops = {
    .init = qt_init,
    .deinit = qt_deinit,
    .enter_bootloader = qt_enter_bootloader,
    .reset_target = qt_reset_target,
    .start_timer = qt_start_timer,
    .remaining_time = qt_remaining_time,
    .delay_ms = qt_delay_ms,
    .log = qt_log,
    .log_hex = NULL,
    .change_transmission_rate = qt_change_transmission_rate,
    .write = qt_write,
    .read = qt_read,
    .spi_set_cs = NULL,
    .sdio_write = NULL,
    .sdio_read = NULL,
    .sdio_card_init = NULL,
};

esp_loader_port_t* get_esp_loader_port(QSerialPort *serial, EspFlashWorker *worker) {
    g_qt_port.base.ops = &qt_ops;
    g_qt_port.serial = serial;
    g_qt_port.worker = worker;
    return &g_qt_port.base;
}
