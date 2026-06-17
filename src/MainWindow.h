#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QThread>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>
#include <QDir>
#include <QDateTime>
#include "VCDUFileReader.h"
#include "RecordWidget.h"
#include "DiffEngine.h"
#include "ScannerWorker.h"

QT_BEGIN_NAMESPACE
class QSplitter;
class QProgressDialog;
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
    void onCompare();
    void onDiffItemClicked(int row, int col);
    void onErrorItemClicked(int row, int col);
    void onRecordIndexChanged(int newIndex);

    void handleOriginalScanned(bool success, const QString &error);
    void handleComparedScanned(bool success, const QString &error);

    // 分页控制
    void onDiffPrevPage();
    void onDiffNextPage();
    void onDiffFirstPage();
    void onDiffLastPage();
    void onDiffPageJump();

    void onErrorPrevPage();
    void onErrorNextPage();
    void onErrorFirstPage();
    void onErrorLastPage();
    void onErrorPageJump();

private:
    void displayCurrentRecords();
    void compareAndUpdate();
    void loadDiffPage(int pageIndex);
    void loadErrorPage(int pageIndex);
    void populateErrorTables();

    // 错误列表存储（用于分页）
    struct ErrorItem {
        QString file;
        QString type;
        int recordIndex;
        QString detail;
        bool isLeft;
    };

    VCDUFileReader m_readerOrig;
    VCDUFileReader m_readerComp;
    QVector<FrameMatchError> m_frameMatchErrors;

    // 差异文件相关
    QString m_diffFilePath;
    qint64 m_totalDiffCount = 0;

    RecordWidget *m_leftWidget;
    RecordWidget *m_rightWidget;
    QTableWidget *m_diffTable;
    QTableWidget *m_errorTable;
    QTabWidget *m_tabWidget;

    QPushButton *m_btnOrig;
    QPushButton *m_btnComp;
    QPushButton *m_btnCompare;
    QLineEdit *m_vc0Input;

    QString m_origPath;
    QString m_compPath;
    int m_currentRecordIndex = -1;

    // 线程相关
    QThread *m_origThread = nullptr;
    ScannerWorker *m_origWorker = nullptr;
    QThread *m_compThread = nullptr;
    ScannerWorker *m_compWorker = nullptr;
    QProgressDialog *m_progressDialog = nullptr;

    bool m_origLoaded = false;
    bool m_compLoaded = false;

    // 差异分页
    static constexpr int DIFF_PAGE_SIZE = 10000;
    int m_currentDiffPage = 0;
    int m_totalDiffPages = 0;

    // 错误分页
    static constexpr int ERROR_PAGE_SIZE = 10000;
    int m_currentErrorPage = 0;
    int m_totalErrorPages = 0;
    QVector<ErrorItem> m_allErrors;

    // 分页控件
    QWidget *m_diffPaginationWidget;
    QWidget *m_errorPaginationWidget;
    QPushButton *m_diffPrevBtn;
    QPushButton *m_diffNextBtn;
    QPushButton *m_diffFirstBtn;
    QPushButton *m_diffLastBtn;
    QLineEdit *m_diffPageInput;
    QLabel *m_diffPageLabel;

    QPushButton *m_errorPrevBtn;
    QPushButton *m_errorNextBtn;
    QPushButton *m_errorFirstBtn;
    QPushButton *m_errorLastBtn;
    QLineEdit *m_errorPageInput;
    QLabel *m_errorPageLabel;
};

#endif // MAINWINDOW_H