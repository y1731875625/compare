#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QTableWidget>
#include <QProgressDialog>
#include "RecordWidget.h"
#include "ScannerWorker.h"
#include "VCDUFileReader.h"
#include "DiffEngine.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSelectOriginal();
    void onSelectCompared();
    void handleOriginalScanned(bool success, const QString &error);
    void handleComparedScanned(bool success, const QString &error);
    void onCompare();
    void onDiffItemClicked(int row, int col);
    void onErrorItemClicked(int row, int col);

    // 分页槽
    void onDiffFirstPage();
    void onDiffPrevPage();
    void onDiffNextPage();
    void onDiffLastPage();
    void onDiffPageJump();
    void onErrorFirstPage();
    void onErrorPrevPage();
    void onErrorNextPage();
    void onErrorLastPage();
    void onErrorPageJump();

private:
    Ui::MainWindow *ui;

    // 业务逻辑成员
    QString m_origPath, m_compPath;
    VCDUFileReader m_readerOrig, m_readerComp;
    bool m_origLoaded = false, m_compLoaded = false;
    int m_currentRecordIndex = 0;

    QThread *m_origThread = nullptr;
    ScannerWorker *m_origWorker = nullptr;
    QThread *m_compThread = nullptr;
    ScannerWorker *m_compWorker = nullptr;
    QProgressDialog *m_progressDialog = nullptr;

    // 差异相关
    QString m_diffFilePath;
    qint64 m_totalDiffCount = 0;
    int m_currentDiffPage = 0;
    int m_totalDiffPages = 0;

    // 错误相关
    struct ErrorItem {
        QString file;
        QString type;
        int recordIndex;
        QString detail;
        bool isLeft;
    };
    QVector<ErrorItem> m_allErrors;
    int m_currentErrorPage = 0;
    int m_totalErrorPages = 0;

    QVector<FrameMatchError> m_frameMatchErrors;

    void displayCurrentRecords();
    void compareAndUpdate();
    void loadDiffPage(int pageIndex);
    void populateErrorTables();
    void loadErrorPage(int pageIndex);
};

#endif // MAINWINDOW_H