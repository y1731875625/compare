#ifndef VCDUFILEREADER_H
#define VCDUFILEREADER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QtGlobal>
#include <QFile>

struct RecordInfo {
    qint64 fileOffset;
    quint32 frameCount;
    bool crcValid;
};

class VCDUFileReader : public QObject
{
    Q_OBJECT
public:
    enum ScanStrategy {
        StrategyVCDU,   // 原有：1A CF FC 1D + VC0，VC0 在前导后4字节
        StrategyEA      // 新增：EA 00 03 B4 + 56字节偏移 + VC0
    };

    explicit VCDUFileReader(QObject *parent = nullptr);
    ~VCDUFileReader();

    // 打开并扫描，指定策略
    bool open(const QString &filePath, quint16 expectedVC0, ScanStrategy strategy = StrategyVCDU);
    bool openForRead(const QString &filePath);
    void close();
    void scanFile();

    const QVector<RecordInfo>& records() const { return m_records; }
    int recordCount() const { return m_records.size(); }
    QByteArray readRawRecord(int index) const;

    struct FrameError {
        int recordIndex;
        quint32 expectedFrame;
        quint32 actualFrame;
    };
    const QVector<FrameError>& frameErrors() const { return m_frameErrors; }
    const QVector<int>& crcErrors() const { return m_crcErrors; }

    bool verifyCRCDirect(const uchar* data) const;

    void setResults(const QVector<RecordInfo> &records,
                    const QVector<FrameError> &frameErrors,
                    const QVector<int> &crcErrors);

signals:
    void progressUpdated(int percent);

private:
    quint32 readFrameCount(const QByteArray &raw) const;
    bool verifyCRC(const QByteArray &raw) const;

    QString m_filePath;
    mutable QFile m_file;
    quint16 m_expectedVC0 = 0;
    ScanStrategy m_strategy = StrategyVCDU;
    QVector<RecordInfo> m_records;
    QVector<FrameError> m_frameErrors;
    QVector<int> m_crcErrors;
    bool m_isOpen = false;

    static const QByteArray PREAMBLE_VCDU;   // 1A CF FC 1D
    static const QByteArray PREAMBLE_EA;     // EA 00 03 B4
};

#endif // VCDUFILEREADER_H