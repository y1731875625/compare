/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
// #include "RecordWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *vc0Input;
    QLineEdit *vc0Input_2;
    QPushButton *btnOrig;
    QPushButton *btnComp;
    QPushButton *btnCompare;
    QSplitter *vSplitter;
    QSplitter *hSplitter;
    RecordWidget *leftWidget;
    RecordWidget *rightWidget;
    QTabWidget *tabWidget;
    QWidget *diffTab;
    QVBoxLayout *diffLayout;
    QTableWidget *diffTable;
    QWidget *diffPaginationWidget;
    QHBoxLayout *diffPageLayout;
    QPushButton *diffFirstBtn;
    QPushButton *diffPrevBtn;
    QPushButton *diffNextBtn;
    QPushButton *diffLastBtn;
    QLabel *diffJumpLabel;
    QLineEdit *diffPageInput;
    QLabel *diffPageLabel;
    QSpacerItem *diffSpacer;
    QWidget *errorTab;
    QVBoxLayout *errorLayout;
    QTableWidget *errorTable;
    QWidget *errorPaginationWidget;
    QHBoxLayout *errorPageLayout;
    QPushButton *errorFirstBtn;
    QPushButton *errorPrevBtn;
    QPushButton *errorNextBtn;
    QPushButton *errorLastBtn;
    QLabel *errorJumpLabel;
    QLineEdit *errorPageInput;
    QLabel *errorPageLabel;
    QSpacerItem *errorSpacer;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        vc0Input = new QLineEdit(centralwidget);
        vc0Input->setObjectName("vc0Input");

        horizontalLayout->addWidget(vc0Input);

        vc0Input_2 = new QLineEdit(centralwidget);
        vc0Input_2->setObjectName("vc0Input_2");

        horizontalLayout->addWidget(vc0Input_2);

        btnOrig = new QPushButton(centralwidget);
        btnOrig->setObjectName("btnOrig");

        horizontalLayout->addWidget(btnOrig);

        btnComp = new QPushButton(centralwidget);
        btnComp->setObjectName("btnComp");

        horizontalLayout->addWidget(btnComp);

        btnCompare = new QPushButton(centralwidget);
        btnCompare->setObjectName("btnCompare");
        btnCompare->setEnabled(false);

        horizontalLayout->addWidget(btnCompare);


        verticalLayout->addLayout(horizontalLayout);

        vSplitter = new QSplitter(centralwidget);
        vSplitter->setObjectName("vSplitter");
        vSplitter->setOrientation(Qt::Orientation::Vertical);
        vSplitter->setChildrenCollapsible(false);
        hSplitter = new QSplitter(vSplitter);
        hSplitter->setObjectName("hSplitter");
        hSplitter->setOrientation(Qt::Orientation::Horizontal);
        leftWidget = new RecordWidget(hSplitter);
        leftWidget->setObjectName("leftWidget");
        hSplitter->addWidget(leftWidget);
        rightWidget = new RecordWidget(hSplitter);
        rightWidget->setObjectName("rightWidget");
        hSplitter->addWidget(rightWidget);
        vSplitter->addWidget(hSplitter);
        tabWidget = new QTabWidget(vSplitter);
        tabWidget->setObjectName("tabWidget");
        diffTab = new QWidget();
        diffTab->setObjectName("diffTab");
        diffLayout = new QVBoxLayout(diffTab);
        diffLayout->setObjectName("diffLayout");
        diffTable = new QTableWidget(diffTab);
        if (diffTable->columnCount() < 4)
            diffTable->setColumnCount(4);
        diffTable->setObjectName("diffTable");
        diffTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        diffTable->setColumnCount(4);

        diffLayout->addWidget(diffTable);

        diffPaginationWidget = new QWidget(diffTab);
        diffPaginationWidget->setObjectName("diffPaginationWidget");
        diffPageLayout = new QHBoxLayout(diffPaginationWidget);
        diffPageLayout->setObjectName("diffPageLayout");
        diffFirstBtn = new QPushButton(diffPaginationWidget);
        diffFirstBtn->setObjectName("diffFirstBtn");

        diffPageLayout->addWidget(diffFirstBtn);

        diffPrevBtn = new QPushButton(diffPaginationWidget);
        diffPrevBtn->setObjectName("diffPrevBtn");

        diffPageLayout->addWidget(diffPrevBtn);

        diffNextBtn = new QPushButton(diffPaginationWidget);
        diffNextBtn->setObjectName("diffNextBtn");

        diffPageLayout->addWidget(diffNextBtn);

        diffLastBtn = new QPushButton(diffPaginationWidget);
        diffLastBtn->setObjectName("diffLastBtn");

        diffPageLayout->addWidget(diffLastBtn);

        diffJumpLabel = new QLabel(diffPaginationWidget);
        diffJumpLabel->setObjectName("diffJumpLabel");

        diffPageLayout->addWidget(diffJumpLabel);

        diffPageInput = new QLineEdit(diffPaginationWidget);
        diffPageInput->setObjectName("diffPageInput");
        diffPageInput->setProperty("fixedWidth", QVariant(50));

        diffPageLayout->addWidget(diffPageInput);

        diffPageLabel = new QLabel(diffPaginationWidget);
        diffPageLabel->setObjectName("diffPageLabel");

        diffPageLayout->addWidget(diffPageLabel);

        diffSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        diffPageLayout->addItem(diffSpacer);


        diffLayout->addWidget(diffPaginationWidget);

        tabWidget->addTab(diffTab, QString());
        errorTab = new QWidget();
        errorTab->setObjectName("errorTab");
        errorLayout = new QVBoxLayout(errorTab);
        errorLayout->setObjectName("errorLayout");
        errorTable = new QTableWidget(errorTab);
        if (errorTable->columnCount() < 5)
            errorTable->setColumnCount(5);
        errorTable->setObjectName("errorTable");
        errorTable->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        errorTable->setColumnCount(5);

        errorLayout->addWidget(errorTable);

        errorPaginationWidget = new QWidget(errorTab);
        errorPaginationWidget->setObjectName("errorPaginationWidget");
        errorPageLayout = new QHBoxLayout(errorPaginationWidget);
        errorPageLayout->setObjectName("errorPageLayout");
        errorFirstBtn = new QPushButton(errorPaginationWidget);
        errorFirstBtn->setObjectName("errorFirstBtn");

        errorPageLayout->addWidget(errorFirstBtn);

        errorPrevBtn = new QPushButton(errorPaginationWidget);
        errorPrevBtn->setObjectName("errorPrevBtn");

        errorPageLayout->addWidget(errorPrevBtn);

        errorNextBtn = new QPushButton(errorPaginationWidget);
        errorNextBtn->setObjectName("errorNextBtn");

        errorPageLayout->addWidget(errorNextBtn);

        errorLastBtn = new QPushButton(errorPaginationWidget);
        errorLastBtn->setObjectName("errorLastBtn");

        errorPageLayout->addWidget(errorLastBtn);

        errorJumpLabel = new QLabel(errorPaginationWidget);
        errorJumpLabel->setObjectName("errorJumpLabel");

        errorPageLayout->addWidget(errorJumpLabel);

        errorPageInput = new QLineEdit(errorPaginationWidget);
        errorPageInput->setObjectName("errorPageInput");
        errorPageInput->setProperty("fixedWidth", QVariant(50));

        errorPageLayout->addWidget(errorPageInput);

        errorPageLabel = new QLabel(errorPaginationWidget);
        errorPageLabel->setObjectName("errorPageLabel");

        errorPageLayout->addWidget(errorPageLabel);

        errorSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        errorPageLayout->addItem(errorSpacer);


        errorLayout->addWidget(errorPaginationWidget);

        tabWidget->addTab(errorTab, QString());
        vSplitter->addWidget(tabWidget);

        verticalLayout->addWidget(vSplitter);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "VCDU \346\257\224\345\257\271\345\267\245\345\205\267", nullptr));
        vc0Input->setPlaceholderText(QCoreApplication::translate("MainWindow", "VC0 (Hex)", nullptr));
        vc0Input_2->setText(QCoreApplication::translate("MainWindow", "EA0003B4000000AF600000000384111E", nullptr));
        vc0Input_2->setPlaceholderText(QCoreApplication::translate("MainWindow", "IPE (Hex)", nullptr));
        btnOrig->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\216\237\346\226\207\344\273\266", nullptr));
        btnComp->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\346\257\224\350\276\203\346\226\207\344\273\266", nullptr));
        btnCompare->setText(QCoreApplication::translate("MainWindow", "\351\207\215\346\226\260\346\257\224\345\257\271", nullptr));
        diffTable->setProperty("horizontalHeaderLabels", QVariant(QStringList{
            QCoreApplication::translate("MainWindow", "\350\256\260\345\275\225\345\217\267", nullptr),
            QCoreApplication::translate("MainWindow", "\345\255\227\350\212\202\345\201\217\347\247\273", nullptr),
            QCoreApplication::translate("MainWindow", "\345\216\237\345\200\274", nullptr),
            QCoreApplication::translate("MainWindow", "\346\226\260\345\200\274", nullptr)}));
        diffFirstBtn->setText(QCoreApplication::translate("MainWindow", "\351\246\226\351\241\265", nullptr));
        diffPrevBtn->setText(QCoreApplication::translate("MainWindow", "\344\270\212\344\270\200\351\241\265", nullptr));
        diffNextBtn->setText(QCoreApplication::translate("MainWindow", "\344\270\213\344\270\200\351\241\265", nullptr));
        diffLastBtn->setText(QCoreApplication::translate("MainWindow", "\346\234\253\351\241\265", nullptr));
        diffJumpLabel->setText(QCoreApplication::translate("MainWindow", "\350\267\263\350\275\254:", nullptr));
        diffPageLabel->setText(QCoreApplication::translate("MainWindow", " / 1 \351\241\265", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(diffTab), QCoreApplication::translate("MainWindow", "\346\225\260\346\215\256\345\267\256\345\274\202", nullptr));
        errorTable->setProperty("horizontalHeaderLabels", QVariant(QStringList{
            QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr),
            QCoreApplication::translate("MainWindow", "\347\261\273\345\236\213", nullptr),
            QCoreApplication::translate("MainWindow", "\350\256\260\345\275\225\345\217\267", nullptr),
            QCoreApplication::translate("MainWindow", "\350\257\246\346\203\205", nullptr),
            QCoreApplication::translate("MainWindow", "\350\267\263\350\275\254", nullptr)}));
        errorFirstBtn->setText(QCoreApplication::translate("MainWindow", "\351\246\226\351\241\265", nullptr));
        errorPrevBtn->setText(QCoreApplication::translate("MainWindow", "\344\270\212\344\270\200\351\241\265", nullptr));
        errorNextBtn->setText(QCoreApplication::translate("MainWindow", "\344\270\213\344\270\200\351\241\265", nullptr));
        errorLastBtn->setText(QCoreApplication::translate("MainWindow", "\346\234\253\351\241\265", nullptr));
        errorJumpLabel->setText(QCoreApplication::translate("MainWindow", "\350\267\263\350\275\254:", nullptr));
        errorPageLabel->setText(QCoreApplication::translate("MainWindow", " / 1 \351\241\265", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(errorTab), QCoreApplication::translate("MainWindow", "CRC/\345\270\247\351\224\231\350\257\257", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
