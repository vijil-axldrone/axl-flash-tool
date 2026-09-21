#ifndef ESPFLASHWORKER_H
#define ESPFLASHWORKER_H

#include <QThread>
#include <QString>

class EspFlashWorker : public QThread
{
    Q_OBJECT
public:
    explicit EspFlashWorker(const QString &portName, const QString &bootloader, const QString &partitions, const QString &bootApp, const QString &app, QObject *parent = nullptr);
    ~EspFlashWorker() override;

signals:
    void logMessage(const QString &message);
    void progress(int value, int maximum);
    void finishedWithError(const QString &error);
    void finishedWithSuccess();

protected:
    void run() override;

private:
    QString m_portName;
    QString m_bootloader;
    QString m_partitions;
    QString m_bootApp;
    QString m_app;
};

#endif // ESPFLASHWORKER_H
