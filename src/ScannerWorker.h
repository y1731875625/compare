#ifndef SCANNERWORKER_H
#define SCANNERWORKER_H

#include <QObject>
#include <QString>
#include "VCDUFileReader.h"

class ScannerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ScannerWorker(QObject *parent = nullptr);
public slots:
    void doScan(const QString &path, quint16 vc0, VCDUFileReader::ScanStrategy strategy);
signals:
    void scanFinished(bool success, const QString &errorMsg);
    void progressUpdated(int percent);
    void recordsReady(const QVector<RecordInfo> &records,
                      const QVector<VCDUFileReader::FrameError> &frameErrors,
                      const QVector<int> &crcErrors);
private:
    VCDUFileReader m_reader;
};

#endif // SCANNERWORKER_H