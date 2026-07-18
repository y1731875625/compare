#include "VCDUFileReader.h"
#include "RecordConstants.h"
#include <QDebug>
extern QString EAstring;
const QByteArray VCDUFileReader::PREAMBLE_VCDU = QByteArray::fromHex("1ACFFC1D");

QByteArray VCDUFileReader::PREAMBLE_EA = QByteArray::fromHex("EA0003B4000000AF600000000384111E");

void VCDUFileReader::setCustomPreamble(const QByteArray &preamble)
{
    if (preamble.isEmpty()) {
        m_useCustomPreamble = false;
        m_customPreamble.clear();
    } else {
        m_customPreamble = preamble;
        m_useCustomPreamble = true;
    }
}

static quint16 crc16_ccitt_false_table[256];
static bool crc_table_initialized = false;

static void initCRC16_CCITT_FALSE_Table()
{
    if (crc_table_initialized) return;
    const quint16 polynomial = 0x1021;
    for (int i = 0; i < 256; i++) {
        quint16 crc = (quint16)i << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
        crc16_ccitt_false_table[i] = crc;
    }
    crc_table_initialized = true;
}

VCDUFileReader::VCDUFileReader(QObject *parent) : QObject(parent) {}
VCDUFileReader::~VCDUFileReader() { close(); }

bool VCDUFileReader::open(const QString &filePath, quint16 expectedVC0, ScanStrategy strategy)
{
    close();
    m_filePath = filePath;
    m_expectedVC0 = expectedVC0;
    m_strategy = strategy;
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return false;
    }
    m_isOpen = true;
    return true;
}

void VCDUFileReader::close()
{
    if (m_file.isOpen())
        m_file.close();
    m_records.clear();
    m_frameErrors.clear();
    m_crcErrors.clear();
    m_isOpen = false;
}

bool VCDUFileReader::verifyCRCDirect(const uchar* data) const
{
    initCRC16_CCITT_FALSE_Table();

    quint16 crc = 0xFFFF;
    for (int i = 0; i < VCDU::CRC_OFFSET; i++) {
        quint8 byte = data[i];
        crc = (crc << 8) ^ crc16_ccitt_false_table[((crc >> 8) ^ byte) & 0xFF];
    }
    quint16 storedCRC = (data[VCDU::CRC_OFFSET] << 8) | data[VCDU::CRC_OFFSET + 1];
    return crc == storedCRC;
}

void VCDUFileReader::scanFile()
{
    if (!m_isOpen) return;
    const qint64 totalSize = m_file.size();
    qDebug() << "========== 开始扫描文件 ==========";
    qDebug() << "文件路径:" << m_filePath;
    qDebug() << "文件大小:" << totalSize << "字节";
    qDebug() << "期望 VC0: 0x" << QString::number(m_expectedVC0, 16).toUpper();
    qDebug() << "扫描策略:" << (m_strategy == StrategyVCDU ? "VCDU (4字节前导)" : "EA (16字节前导)");

    m_records.clear();
    m_frameErrors.clear();
    m_crcErrors.clear();

    // ===== 根据策略确定参数 =====
    QByteArray preamble;
    int preambleSize;
    int vc0Offset;
    int recordStartOffset;
    int frameInterval;
    int preambleSkipSize;

    if (m_strategy == StrategyVCDU) {
        preamble = PREAMBLE_VCDU;
        preambleSize = 4;
        vc0Offset = 4;
        recordStartOffset = 4;
        frameInterval = 4 + VCDU::RECORD_SIZE;
        preambleSkipSize = 1;
    } else {
        preamble = PREAMBLE_EA;
        preambleSize = 16;
        vc0Offset = 56;
        recordStartOffset = 56;
        frameInterval =  56+VCDU::RECORD_SIZE;//56
        preambleSkipSize = 58;
    }

    // ===== 使用内存映射 =====
    uchar *mappedData = m_file.map(0, totalSize);
    if (!mappedData) {
        qDebug() << "内存映射失败";
        emit progressUpdated(100);
        return;
    }

    qDebug() << "内存映射成功，大小:" << totalSize << "字节";

    qint64 searchPos = 0;
    int foundCount = 0;

    // ===== 直接扫描映射内存 =====
    while (searchPos + vc0Offset + VCDU::RECORD_SIZE <= totalSize) {
        // if(foundCount == 21373)
        // {
        //     qDebug()<<"开始查找21373帧数据";
        // }
        // 比较前导码（使用 memcmp 避免拷贝）
        if (memcmp(mappedData + searchPos, preamble.constData(), preambleSize) == 0) {
            qint64 vc0Pos = searchPos + vc0Offset;
            
            // qDebug()<<"vc0Pos:"<<vc0Pos;
            quint16 vc0 = (mappedData[vc0Pos] << 8) | mappedData[vc0Pos + 1];
            // if(foundCount == 21373)
            // {
            //     qDebug()<<"vc0Pos:"<<vc0;
            // }
            if (vc0 == m_expectedVC0) {
                // if(foundCount == 21373){
                //     qDebug() << "找到记录: 偏移=0x" << QString::number(searchPos, 16).toUpper() << "  VC0=0x" << QString::number(vc0, 16).toUpper();
                // }
                
                qint64 recordStart = searchPos + recordStartOffset;

                // ===== 直接从映射内存读取数据，不创建 QByteArray =====
                // 只记录偏移，不拷贝数据
                RecordInfo info;
                info.fileOffset = recordStart;
                // qDebug()<<(mappedData[recordStart + VCDU::FRAME_OFFSET] << 16) |(mappedData[recordStart + VCDU::FRAME_OFFSET + 1] << 8) | mappedData[recordStart + VCDU::FRAME_OFFSET + 2];
                // 读取帧计数（直接从映射内存）
                quint32 frameCount = (mappedData[recordStart + VCDU::FRAME_OFFSET] << 16) |
                                     (mappedData[recordStart + VCDU::FRAME_OFFSET + 1] << 8) |
                                     mappedData[recordStart + VCDU::FRAME_OFFSET + 2];
                info.frameCount = frameCount;
                
                // qDebug()<<"帧计数:"<<frameCount;
                // CRC 校验（直接从映射内存）
                bool crcValid = verifyCRCDirect(mappedData + recordStart);
                info.crcValid = crcValid;

                m_records.append(info);
                // if (!crcValid)
                //     m_crcErrors.append(m_records.size() - 1);

                foundCount++;
                // if(foundCount == 21374)
                // {
                //     qDebug()<<"21374";
                // }
                // if (foundCount <= 10 || foundCount % 1000 == 0) {
                //     qDebug() << QString("  [%1] 找到记录: 偏移=0x%2  帧计数=%3  CRC有效=%4")
                //                 .arg(foundCount)
                //                 .arg(info.fileOffset, 8, 16, QChar('0'))
                //                 .arg(info.frameCount)
                //                 .arg(info.crcValid ? "是" : "否");
                // }

                // ===== 跳转到下一帧 =====
                qint64 nextPos = searchPos + frameInterval;
            //     if(foundCount == 21373){
            //     qDebug()<<"nextPos:"<<nextPos;
            //     const uchar* dataAfter = mappedData + nextPos ;
            //     int remain = totalSize - (nextPos );
            //     int dumpLen = qMin(remain, 16); // 输出前 32 字节，避免刷屏
        
            //     // 关键：reinterpret_cast 转为 const char*，以便 QByteArray 接受
            //     QByteArray after(reinterpret_cast<const char*>(dataAfter), dumpLen);
            //     qDebug() << "后续 32 字节 (hex):" << after.toHex();
            
            // }
                bool jumpSuccess = false;
                if (nextPos + preambleSize < totalSize &&
                    memcmp(mappedData + nextPos, preamble.constData(), preambleSize) == 0) {
                        // if(foundCount == 21373){
                        //     qDebug()<<"跳转成功已顺利匹配21373帧下次进入while后查找21373帧";
                        // }
                        
                    jumpSuccess = true;
                }

                if (jumpSuccess) {
                    searchPos = nextPos;
                } else {
                    searchPos += 1;
                }
            } else {
                searchPos += preambleSkipSize;
            }
        } else {
            searchPos += 1;
        }

        // 进度更新
        if (searchPos % (1024 * 1024) == 0) {
            int percent = (int)((searchPos * 100) / totalSize);
            emit progressUpdated(percent);
        }
        // if(foundCount == 21373)
        // {
        //     qDebug()<<"进度:"<<searchPos + preambleSize + vc0Offset + VCDU::RECORD_SIZE<<totalSize;
        // }
       
    }

    m_file.unmap(mappedData);

    // ===== 帧连续性检查 =====
    if (m_records.size() > 1) {
        for (int i = 1; i < m_records.size(); ++i) {
            quint32 expected = m_records[i-1].frameCount + 1;
            if (m_records[i].frameCount != expected)
                m_frameErrors.append({i, expected, m_records[i].frameCount});
        }
    }
    emit progressUpdated(100);

    qDebug() << "========== 扫描完成 ==========";
    qDebug() << "总共找到记录数:" << m_records.size();
    // qDebug() << "CRC 错误记录数:" << m_crcErrors.size();
    qDebug() << "帧不连续错误数:" << m_frameErrors.size();
    qDebug() << "=====================================";
}

// void VCDUFileReader::scanFile()
// {
//     if (!m_isOpen) return;
//     const qint64 totalSize = m_file.size();
//     qDebug() << "========== 开始扫描文件 ==========";
//     qDebug() << "文件路径:" << m_filePath;
//     qDebug() << "文件大小:" << totalSize << "字节";
//     qDebug() << "期望 VC0: 0x" << QString::number(m_expectedVC0, 16).toUpper();
//     qDebug() << "扫描策略:" << (m_strategy == StrategyVCDU ? "VCDU" : "EA");

//     m_records.clear();
//     m_frameErrors.clear();
//     m_crcErrors.clear();

//     // ===== 根据策略确定参数 =====
//     QByteArray preamble;
//     int preambleSize;
//     int vc0Offset;
//     int recordStartOffset;
//     int frameInterval;
//     int preambleSkipSize;

//     if (m_strategy == StrategyVCDU) {
//         // VCDU: 搜索 1A CF FC 1D，VC0 在偏移4
//         preamble = PREAMBLE_VCDU;           // 1A CF FC 1D
//         preambleSize = 4;
//         vc0Offset = 4;
//         recordStartOffset = 4;
//         frameInterval = 4 + VCDU::RECORD_SIZE;  // 896
//         preambleSkipSize = 1;  // VC0不匹配时跳过1字节
//     } else {
//         // EA: 搜索完整前导 EA 00 03 B4 00 00 00 AF 60 00 00 00 03 84 11 1F
//         // VC0 在前导后 40 字节处，即偏移 56
//         preamble = PREAMBLE_EA;              // EA 00 03 B4 00 00 00 AF 60 00 00 00 03 84 11 1F
//         preambleSize = 16;
//         vc0Offset = 56;                      // 16 + 40 = 56
//         recordStartOffset = 56;              // 从 VC0 开始截取
//         frameInterval = 56 + VCDU::RECORD_SIZE; // 948
//         preambleSkipSize = 58;  // VC0不匹配时跳过前导16字节 + 2字节VC0 = 58
//     }

//     const qint64 THRESHOLD = 1024 * 1024 * 1024; // 1GB
//     if (totalSize <= THRESHOLD) {
//         // ===== 小文件：一次性读取 =====
//         QByteArray data = m_file.readAll();
//         int searchPos = 0;
//         int foundCount = 0;

//         while (searchPos <= data.size() - (preambleSize + vc0Offset + VCDU::RECORD_SIZE)) {
//             if (data.mid(searchPos, preambleSize) == preamble) {
//                 int vc0Pos = searchPos + vc0Offset;
//                 quint16 vc0 = (quint8(data[vc0Pos]) << 8) | (quint8(data[vc0Pos + 1]));
//                 if (vc0 == m_expectedVC0) {
//                     int recordStart = searchPos + recordStartOffset;
//                     if (recordStart + VCDU::RECORD_SIZE <= data.size()) {
//                         QByteArray raw = data.mid(recordStart, VCDU::RECORD_SIZE);
//                         RecordInfo info;
//                         info.fileOffset = recordStart;
//                         info.frameCount = readFrameCount(raw);
//                         info.crcValid = verifyCRC(raw);
//                         m_records.append(info);
//                         if (!info.crcValid)
//                             m_crcErrors.append(m_records.size() - 1);

//                         foundCount++;
//                         // if (foundCount <= 10 || foundCount % 1000 == 0) {
//                         //     qDebug() << QString("  [%1] 找到记录: 偏移=0x%2  帧计数=%3  CRC有效=%4")
//                         //                 .arg(foundCount)
//                         //                 .arg(info.fileOffset, 8, 16, QChar('0'))
//                         //                 .arg(info.frameCount)
//                         //                 .arg(info.crcValid ? "是" : "否");
//                         // }

//                         // ===== 尝试跳转到下一帧 =====
//                         qint64 nextPos = searchPos + frameInterval;
//                         bool jumpSuccess = false;
//                         if (nextPos + preambleSize <= data.size() &&
//                             data.mid(nextPos, preambleSize) == preamble) {
//                             jumpSuccess = true;
//                         }

//                         if (jumpSuccess) {
//                             searchPos = nextPos;
//                         } else {
//                             searchPos += 1;
//                         }
//                     } else {
//                         searchPos += 1;
//                     }
//                 } else {
//                     // ===== VC0 不匹配：跳过前导 =====
//                     searchPos += preambleSkipSize;
//                 }
//             } else {
//                 searchPos += 1;
//             }
//         }
//     } else {
//         // ===== 大文件：分块读取 =====
//         const qint64 BUFFER_SIZE = 8 * 1024 * 1024; // 8MB
//         QByteArray buffer;
//         qint64 filePos = 0;
//         qint64 readTotal = 0;
//         int foundCount = 0;
//         const int OVERLAP = 4096;

//         while (!m_file.atEnd()) {
//             QByteArray newData = m_file.read(BUFFER_SIZE);
//             if (newData.isEmpty()) break;
//             buffer.append(newData);
//             readTotal += newData.size();

//             int searchPos = 0;
//             int bufferSize = buffer.size();

//             while (searchPos + preambleSize + vc0Offset + VCDU::RECORD_SIZE <= bufferSize) {
//                 if (buffer.mid(searchPos, preambleSize) == preamble) {
//                     int vc0Pos = searchPos + vc0Offset;
//                     quint16 vc0 = (quint8(buffer[vc0Pos]) << 8) | (quint8(buffer[vc0Pos + 1]));
//                     // ===== 在记录 8840 附近打印调试信息 =====
//                     if (foundCount >= 8830 && foundCount <= 8850) {
//                         qDebug() << "========================================";
//                         qDebug() << "=== 调试信息 (foundCount=" << foundCount << ") ===";
//                         qDebug() << "searchPos: 0x" << QString::number(searchPos, 16);
//                         qDebug() << "filePos: 0x" << QString::number(filePos, 16);
//                         qDebug() << "绝对偏移: 0x" << QString::number(filePos + searchPos, 16);
//                         qDebug() << "vc0Pos: 0x" << QString::number(vc0Pos, 16);
//                         qDebug() << "vc0: 0x" << QString::number(vc0, 16).toUpper();
//                         qDebug() << "期望 VC0: 0x" << QString::number(m_expectedVC0, 16).toUpper();
//                         qDebug() << "前导码: " << buffer.mid(searchPos, preambleSize).toHex(' ');
//                     }
                    
//                     if (vc0 == m_expectedVC0) {
//                         int recordStart = searchPos + recordStartOffset;
                
//                         // ===== 在记录 8840 附近打印记录起始信息 =====
//                         if (foundCount >= 8830 && foundCount <= 8850) {
//                             qDebug() << "recordStart: 0x" << QString::number(recordStart, 16);
//                             qDebug() << "记录前16字节: " << buffer.mid(recordStart, 16).toHex(' ');
//                             qDebug() << "========================================";
//                         }
//                         if (recordStart + VCDU::RECORD_SIZE <= bufferSize) {
//                             QByteArray raw = buffer.mid(recordStart, VCDU::RECORD_SIZE);
//                             RecordInfo info;
//                             info.fileOffset = filePos + recordStart;
//                             info.frameCount = readFrameCount(raw);
//                             info.crcValid = verifyCRC(raw);
//                             m_records.append(info);
//                             if (!info.crcValid)
//                                 m_crcErrors.append(m_records.size() - 1);

//                             foundCount++;
//                             // if (foundCount <= 10 || foundCount % 1000 == 0) {
//                             //     qDebug() << QString("  [%1] 找到记录: 偏移=0x%2  帧计数=%3  CRC有效=%4")
//                             //                 .arg(foundCount)
//                             //                 .arg(info.fileOffset, 8, 16, QChar('0'))
//                             //                 .arg(info.frameCount)
//                             //                 .arg(info.crcValid ? "是" : "否");
//                             // }

//                             // ===== 尝试跳转到下一帧 =====
//                             qint64 nextPos = searchPos + frameInterval;
//                             bool jumpSuccess = false;
//                             if (nextPos + preambleSize <= bufferSize &&
//                                 buffer.mid(nextPos, preambleSize) == preamble) {
//                                 jumpSuccess = true;
//                             }

//                             if (jumpSuccess) {
//                                 searchPos = nextPos;
//                             } else {
//                                 searchPos += 1;
//                             }
//                         } else {
//                             searchPos += 1;
//                         }
//                     } else {
//                         // ===== VC0 不匹配：跳过前导 =====
//                         searchPos += preambleSkipSize;
//                     }
//                 } else {
//                     searchPos += 1;
//                 }
//             }

//             // 保留重叠数据（防止跨块丢失帧头）
//             int keepSize = qMin(OVERLAP, buffer.size());
//             buffer = buffer.right(keepSize);
//             filePos += newData.size();

//             int percent = (int)((readTotal * 100) / totalSize);
//             if (percent % 5 == 0 || percent == 100) {
//                 emit progressUpdated(percent);
//             }
//         }

//         // ===== 处理最终缓冲区中可能残留的完整记录 =====
//         int searchPos = 0;
//         int bufferSize = buffer.size();

//         while (searchPos + preambleSize + vc0Offset + VCDU::RECORD_SIZE <= bufferSize) {
//             if (buffer.mid(searchPos, preambleSize) == preamble) {
//                 int vc0Pos = searchPos + vc0Offset;
//                 quint16 vc0 = (quint8(buffer[vc0Pos]) << 8) | (quint8(buffer[vc0Pos + 1]));
//                 if (vc0 == m_expectedVC0) {
//                     int recordStart = searchPos + recordStartOffset;
//                     if (recordStart + VCDU::RECORD_SIZE <= bufferSize) {
//                         QByteArray raw = buffer.mid(recordStart, VCDU::RECORD_SIZE);
//                         RecordInfo info;
//                         info.fileOffset = filePos + recordStart;
//                         info.frameCount = readFrameCount(raw);
//                         info.crcValid = verifyCRC(raw);
//                         m_records.append(info);
//                         if (!info.crcValid)
//                             m_crcErrors.append(m_records.size() - 1);

//                         foundCount++;
//                         // if (foundCount <= 10 || foundCount % 1000 == 0) {
//                         //     qDebug() << QString("  [%1] 找到记录: 偏移=0x%2  帧计数=%3  CRC有效=%4")
//                         //                 .arg(foundCount)
//                         //                 .arg(info.fileOffset, 8, 16, QChar('0'))
//                         //                 .arg(info.frameCount)
//                         //                 .arg(info.crcValid ? "是" : "否");
//                         // }

//                         // ===== 尝试跳转到下一帧 =====
//                         qint64 nextPos = searchPos + frameInterval;
//                         bool jumpSuccess = false;
//                         if (nextPos + preambleSize <= bufferSize &&
//                             buffer.mid(nextPos, preambleSize) == preamble) {
//                             jumpSuccess = true;
//                         }

//                         if (jumpSuccess) {
//                             searchPos = nextPos;
//                         } else {
//                             searchPos += 1;
//                         }
//                     } else {
//                         searchPos += 1;
//                     }
//                 } else {
//                     // ===== VC0 不匹配：跳过前导 =====
//                     searchPos += preambleSkipSize;
//                 }
//             } else {
//                 searchPos += 1;
//             }
//         }
//     }

//     // ===== 清理不完整的记录 =====
//     int removedCount = 0;
//     while (!m_records.isEmpty()) {
//         qint64 lastOffset = m_records.last().fileOffset;
//         if (lastOffset + VCDU::RECORD_SIZE <= m_file.size()) {
//             break;
//         }
//         qDebug() << "移除不完整记录: 索引=" << (m_records.size() - 1)
//                  << " 偏移=0x" << QString::number(lastOffset, 16);
//         m_records.removeLast();
//         removedCount++;
//         int lastIdx = m_records.size();
//         for (int i = 0; i < m_crcErrors.size(); ++i) {
//             if (m_crcErrors[i] == lastIdx) {
//                 m_crcErrors.removeAt(i);
//                 break;
//             }
//         }
//     }
//     if (removedCount > 0) {
//         qDebug() << "已移除" << removedCount << "条不完整记录";
//     }

//     // ===== 帧连续性检查 =====
//     if (m_records.size() > 1) {
//         for (int i = 1; i < m_records.size(); ++i) {
//             quint32 expected = m_records[i-1].frameCount + 1;
//             if (m_records[i].frameCount != expected)
//                 m_frameErrors.append({i, expected, m_records[i].frameCount});
//         }
//     }
//     emit progressUpdated(100);

//     qDebug() << "========== 扫描完成 ==========";
//     qDebug() << "总共找到记录数:" << m_records.size();
//     qDebug() << "CRC 错误记录数:" << m_crcErrors.size();
//     qDebug() << "帧不连续错误数:" << m_frameErrors.size();
//     qDebug() << "=====================================";
// }
bool VCDUFileReader::openForRead(const QString &filePath)
{
    // 如果已经打开了文件，先关闭（但不清空记录）
    if (m_file.isOpen())
        m_file.close();
    m_filePath = filePath;
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件用于读取:" << filePath;
        return false;
    }
    m_isOpen = true;
    return true;
}

quint32 VCDUFileReader::readFrameCount(const QByteArray &raw) const
{
    return (quint8(raw[VCDU::FRAME_OFFSET]) << 16) |
           (quint8(raw[VCDU::FRAME_OFFSET+1]) << 8) |
           (quint8(raw[VCDU::FRAME_OFFSET+2]));
}

bool VCDUFileReader::verifyCRC(const QByteArray &raw) const
{
    initCRC16_CCITT_FALSE_Table();
    int start = 0;
    int end = VCDU::CRC_OFFSET - 1;
    quint16 crc = 0xFFFF;
    for (int i = start; i <= end; i++) {
        quint8 byte = static_cast<quint8>(raw[i]);
        crc = (crc << 8) ^ crc16_ccitt_false_table[((crc >> 8) ^ byte) & 0xFF];
    }
    quint16 storedCRC = (static_cast<quint8>(raw[VCDU::CRC_OFFSET]) << 8) |
                        (static_cast<quint8>(raw[VCDU::CRC_OFFSET+1]));
    return crc == storedCRC;
}

QByteArray VCDUFileReader::readRawRecord(int index) const
{
    if (!m_isOpen) {
        qDebug() << "readRawRecord: 文件未打开";
        return QByteArray();
    }
    if (index < 0 || index >= m_records.size()) {
        qDebug() << "readRawRecord: 索引越界" << index << "记录总数" << m_records.size();
        return QByteArray();
    }
    qint64 offset = m_records[index].fileOffset;
    if (offset + VCDU::RECORD_SIZE > m_file.size()) {
        qWarning() << "readRawRecord: 记录" << index << "超出文件范围，偏移" << offset;
        return QByteArray();
    }
    if (!m_file.seek(offset)) {
        qWarning() << "readRawRecord: seek 失败，偏移" << offset;
        return QByteArray();
    }
    QByteArray data = m_file.read(VCDU::RECORD_SIZE);
    if (data.size() != VCDU::RECORD_SIZE) {
        qWarning() << "readRawRecord: 记录" << index << "长度不足，期望" << VCDU::RECORD_SIZE << "实际" << data.size();
    }
    return data;
}

void VCDUFileReader::setResults(const QVector<RecordInfo> &records,
                                const QVector<FrameError> &frameErrors,
                                const QVector<int> &crcErrors)
{
    m_records = records;
    m_frameErrors = frameErrors;
    m_crcErrors = crcErrors;
}