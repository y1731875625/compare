#include "MainWindow.h"
#include "RecordConstants.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDebug>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建中央部件
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // ========== 顶部控件 ==========
    QHBoxLayout *topLayout = new QHBoxLayout;
    m_vc0Input = new QLineEdit;
    m_vc0Input->setPlaceholderText("VC0 (Hex)");
    m_btnOrig = new QPushButton("选择原文件");
    m_btnComp = new QPushButton("选择比较文件");
    m_btnCompare = new QPushButton("重新比对");
    m_btnCompare->setEnabled(false);
    topLayout->addWidget(m_vc0Input);
    topLayout->addWidget(m_btnOrig);
    topLayout->addWidget(m_btnComp);
    topLayout->addWidget(m_btnCompare);
    mainLayout->addLayout(topLayout);

    // ========== 中间：左右分栏（水平分割器） ==========
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    m_leftWidget = new RecordWidget;
    m_rightWidget = new RecordWidget;
    splitter->addWidget(m_leftWidget);
    splitter->addWidget(m_rightWidget);

    // ========== 底部：差异列表和错误列表（标签页） ==========
    m_tabWidget = new QTabWidget;

    // ---- 差异列表标签页 ----
    QWidget *diffTab = new QWidget;
    QVBoxLayout *diffLayout = new QVBoxLayout(diffTab);
    m_diffTable = new QTableWidget;
    m_diffTable->setColumnCount(4);
    m_diffTable->setHorizontalHeaderLabels({"记录号", "字节偏移", "原值", "新值"});
    m_diffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    diffLayout->addWidget(m_diffTable);

    // 差异分页控件
    m_diffPaginationWidget = new QWidget;
    QHBoxLayout *diffPageLayout = new QHBoxLayout(m_diffPaginationWidget);
    m_diffFirstBtn = new QPushButton("首页");
    m_diffPrevBtn = new QPushButton("上一页");
    m_diffNextBtn = new QPushButton("下一页");
    m_diffLastBtn = new QPushButton("末页");
    m_diffPageInput = new QLineEdit;
    m_diffPageInput->setFixedWidth(50);
    m_diffPageLabel = new QLabel(" / 1 页");
    diffPageLayout->addWidget(m_diffFirstBtn);
    diffPageLayout->addWidget(m_diffPrevBtn);
    diffPageLayout->addWidget(m_diffNextBtn);
    diffPageLayout->addWidget(m_diffLastBtn);
    diffPageLayout->addSpacing(10);
    diffPageLayout->addWidget(new QLabel("跳转:"));
    diffPageLayout->addWidget(m_diffPageInput);
    diffPageLayout->addWidget(m_diffPageLabel);
    diffPageLayout->addStretch();
    diffLayout->addWidget(m_diffPaginationWidget);
    m_diffPaginationWidget->setVisible(false);

    m_tabWidget->addTab(diffTab, "数据差异");

    // ---- CRC/帧错误标签页 ----
    QWidget *errorTab = new QWidget;
    QVBoxLayout *errorLayout = new QVBoxLayout(errorTab);
    m_errorTable = new QTableWidget;
    m_errorTable->setColumnCount(5);
    m_errorTable->setHorizontalHeaderLabels({"文件", "类型", "记录号", "详情", "跳转"});
    m_errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    errorLayout->addWidget(m_errorTable);

    // 错误分页控件
    m_errorPaginationWidget = new QWidget;
    QHBoxLayout *errorPageLayout = new QHBoxLayout(m_errorPaginationWidget);
    m_errorFirstBtn = new QPushButton("首页");
    m_errorPrevBtn = new QPushButton("上一页");
    m_errorNextBtn = new QPushButton("下一页");
    m_errorLastBtn = new QPushButton("末页");
    m_errorPageInput = new QLineEdit;
    m_errorPageInput->setFixedWidth(50);
    m_errorPageLabel = new QLabel(" / 1 页");
    errorPageLayout->addWidget(m_errorFirstBtn);
    errorPageLayout->addWidget(m_errorPrevBtn);
    errorPageLayout->addWidget(m_errorNextBtn);
    errorPageLayout->addWidget(m_errorLastBtn);
    errorPageLayout->addSpacing(10);
    errorPageLayout->addWidget(new QLabel("跳转:"));
    errorPageLayout->addWidget(m_errorPageInput);
    errorPageLayout->addWidget(m_errorPageLabel);
    errorPageLayout->addStretch();
    errorLayout->addWidget(m_errorPaginationWidget);
    m_errorPaginationWidget->setVisible(false);

    m_tabWidget->addTab(errorTab, "CRC/帧错误");

    // ========== ★ 关键修改：将水平分割器和标签页放入垂直分割器 ==========
    QSplitter *vSplitter = new QSplitter(Qt::Vertical);
    vSplitter->addWidget(splitter);      // 上部：左右分栏
    vSplitter->addWidget(m_tabWidget);   // 下部：标签页
    vSplitter->setStretchFactor(0, 3);   // 上部拉伸因子 3
    vSplitter->setStretchFactor(1, 1);   // 下部拉伸因子 1

    // 将垂直分割器添加到主布局（替代原来的两个 addWidget）
    mainLayout->addWidget(vSplitter);

    // ========== 信号连接 ==========
    connect(m_btnOrig, &QPushButton::clicked, this, &MainWindow::onSelectOriginal);
    connect(m_btnComp, &QPushButton::clicked, this, &MainWindow::onSelectCompared);
    connect(m_btnCompare, &QPushButton::clicked, this, &MainWindow::onCompare);
    connect(m_diffTable, &QTableWidget::cellClicked, this, &MainWindow::onDiffItemClicked);
    connect(m_errorTable, &QTableWidget::cellClicked, this, &MainWindow::onErrorItemClicked);

    // 差异分页信号
    connect(m_diffFirstBtn, &QPushButton::clicked, this, &MainWindow::onDiffFirstPage);
    connect(m_diffPrevBtn, &QPushButton::clicked, this, &MainWindow::onDiffPrevPage);
    connect(m_diffNextBtn, &QPushButton::clicked, this, &MainWindow::onDiffNextPage);
    connect(m_diffLastBtn, &QPushButton::clicked, this, &MainWindow::onDiffLastPage);
    connect(m_diffPageInput, &QLineEdit::returnPressed, this, &MainWindow::onDiffPageJump);

    // 错误分页信号
    connect(m_errorFirstBtn, &QPushButton::clicked, this, &MainWindow::onErrorFirstPage);
    connect(m_errorPrevBtn, &QPushButton::clicked, this, &MainWindow::onErrorPrevPage);
    connect(m_errorNextBtn, &QPushButton::clicked, this, &MainWindow::onErrorNextPage);
    connect(m_errorLastBtn, &QPushButton::clicked, this, &MainWindow::onErrorLastPage);
    connect(m_errorPageInput, &QLineEdit::returnPressed, this, &MainWindow::onErrorPageJump);

    resize(1200, 800);
}

MainWindow::~MainWindow()
{
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

void MainWindow::onSelectOriginal()
{
    QString path = QFileDialog::getOpenFileName(this, "选择原文件");
    if (path.isEmpty()) return;
    m_origPath = path;

    QString vc0Hex = m_vc0Input->text().trimmed();
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

void MainWindow::onSelectCompared()
{
    QString path = QFileDialog::getOpenFileName(this, "选择比较文件");
    if (path.isEmpty()) return;
    m_compPath = path;

    QString vc0Hex = m_vc0Input->text().trimmed();
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
    m_btnCompare->setEnabled(true);
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
    m_btnCompare->setEnabled(true);
    setWindowTitle(QString("比较文件记录数: %1").arg(m_readerComp.recordCount()));
}

void MainWindow::displayCurrentRecords()
{
    if (m_origLoaded && m_currentRecordIndex >= 0 &&
        m_currentRecordIndex < m_readerOrig.recordCount()) {
        QByteArray raw = m_readerOrig.readRawRecord(m_currentRecordIndex);
        m_leftWidget->setRecord(raw);
    } else {
        m_leftWidget->clear();
    }

    if (m_compLoaded && m_currentRecordIndex >= 0 &&
        m_currentRecordIndex < m_readerComp.recordCount()) {
        QByteArray raw = m_readerComp.readRawRecord(m_currentRecordIndex);
        m_rightWidget->setRecord(raw);
    } else {
        m_rightWidget->clear();
    }
}

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

    int maxRec = qMin(countA, countB);
    if (m_currentRecordIndex >= maxRec)
        m_currentRecordIndex = 0;
    displayCurrentRecords();
}

// ==================== 差异分页函数 ====================

void MainWindow::loadDiffPage(int pageIndex)
{
    if (m_diffFilePath.isEmpty() || !QFile::exists(m_diffFilePath)) {
        m_diffTable->setRowCount(0);
        m_diffPaginationWidget->setVisible(false);
        return;
    }

    qint64 startIndex = (qint64)pageIndex * DIFF_PAGE_SIZE;
    if (startIndex >= m_totalDiffCount) {
        m_diffTable->setRowCount(0);
        return;
    }

    m_totalDiffPages = (int)((m_totalDiffCount + DIFF_PAGE_SIZE - 1) / DIFF_PAGE_SIZE);
    int count = (int)qMin((qint64)DIFF_PAGE_SIZE, m_totalDiffCount - startIndex);

    QVector<DataDiff> diffs = DiffEngine::loadDiffRange(m_diffFilePath, startIndex, count);

    m_diffTable->setRowCount(diffs.size());
    for (int i = 0; i < diffs.size(); ++i) {
        const DataDiff &d = diffs[i];
        m_diffTable->setItem(i, 0, new QTableWidgetItem(QString::number(d.recordIndex)));
        m_diffTable->setItem(i, 1, new QTableWidgetItem(QString::number(d.byteOffsetInRecord)));
        m_diffTable->setItem(i, 2, new QTableWidgetItem(QString("0x%1").arg(d.valueA, 2, 16, QChar('0'))));
        m_diffTable->setItem(i, 3, new QTableWidgetItem(QString("0x%1").arg(d.valueB, 2, 16, QChar('0'))));
    }

    m_currentDiffPage = pageIndex;
    m_diffPageLabel->setText(QString(" / %1 页").arg(m_totalDiffPages));
    m_diffPageInput->setText(QString::number(pageIndex + 1));
    m_diffPaginationWidget->setVisible(m_totalDiffPages > 1);

    m_diffFirstBtn->setEnabled(pageIndex > 0);
    m_diffPrevBtn->setEnabled(pageIndex > 0);
    m_diffNextBtn->setEnabled(pageIndex < m_totalDiffPages - 1);
    m_diffLastBtn->setEnabled(pageIndex < m_totalDiffPages - 1);
}

void MainWindow::onDiffFirstPage()
{
    loadDiffPage(0);
}

void MainWindow::onDiffPrevPage()
{
    if (m_currentDiffPage > 0) {
        loadDiffPage(m_currentDiffPage - 1);
    }
}

void MainWindow::onDiffNextPage()
{
    if (m_currentDiffPage < m_totalDiffPages - 1) {
        loadDiffPage(m_currentDiffPage + 1);
    }
}

void MainWindow::onDiffLastPage()
{
    if (m_totalDiffPages > 0) {
        loadDiffPage(m_totalDiffPages - 1);
    }
}

void MainWindow::onDiffPageJump()
{
    int page = m_diffPageInput->text().toInt() - 1;
    if (page >= 0 && page < m_totalDiffPages) {
        loadDiffPage(page);
    }
}

void MainWindow::onDiffItemClicked(int row, int col)
{
    if (row < 0 || row >= m_diffTable->rowCount()) return;
    
    // 计算实际索引
    qint64 startIndex = (qint64)m_currentDiffPage * DIFF_PAGE_SIZE;
    qint64 dataIndex = startIndex + row;
    if (dataIndex < 0 || dataIndex >= m_totalDiffCount) return;

    // 从差异文件读取该条记录
    QVector<DataDiff> diffs = DiffEngine::loadDiffRange(m_diffFilePath, dataIndex, 1);
    if (diffs.isEmpty()) return;

    const DataDiff &diff = diffs[0];
    m_currentRecordIndex = diff.recordIndex;
    displayCurrentRecords();

    QColor highlightColor(Qt::yellow);
    m_leftWidget->highlightByteRange(diff.byteOffsetInRecord, 1, highlightColor);
    m_rightWidget->highlightByteRange(diff.byteOffsetInRecord, 1, highlightColor);

    m_leftWidget->jumpToByteOffset(diff.byteOffsetInRecord);
    m_rightWidget->jumpToByteOffset(diff.byteOffsetInRecord);
}

// ==================== 错误列表函数 ====================

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
        m_errorTable->setRowCount(0);
        m_errorPaginationWidget->setVisible(false);
        return;
    }

    int startIndex = pageIndex * ERROR_PAGE_SIZE;
    if (startIndex >= m_allErrors.size()) {
        return;
    }

    m_totalErrorPages = (m_allErrors.size() + ERROR_PAGE_SIZE - 1) / ERROR_PAGE_SIZE;
    int endIndex = qMin(startIndex + ERROR_PAGE_SIZE, m_allErrors.size());
    int count = endIndex - startIndex;

    m_errorTable->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        const ErrorItem &err = m_allErrors[startIndex + i];
        int row = i;
        m_errorTable->setItem(row, 0, new QTableWidgetItem(err.file));
        m_errorTable->setItem(row, 1, new QTableWidgetItem(err.type));
        m_errorTable->setItem(row, 2, new QTableWidgetItem(
            err.recordIndex >= 0 ? QString::number(err.recordIndex) : ""));
        m_errorTable->setItem(row, 3, new QTableWidgetItem(err.detail));

        QTableWidgetItem *jumpItem = new QTableWidgetItem("点击跳转");
        jumpItem->setForeground(Qt::blue);
        jumpItem->setTextAlignment(Qt::AlignCenter);
        jumpItem->setData(Qt::UserRole, 
            QVariant::fromValue(QPair<int,bool>(err.recordIndex, err.isLeft)));
        m_errorTable->setItem(row, 4, jumpItem);
    }

    m_currentErrorPage = pageIndex;
    m_errorPageLabel->setText(QString(" / %1 页").arg(m_totalErrorPages));
    m_errorPageInput->setText(QString::number(pageIndex + 1));
    m_errorPaginationWidget->setVisible(m_totalErrorPages > 1);

    m_errorFirstBtn->setEnabled(pageIndex > 0);
    m_errorPrevBtn->setEnabled(pageIndex > 0);
    m_errorNextBtn->setEnabled(pageIndex < m_totalErrorPages - 1);
    m_errorLastBtn->setEnabled(pageIndex < m_totalErrorPages - 1);
}

// ==================== 错误分页控制 ====================

void MainWindow::onErrorFirstPage()
{
    loadErrorPage(0);
}

void MainWindow::onErrorPrevPage()
{
    if (m_currentErrorPage > 0) {
        loadErrorPage(m_currentErrorPage - 1);
    }
}

void MainWindow::onErrorNextPage()
{
    if (m_currentErrorPage < m_totalErrorPages - 1) {
        loadErrorPage(m_currentErrorPage + 1);
    }
}

void MainWindow::onErrorLastPage()
{
    if (m_totalErrorPages > 0) {
        loadErrorPage(m_totalErrorPages - 1);
    }
}

void MainWindow::onErrorPageJump()
{
    int page = m_errorPageInput->text().toInt() - 1;
    if (page >= 0 && page < m_totalErrorPages) {
        loadErrorPage(page);
    }
}

void MainWindow::onErrorItemClicked(int row, int col)
{
    if (col != 4) return; // 只有"点击跳转"列才处理
    QTableWidgetItem *item = m_errorTable->item(row, 4);
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
    // 高亮帧计数位置（偏移2，长度3）
    if (isLeft && recordIndex < m_readerOrig.recordCount()) {
        m_leftWidget->highlightByteRange(VCDU::FRAME_OFFSET, 3, highlightColor);
        m_leftWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
    } else if (!isLeft && recordIndex < m_readerComp.recordCount()) {
        m_rightWidget->highlightByteRange(VCDU::FRAME_OFFSET, 3, highlightColor);
        m_rightWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
    } else {
        if (recordIndex < m_readerOrig.recordCount()) {
            m_leftWidget->highlightByteRange(VCDU::FRAME_OFFSET, 3, highlightColor);
            m_leftWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
        }
        if (recordIndex < m_readerComp.recordCount()) {
            m_rightWidget->highlightByteRange(VCDU::FRAME_OFFSET, 3, highlightColor);
            m_rightWidget->jumpToByteOffset(VCDU::FRAME_OFFSET);
        }
    }
}

void MainWindow::onCompare()
{
    if (!m_origLoaded || !m_compLoaded) {
        QMessageBox::warning(this, "提示", "请加载两个文件");
        return;
    }
    compareAndUpdate();
}

void MainWindow::onRecordIndexChanged(int newIndex)
{
    if (newIndex >= 0 && newIndex < m_readerOrig.recordCount() &&
        newIndex < m_readerComp.recordCount()) {
        m_currentRecordIndex = newIndex;
        displayCurrentRecords();
    }
}