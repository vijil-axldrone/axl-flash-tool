#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QThread>
#include <QSerialPort>
#include <QString>

class SerialWorker : public QThread
{
    Q_OBJECT
public:
    explicit SerialWorker(const QString &portName, QObject *parent = nullptr);
    ~SerialWorker();

    void run() override;

signals:
    void logMessage(const QString &msg);
    void otaIpAddressFound(const QString &ip);
    void errorOccurred(const QString &errorMsg);
    void enterBootSuccess();

private:
    QString m_portName;
};

#endif // SERIALWORKER_H
