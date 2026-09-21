#ifndef STMFLASHWORKER_H
#define STMFLASHWORKER_H

#include <QThread>
#include <QString>

class StmFlashWorker : public QThread
{
    Q_OBJECT
public:
    explicit StmFlashWorker(const QString &appPath, bool useDfu, QObject *parent = nullptr);
    ~StmFlashWorker() override;

    // Static callbacks for STM32CubeProgrammer API
    static void staticLogMessage(int msgType, const wchar_t* str);
    static void staticLoadBar(int x, int n);

signals:
    void logMessage(const QString &message);
    void progress(int value, int maximum);
    void finishedWithError(const QString &error);
    void finishedWithSuccess();

protected:
    void run() override;

private:
    QString m_appPath;
    bool m_useDfu;
};

#endif // STMFLASHWORKER_H
