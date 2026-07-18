#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "RecordConstants.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStatusBar>
#include <QDir>
#include <QDateTime>
#include <QDebug>

// 分页大小常量（与原有定义一致）
static const int DIFF_PAGE_SIZE = 100;
static const int ERROR_PAGE_SIZE = 50;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // ---------- 信号连接 ----------
    connect(ui->btnOrig, &QPushButton::clicked, this, &MainWindow::onSelectOriginal);
    connect(ui->btnComp, &QPushButton::clicked, this, &MainWindow::onSelectCompared);
    connect(ui->btnCompare, &QPushButton::clicked, this, &MainWindow::onCompare);
    connect(ui->diffTable, &QTableWidget::cellClicked, this, &MainWindow::onDiffItemClicked);
    connect(ui->errorTable, &QTableWidget::cellClicked, this, &MainWindow::onErrorItemClicked);

    // 差异分页
    connect(ui->diffFirstBtn, &QPushButton::clicked, this, &MainWindow::onDiffFirstPage);
    connect(ui->diffPrevBtn, &QPushButton::clicked, this, &MainWindow::onDiffPrevPage);
    connect(ui->diffNextBtn, &QPushButton::clicked, this, &MainWindow::onDiffNextPage);
    connect(ui->diffLastBtn, &QPushButton::clicked, this, &MainWindow::onDiffLastPage);
    connect(ui->diffPageInput, &QLineEdit::returnPressed, this, &MainWindow::onDiffPageJump);

    // 错误分页
    connect(ui->errorFirstBtn, &QPushButton::clicked, this, &MainWindow::onErrorFirstPage);
    connect(ui->errorPrevBtn, &QPushButton::clicked, this, &MainWindow::onErrorPrevPage);
    connect(ui->errorNextBtn, &QPushButton::clicked, this, &MainWindow::onErrorNextPage);
    connect(ui->errorLastBtn, &QPushButton::clicked, this, &MainWindow::onErrorLastPage);
    connect(ui->errorPageInput, &QLineEdit::returnPressed, this, &MainWindow::onErrorPageJump);

    // 初始隐藏分页控件
    ui->diffPaginationWidget->setVisible(false);
    ui->errorPaginationWidget->setVisible(false);

    

    // 窗口初始大小（可由 .ui 决定，也可在此设置）
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_origThread) {
        m_origThread->quit();
        m_origThread->wait();
    }
    if (m_compThread) {
        m_compThread->quit();
        m_compThread->wait();
    }
    if (!m_diffFilePath.isEmpty() && QFile::exists(m_diffFilePath)) {
        QFile::remove(m_diffFilePath);
    }
}

// -------------------- 选择原文件 --------------------
void MainWindow::onSelectOriginal()
{
    QString path = QFileDialog::getOpenFileName(this, "选择原文件");
    if (path.isEmpty()) return;
    m_origPath = path;

    QString vc0Hex = ui->vc0Input->text().trimmed();
    bool ok;
    quint16 vc0 = vc0Hex.toUShort(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的VC0十六进制值");
        return;
    }

    m_origThread = new QThread(this);
    m_origWorker = new ScannerWorker;
    m_origWorker->moveToThread(m_origThread);

    m_progressDialog = new QProgressDialog("正在扫描原文件...", "取消", 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    connect(m_progressDialog, &QProgressDialog::canceled, [=]() {});

    connect(m_origWorker, &ScannerWorker::progressUpdated,
            m_progressDialog, &QProgressDialog::setValue);
    connect(m_origWorker, &ScannerWorker::scanFinished,
            this, &MainWindow::handleOriginalScanned);
    connect(m_origWorker, &ScannerWorker::recordsReady,
            [this](const QVector<RecordInfo> &records,
                   const QVector<VCDUFileReader::FrameError> &frameErrors,
                   const QVector<int> &crcErrors) {
        m_readerOrig.setResults(records, frameErrors, crcErrors);
    });
    connect(m_origThread, &QThread::finished, m_origWorker, &QObject::deleteLater);
    connect(m_origThread, &QThread::finished, m_origThread, &QObject::deleteLater);

    m_origThread->start();
    QMetaObject::invokeMethod(m_origWorker, "doScan", Qt::QueuedConnection,
                              Q_ARG(QString, path),
                              Q_ARG(quint16, vc0),
                              Q_ARG(VCDUFileReader::ScanStrategy, VCDUFileReader::StrategyVCDU));
}

// extern QString EAstring ;
// -------------------- 选择比较文件 --------------------
void MainWindow::onSelectCompared()
{
    QString path = QFileDialog::getOpenFileName(this, "选择比较文件");
    if (path.isEmpty()) return;
    m_compPath = path;

    QString vc0Hex = ui->vc0Input->text().trimmed();
    QString eaHex = ui->vc0Input_2->text().trimmed(); 
    QByteArray customPreamble;
    if (eaHex.isEmpty()) {
        QMessageBox::warning(this, "错误", "EA 前导码不能为空，请输入 32 个十六进制字符");
        return;
    }
    customPreamble = QByteArray::fromHex(eaHex.toUtf8());
    if (customPreamble.size() != 16) {
        QMessageBox::warning(this, "错误",
                             QString("EA 前导码长度错误，需要 16 字节（32 个十六进制字符），当前为 %1 字节")
                             .arg(customPreamble.size()));
        return;
    }
    VCDUFileReader::PREAMBLE_EA = customPreamble;
    bool ok;
    quint16 vc0 = vc0Hex.toUShort(&ok, 16);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的VC0十六进制值");
        return;
    }

    m_compThread = new QThread(this);
    m_compWorker = new ScannerWorker;
    m_compWorker->moveToThread(m_compThread);

    m_progressDialog = new QProgressDialog("正在扫描比较文件...", "取消", 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);

    connect(m_compWorker, &ScannerWorker::progressUpdated,
            m_progressDialog, &QProgressDialog::setValue);
    connect(m_compWorker, &ScannerWorker::scanFinished,
            this, &MainWindow::handleComparedScanned);
    connect(m_compWorker, &ScannerWorker::recordsReady,
            [this](const QVector<RecordInfo> &records,
                   const QVector<VCDUFileReader::FrameError> &frameErrors,
                   const QVector<int> &crcErrors) {
        m_readerComp.setResults(records, frameErrors, crcErrors);
    });
    connect(m_compThread, &QThread::finished, m_compWorker, &QObject::deleteLater);
    connect(m_compThread, &QThread::finished, m_compThread, &QObject::deleteLater);

    m_compThread->start();
    QMetaObject::invokeMethod(m_compWorker, "doScan", Qt::QueuedConnection,
                              Q_ARG(QString, path),
                              Q_ARG(quint16, vc0),
                              Q_ARG(VCDUFileReader::ScanStrategy, VCDUFileReader::StrategyEA));
}

// -------------------- 扫描完成回调 --------------------
void MainWindow::handleOriginalScanned(bool success, const QString &error)
{
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
    if (!success) {
        QMessageBox::warning(this, "错误", error);
        return;
    }

    if (!m_readerOrig.openForRead(m_origPath)) {
        QMessageBox::warning(this, "错误", "无法打开原文件用于读取");
        return;
    }

    m_origLoaded = true;
    m_currentRecordIndex = 0;
    displayCurrentRecords();
    if (m_compLoaded) {
        compareAndUpdate();
    }
    ui->btnCompare->setEnabled(true);
    setWindowTitle(QString("原文件记录数: %1").arg(m_readerOrig.recordCount()));
}

void MainWindow::handleComparedScanned(bool success, const QString &error)
{
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
    if (!success) {
        QMessageBox::warning(this, "错误", error);
        return;
    }

    if (!m_readerComp.openForRead(m_compPath)) {
        QMessageBox::warning(this, "错误", "无法打开比较文件用于读取");
        return;
    }

    m_compLoaded = true;
    m_currentRecordIndex = 0;
    displayCurrentRecords();
    if (m_origLoaded) {
        compareAndUpdate();
    }
    ui->btnCompare->setEnabled(true);
    setWindowTitle(QString("比较文件记录数: %1").arg(m_readerComp.recordCount()));
}

// -------------------- 显示当前记录 --------------------
void MainWindow::displayCurrentRecords()
{
    if (m_origLoaded && m_currentRecordIndex >= 0 &&
        m_currentRecordIndex < m_readerOrig.recordCount()) {
        QByteArray raw = m_readerOrig.readRawRecord(m_currentRecordIndex);
        ui->leftWidget->setRecord(raw);
    } else {
        ui->leftWidget->clear();
    }

    if (m_compLoaded && m_currentRecordIndex >= 0 &&
        m_currentRecordIndex < m_readerComp.recordCount()) {
        QByteArray raw = m_readerComp.readRawRecord(m_currentRecordIndex);
        ui->rightWidget->setRecord(raw);
    } else {
        ui->rightWidget->clear();
    }
}

// -------------------- 比对核心 --------------------
void MainWindow::compareAndUpdate()
{
    if (!m_origLoaded || !m_compLoaded) return;

    int countA = m_readerOrig.recordCount();
    int countB = m_readerComp.recordCount();

    int crcErrB = m_readerComp.crcErrors().size();
    if (crcErrB > countB * 0.1) {
        int ret = QMessageBox::question(this, "警告",
            QString("比较文件有 %1 条记录的CRC校验错误（共 %2 条记录），\n"
                    "可能表示数据格式不匹配。是否继续比对？")
            .arg(crcErrB).arg(countB),
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) return;
    }

    QProgressDialog progress("正在生成差异文件...", "取消", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);

    m_diffFilePath = QDir::temp().filePath("vcd_diff_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".dat");

    try {
        m_totalDiffCount = DiffEngine::generateDiffFile(m_readerOrig, m_readerComp,
            m_diffFilePath,
            [&](int percent) {
                if (progress.wasCanceled()) return;
                progress.setValue(percent);
            });
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "错误", QString("生成差异文件失败: %1").arg(e.what()));
        return;
    }

    progress.setValue(100);

    m_frameMatchErrors = DiffEngine::compareFrameSequences(m_readerOrig, m_readerComp);

    populateErrorTables();

    m_currentDiffPage = 0;
    loadDiffPage(0);

    statusBar()->showMessage(QString("总差异数: %1 条").arg(m_totalDiffCount));
    qDebug() << "总差异数: " << m_totalDiffCount;
    if(m_totalDiffCount == 0) {
        qDebug()<<"载荷数据内容比对一致";
    }

    int maxRec = qMin(countA, countB);
    if (m_currentRecordIndex >= maxRec)
        m_currentRecordIndex = 0;
    displayCurrentRecords();
}

// ==================== 差异分页 ====================
void MainWindow::loadDiffPage(int pageIndex)
{
    if (m_diffFilePath.isEmpty() || !QFile::exists(m_diffFilePath)) {
        ui->diffTable->setRowCount(0);
        ui->diffPaginationWidget->setVisible(false);
        return;
    }

    qint64 startIndex = (qint64)pageIndex * DIFF_PAGE_SIZE;
    if (startIndex >= m_totalDiffCount) {
        ui->diffTable->setRowCount(0);
        return;
    }

    m_totalDiffPages = (int)((m_totalDiffCount + DIFF_PAGE_SIZE - 1) / DIFF_PAGE_SIZE);
    int count = (int)qMin((qint64)DIFF_PAGE_SIZE, m_totalDiffCount - startIndex);

    QVector<DataDiff> diffs = DiffEngine::loadDiffRange(m_diffFilePath, startIndex, count);

    ui->diffTable->setRowCount(diffs.size());
    for (int i = 0; i < diffs.size(); ++i) {
        const DataDiff &d = diffs[i];
        ui->diffTable->setItem(i, 0, new QTableWidgetItem(QString::number(d.recordIndex)));
        ui->diffTable->setItem(i, 1, new QTableWidgetItem(QString::number(d.byteOffsetInRecord)));
        ui->diffTable->setItem(i, 2, new QTableWidgetItem(QString("0x%1").arg(d.valueA, 2, 16, QChar('0'))));
        ui->diffTable->setItem(i, 3, new QTableWidgetItem(QString("0x%1").arg(d.valueB, 2, 16, QChar('0'))));
    }

    m_currentDiffPage = pageIndex;
    ui->diffPageLabel->setText(QString(" / %1 页").arg(m_totalDiffPages));
    ui->diffPageInput->setText(QString::number(pageIndex + 1));
    ui->diffPaginationWidget->setVisible(m_totalDiffPages > 1);

    ui->diffFirstBtn->setEnabled(pageIndex > 0);
    ui->diffPrevBtn->setEnabled(pageIndex > 0);
    ui->diffNextBtn->setEnabled(pageIndex < m_totalDiffPages - 1);
    ui->diffLastBtn->setEnabled(pageIndex < m_totalDiffPages - 1);
}

void MainWindow::onDiffFirstPage()   { loadDiffPage(0); }
void MainWindow::onDiffPrevPage()    { if (m_currentDiffPage > 0) loadDiffPage(m_currentDiffPage - 1); }
void MainWindow::onDiffNextPage()    { if (m_currentDiffPage < m_totalDiffPages - 1) loadDiffPage(m_currentDiffPage + 1); }
void MainWindow::onDiffLastPage()    { if (m_totalDiffPages > 0) loadDiffPage(m_totalDiffPages - 1); }
void MainWindow::onDiffPageJump()
{
    int page = ui->diffPageInput->text().toInt() - 1;
    if (page >= 0 && page < m_totalDiffPages) {
        loadDiffPage(page);
    }
}

// -------------------- 差异条目点击跳转 --------------------
void MainWindow::onDiffItemClicked(int row, int col)
{
    if (row < 0 || row >= ui->diffTable->rowCount()) return;

    qint64 startIndex = (qint64)m_currentDiffPage * DIFF_PAGE_SIZE;
    qint64 dataIndex = startIndex + row;
    if (dataIndex < 0 || dataIndex >= m_totalDiffCount) return;

    QVector<DataDiff> diffs = DiffEngine::loadDiffRange(m_diffFilePath, dataIndex, 1);
    if (diffs.isEmpty()) return;

    const DataDiff &diff = diffs[0];
    m_currentRecordIndex = diff.recordIndex;
    displayCurrentRecords();
    QColor bgColor(Qt::green);
    QColor fgColor(Qt::black);
    ui->leftWidget->highlightByteRange(diff.byteOffsetInRecord, 1, bgColor, fgColor);
    ui->rightWidget->highlightByteRange(diff.byteOffsetInRecord, 1, bgColor, fgColor);

    ui->leftWidget->jumpToByteOffset(diff.byteOffsetInRecord);
    ui->rightWidget->jumpToByteOffset(diff.byteOffsetInRecord);
}

// ==================== 错误列表 ====================
void MainWindow::populateErrorTables()
{
    m_allErrors.clear();

    auto addError = [this](const QString &file, const QString &type, int rec, const QString &detail, bool isLeft) {
        ErrorItem item;
        item.file = file;
        item.type = type;
        item.recordIndex = rec;
        item.detail = detail;
        item.isLeft = isLeft;
        m_allErrors.append(item);
    };

    // CRC 错误
    for (int idx : m_readerOrig.crcErrors())
        addError("原文件", "CRC校验错误", idx, "CRC值不匹配", true);
    for (int idx : m_readerComp.crcErrors())
        addError("比较文件", "CRC校验错误", idx, "CRC值不匹配", false);

    // 帧不连续错误
    for (const auto &err : m_readerOrig.frameErrors())
        addError("原文件", "帧不连续", err.recordIndex,
                 QString("期望 %1, 实际 %2").arg(err.expectedFrame).arg(err.actualFrame), true);
    for (const auto &err : m_readerComp.frameErrors())
        addError("比较文件", "帧不连续", err.recordIndex,
                 QString("期望 %1, 实际 %2").arg(err.expectedFrame).arg(err.actualFrame), false);

    // 帧匹配错误
    for (const auto &err : m_frameMatchErrors) {
        QString file, type, detail;
        bool isLeft = true;
        int recIndex = err.recordIndex;
        if (err.type == FrameMatchError::CountMismatch) {
            file = "全局";
            type = "帧数量不匹配";
            detail = QString("原文件 %1 条, 比较文件 %2 条").arg(err.frameA).arg(err.frameB);
            recIndex = -1;
            addError(file, type, recIndex, detail, true);
        } else if (err.type == FrameMatchError::MissingInA) {
            file = "原文件";
            type = "帧缺失";
            detail = QString("比较文件有帧 %1, 原文件缺失").arg(err.frameB);
            isLeft = true;
            addError(file, type, recIndex, detail, isLeft);
        } else if (err.type == FrameMatchError::MissingInB) {
            file = "比较文件";
            type = "帧缺失";
            detail = QString("原文件有帧 %1, 比较文件缺失").arg(err.frameA);
            isLeft = false;
            addError(file, type, recIndex, detail, isLeft);
        }
    }

    m_currentErrorPage = 0;
    loadErrorPage(0);
}

void MainWindow::loadErrorPage(int pageIndex)
{
    if (m_allErrors.isEmpty()) {
        ui->errorTable->setRowCount(0);
        ui->errorPaginationWidget->setVisible(false);
        return;
    }

    int startIndex = pageIndex * ERROR_PAGE_SIZE;
    if (startIndex >= m_allErrors.size()) {
        return;
    }

    m_totalErrorPages = (m_allErrors.size() + ERROR_PAGE_SIZE - 1) / ERROR_PAGE_SIZE;
    int endIndex = qMin(startIndex + ERROR_PAGE_SIZE, m_allErrors.size());
    int count = endIndex - startIndex;

    ui->errorTable->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        const ErrorItem &err = m_allErrors[startIndex + i];
        int row = i;
        ui->errorTable->setItem(row, 0, new QTableWidgetItem(err.file));
        ui->errorTable->setItem(row, 1, new QTableWidgetItem(err.type));
        ui->errorTable->setItem(row, 2, new QTableWidgetItem(
            err.recordIndex >= 0 ? QString::number(err.recordIndex) : ""));
        ui->errorTable->setItem(row, 3, new QTableWidgetItem(err.detail));

        QTableWidgetItem *jumpItem = new QTableWidgetItem("点击跳转");
        jumpItem->setForeground(Qt::blue);
        jumpItem->setTextAlignment(Qt::AlignCenter);
        jumpItem->setData(Qt::UserRole,
            QVariant::fromValue(QPair<int,bool>(err.recordIndex, err.isLeft)));
        ui->errorTable->setItem(row, 4, jumpItem);
    }

    m_currentErrorPage = pageIndex;
    ui->errorPageLabel->setText(QString(" / %1 页").arg(m_totalErrorPages));
    ui->errorPageInput->setText(QString::number(pageIndex + 1));
    ui->errorPaginationWidget->setVisible(m_totalErrorPages > 1);

    ui->errorFirstBtn->setEnabled(pageIndex > 0);
    ui->errorPrevBtn->setEnabled(pageIndex > 0);
    ui->errorNextBtn->setEnabled(pageIndex < m_totalErrorPages - 1);
    ui->errorLastBtn->setEnabled(pageIndex < m_totalErrorPages - 1);
}

// ==================== 错误分页控制 ====================
void MainWindow::onErrorFirstPage()   { loadErrorPage(0); }
void MainWindow::onErrorPrevPage()    { if (m_currentErrorPage > 0) loadErrorPage(m_currentErrorPage - 1); }
void MainWindow::onErrorNextPage()    { if (m_currentErrorPage < m_totalErrorPages - 1) loadErrorPage(m_currentErrorPage + 1); }
void MainWindow::onErrorLastPage()    { if (m_totalErrorPages > 0) loadErrorPage(m_totalErrorPages - 1); }
void MainWindow::onErrorPageJump()
{
    int page = ui->errorPageInput->text().toInt() - 1;
    if (page >= 0 && page < m_totalErrorPages) {
        loadErrorPage(page);
    }
}

// -------------------- 错误条目点击跳转 --------------------
void MainWindow::onErrorItemClicked(int row, int col)
{
    if (col != 4) return; // 只有"点击跳转"列才处理
    QTableWidgetItem *item = ui->errorTable->item(row, 4);
    if (!item) return;

    QPair<int,bool> data = item->data(Qt::UserRole).value<QPair<int,bool>>();
    int recordIndex = data.first;
    bool isLeft = data.second;

    if (recordIndex < 0) {
        QMessageBox::information(this, "提示", "此错误为全局统计信息，无法跳转到具体记录");
        return;
    }

    m_currentRecordIndex = recordIndex;
    displayCurrentRecords();

    QColor highlightColor(Qt::red);
    // 高亮帧计数位置（偏移2，长度3），常量 VCDU::FRAME_OFFSET 需定义
    if (isLeft && recordIndex < m_readerOrig.recordCount()) {
        ui->leftWidget->highlightByteRange(VCDU::FRAME_OFFSET, 2, highlightColor);
        ui->leftWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
    } else if (!isLeft && recordIndex < m_readerComp.recordCount()) {
        ui->rightWidget->highlightByteRange(VCDU::FRAME_OFFSET, 2, highlightColor);
        ui->rightWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
    } else {
        if (recordIndex < m_readerOrig.recordCount()) {
            ui->leftWidget->highlightByteRange(VCDU::FRAME_OFFSET, 2, highlightColor);
            ui->leftWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
        }
        if (recordIndex < m_readerComp.recordCount()) {
            ui->rightWidget->highlightByteRange(VCDU::FRAME_OFFSET, 2, highlightColor);
            ui->rightWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
        }
    }
}

// -------------------- 手动触发比对 --------------------
void MainWindow::onCompare()
{
    if (!m_origLoaded || !m_compLoaded) {
        QMessageBox::warning(this, "提示", "请加载两个文件");
        return;
    }
    compareAndUpdate();
}