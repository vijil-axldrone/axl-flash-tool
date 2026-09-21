#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QProcess>

class SerialWorker;
class EspFlashWorker;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectDirectory();
    void confirmSetup();
    void refreshPorts();
    void flashEsp();
    void flashStm();
    void updateOta();
    void cancelOperation();
    void appendLog(const QString &message);

private slots:
    void handleProcessOutput();
    void handleProcessError();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::MainWindow *ui;
    QProcess *activeProcess;
    SerialWorker *activeWorker;
    QNetworkReply *activeReply;
    EspFlashWorker *activeEspWorker;
    
    QString m_validatedDirPath;
    QString m_stmAppPath;
    QString m_espAppPath;
    QString m_espBootloaderPath;
    QString m_espPartitionsPath;
    QString m_espBootAppPath;

    QString getSelectedPort();
    void validateDirectory(const QString &dirPath);
    
    // OTA related
    void startOtaSequence(const QString &portName);
};

#endif // MAINWINDOW_H
