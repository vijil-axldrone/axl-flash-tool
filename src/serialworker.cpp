#include "serialworker.h"
#include "packet_serial.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// Thread-local or global static for C callbacks
static QSerialPort *g_serialPort = nullptr;

// Callbacks for packet_serial
static void sp_tx(const uint8_t *data, size_t len) {
    if (g_serialPort && g_serialPort->isOpen()) {
        g_serialPort->write(reinterpret_cast<const char*>(data), len);
        g_serialPort->waitForBytesWritten(200);
    }
}

static int sp_available(void) {
    if (!g_serialPort || !g_serialPort->isOpen()) return 0;
    if (g_serialPort->bytesAvailable() == 0) {
        g_serialPort->waitForReadyRead(5);
    }
    return g_serialPort->bytesAvailable();
}

static uint8_t sp_read_byte(void) {
    uint8_t b = 0;
    if (g_serialPort && g_serialPort->isOpen()) {
        g_serialPort->read(reinterpret_cast<char*>(&b), 1);
    }
    return b;
}

static uint32_t sp_millis(void) {
    return QDateTime::currentMSecsSinceEpoch();
}

// State for receiving packets
static volatile bool rx_data_ready = false;
static QByteArray rx_buffer_qt;

static void on_packet_received(const uint8_t *data, size_t len) {
    rx_buffer_qt = QByteArray(reinterpret_cast<const char*>(data), len);
    rx_data_ready = true;
}

SerialWorker::SerialWorker(const QString &portName, QObject *parent)
    : QThread(parent), m_portName(portName)
{
}

SerialWorker::~SerialWorker()
{
}

void SerialWorker::run()
{
    QSerialPort serial;
    serial.setPortName(m_portName);
    serial.setBaudRate(QSerialPort::Baud9600);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial.open(QIODevice::ReadWrite)) {
        emit errorOccurred("Failed to open serial port: " + m_portName);
        return;
    }

    g_serialPort = &serial;

    reliable_packeter_t pkt;
    uint8_t rx_buf[4096];

    packeter_init(&pkt, 500);
    packeter_set_callbacks(&pkt, sp_tx, sp_available, sp_read_byte, sp_millis);
    packeter_set_receive_buffer(&pkt, rx_buf, sizeof(rx_buf) - 1);
    packeter_on_receive(&pkt, on_packet_received);

    emit logMessage("Starting serial OTA handshake...");

    enum State {
        STATE_CONNECTION_SYNC,
        STATE_GET_IP,
        STATE_ENTER_BOOT_MODE,
        STATE_HALT
    };

    State currentState = STATE_CONNECTION_SYNC;
    bool keepRunning = true;
    QString ipAddress;

    auto sendCommand = [&](const QByteArray &cmd) {
        packet_status_t status = packeter_send(&pkt, reinterpret_cast<const uint8_t*>(cmd.constData()), cmd.length());
        if (status != PACKET_OK) {
            emit logMessage(QString("Packet send failed (status %1) for: %2").arg(status).arg(QString(cmd)));
        }
    };

    while (keepRunning) {
        if (isInterruptionRequested()) {
            break;
        }

        switch (currentState) {
            case STATE_CONNECTION_SYNC:
                sendCommand("{\"type\":\"SYS\",\"cmd\":\"connection_sync\"}");
                break;
            case STATE_GET_IP:
                sendCommand("{\"type\":\"SYS\",\"cmd\":\"ip_address\"}");
                break;
            case STATE_ENTER_BOOT_MODE:
                sendCommand("{\"type\":\"SYS\",\"cmd\":\"enter_boot\"}");
                break;
            case STATE_HALT:
                keepRunning = false;
                break;
        }

        if (!keepRunning) break;

        // Wait for a response, try to parse
        QDateTime waitStart = QDateTime::currentDateTime();
        while (waitStart.msecsTo(QDateTime::currentDateTime()) < 1000) {
            if (isInterruptionRequested()) {
                keepRunning = false;
                break;
            }

            packeter_update(&pkt);
            if (rx_data_ready) {
                rx_data_ready = false;
                QString jsonStr = QString::fromUtf8(rx_buffer_qt);
                emit logMessage("Device: " + jsonStr);

                QJsonDocument doc = QJsonDocument::fromJson(rx_buffer_qt);
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    QString cmd = obj["cmd"].toString();

                    if (cmd == "ack_connection_sync") {
                        currentState = STATE_GET_IP;
                        break;
                    } else if (cmd == "ack_ip_address") {
                        ipAddress = obj["ip_address"].toString();
                        if (ipAddress == "0.0.0.0" || ipAddress.isEmpty()) {
                            emit errorOccurred("No IP Address assigned. Please check device Network.");
                            keepRunning = false;
                        } else {
                            emit logMessage("Found IP: " + ipAddress);
                            emit otaIpAddressFound(ipAddress);
                            // Proceed to enter boot mode (HTTP server starts listening or similar)
                            currentState = STATE_ENTER_BOOT_MODE;
                        }
                        break;
                    } else if (cmd == "ack_enter_boot") {
                        emit enterBootSuccess();
                        currentState = STATE_HALT;
                        break;
                    }
                }
            }
        }
    }

    g_serialPort = nullptr;
    serial.close();
    emit logMessage("Serial handshake finished.");
}
