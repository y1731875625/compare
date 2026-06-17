#include "ScannerWorker.h"
#include <QDebug>

ScannerWorker::ScannerWorker(QObject *parent) : QObject(parent) {}

void ScannerWorker::doScan(const QString &path, quint16 vc0, VCDUFileReader::ScanStrategy strategy)
{
    if (!m_reader.open(path, vc0, strategy)) {
        emit scanFinished(false, "无法打开文件或文件无效");
        return;
    }
    connect(&m_reader, &VCDUFileReader::progressUpdated,
            this, &ScannerWorker::progressUpdated);
    m_reader.scanFile();
    emit recordsReady(m_reader.records(), m_reader.frameErrors(), m_reader.crcErrors());
    emit scanFinished(true, "");
}