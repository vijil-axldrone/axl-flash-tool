/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *setupGroup;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QComboBox *productComboBox;
    QPushButton *selectDirButton;
    QPushButton *confirmButton;
    QGroupBox *flashingGroup;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *portComboBox;
    QPushButton *refreshButton;
    QHBoxLayout *horizontalLayout_stm;
    QLabel *label_3;
    QRadioButton *radioDfu;
    QRadioButton *radioSwd;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *flashEspButton;
    QPushButton *flashStmButton;
    QPushButton *updateOtaButton;
    QPushButton *cancelButton;
    QProgressBar *progressBar;
    QHBoxLayout *bottomLayout;
    QVBoxLayout *logLayout;
    QPushButton *clearLogButton;
    QTextEdit *logTextEdit;
    QVBoxLayout *releaseNotesLayout;
    QLabel *releaseNotesLabel;
    QTextEdit *releaseNotesTextEdit;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        setupGroup = new QGroupBox(centralwidget);
        setupGroup->setObjectName("setupGroup");
        verticalLayout_2 = new QVBoxLayout(setupGroup);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label_2 = new QLabel(setupGroup);
        label_2->setObjectName("label_2");

        horizontalLayout_3->addWidget(label_2);

        productComboBox = new QComboBox(setupGroup);
        productComboBox->addItem(QString());
        productComboBox->addItem(QString());
        productComboBox->addItem(QString());
        productComboBox->setObjectName("productComboBox");

        horizontalLayout_3->addWidget(productComboBox);

        selectDirButton = new QPushButton(setupGroup);
        selectDirButton->setObjectName("selectDirButton");

        horizontalLayout_3->addWidget(selectDirButton);


        verticalLayout_2->addLayout(horizontalLayout_3);

        confirmButton = new QPushButton(setupGroup);
        confirmButton->setObjectName("confirmButton");
        confirmButton->setEnabled(false);

        verticalLayout_2->addWidget(confirmButton);


        verticalLayout->addWidget(setupGroup);

        flashingGroup = new QGroupBox(centralwidget);
        flashingGroup->setObjectName("flashingGroup");
        flashingGroup->setEnabled(false);
        verticalLayout_3 = new QVBoxLayout(flashingGroup);
        verticalLayout_3->setObjectName("verticalLayout_3");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(flashingGroup);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        portComboBox = new QComboBox(flashingGroup);
        portComboBox->setObjectName("portComboBox");
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(portComboBox->sizePolicy().hasHeightForWidth());
        portComboBox->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(portComboBox);

        refreshButton = new QPushButton(flashingGroup);
        refreshButton->setObjectName("refreshButton");

        horizontalLayout->addWidget(refreshButton);


        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayout_stm = new QHBoxLayout();
        horizontalLayout_stm->setObjectName("horizontalLayout_stm");
        label_3 = new QLabel(flashingGroup);
        label_3->setObjectName("label_3");

        horizontalLayout_stm->addWidget(label_3);

        radioDfu = new QRadioButton(flashingGroup);
        radioDfu->setObjectName("radioDfu");
        radioDfu->setChecked(true);

        horizontalLayout_stm->addWidget(radioDfu);

        radioSwd = new QRadioButton(flashingGroup);
        radioSwd->setObjectName("radioSwd");

        horizontalLayout_stm->addWidget(radioSwd);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_stm->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_stm);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        flashEspButton = new QPushButton(flashingGroup);
        flashEspButton->setObjectName("flashEspButton");

        horizontalLayout_2->addWidget(flashEspButton);

        flashStmButton = new QPushButton(flashingGroup);
        flashStmButton->setObjectName("flashStmButton");

        horizontalLayout_2->addWidget(flashStmButton);

        updateOtaButton = new QPushButton(flashingGroup);
        updateOtaButton->setObjectName("updateOtaButton");

        horizontalLayout_2->addWidget(updateOtaButton);

        cancelButton = new QPushButton(flashingGroup);
        cancelButton->setObjectName("cancelButton");

        horizontalLayout_2->addWidget(cancelButton);


        verticalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout->addWidget(flashingGroup);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName("progressBar");
        progressBar->setValue(0);

        verticalLayout->addWidget(progressBar);

        bottomLayout = new QHBoxLayout();
        bottomLayout->setObjectName("bottomLayout");
        logLayout = new QVBoxLayout();
        logLayout->setObjectName("logLayout");
        clearLogButton = new QPushButton(centralwidget);
        clearLogButton->setObjectName("clearLogButton");

        logLayout->addWidget(clearLogButton);

        logTextEdit = new QTextEdit(centralwidget);
        logTextEdit->setObjectName("logTextEdit");
        logTextEdit->setReadOnly(true);

        logLayout->addWidget(logTextEdit);


        bottomLayout->addLayout(logLayout);

        releaseNotesLayout = new QVBoxLayout();
        releaseNotesLayout->setObjectName("releaseNotesLayout");
        releaseNotesLabel = new QLabel(centralwidget);
        releaseNotesLabel->setObjectName("releaseNotesLabel");

        releaseNotesLayout->addWidget(releaseNotesLabel);

        releaseNotesTextEdit = new QTextEdit(centralwidget);
        releaseNotesTextEdit->setObjectName("releaseNotesTextEdit");
        releaseNotesTextEdit->setReadOnly(true);

        releaseNotesLayout->addWidget(releaseNotesTextEdit);


        bottomLayout->addLayout(releaseNotesLayout);


        verticalLayout->addLayout(bottomLayout);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "AxlFlash UI", nullptr));
        setupGroup->setTitle(QCoreApplication::translate("MainWindow", "Setup & Validation", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Product:", nullptr));
        productComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "AXL FLAT", nullptr));
        productComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "AXL BIN", nullptr));
        productComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "AXL GATE (SEC)", nullptr));

        selectDirButton->setText(QCoreApplication::translate("MainWindow", "Select Firmware Directory", nullptr));
        confirmButton->setText(QCoreApplication::translate("MainWindow", "Confirm (Please Validate First)", nullptr));
        flashingGroup->setTitle(QCoreApplication::translate("MainWindow", "Flashing Actions", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Select Port:", nullptr));
        refreshButton->setText(QCoreApplication::translate("MainWindow", "Refresh Ports", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "STM Flash Mode:", nullptr));
        radioDfu->setText(QCoreApplication::translate("MainWindow", "DFU", nullptr));
        radioSwd->setText(QCoreApplication::translate("MainWindow", "SWD", nullptr));
        flashEspButton->setText(QCoreApplication::translate("MainWindow", "Flash ESP", nullptr));
        flashStmButton->setText(QCoreApplication::translate("MainWindow", "Flash STM", nullptr));
        updateOtaButton->setText(QCoreApplication::translate("MainWindow", "Update OTA", nullptr));
        cancelButton->setText(QCoreApplication::translate("MainWindow", "Cancel Operation", nullptr));
        clearLogButton->setText(QCoreApplication::translate("MainWindow", "Clear Logs", nullptr));
        releaseNotesLabel->setText(QCoreApplication::translate("MainWindow", "Release Notes:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
