#include "DiffEngine.h"
#include "RecordConstants.h"
#define XXH_INLINE_ALL
#include "xxhash.h"          // 请将 xxhash.h 放入项目并包含
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QCoreApplication>

qint64 DiffEngine::generateDiffFile(const VCDUFileReader &readerA,
                                    const VCDUFileReader &readerB,
                                    const QString &diffFilePath,
                                    std::function<void(int)> progressCallback)
{
    int countA = readerA.recordCount();
    int countB = readerB.recordCount();
    int count = qMin(countA, countB);

    if (count == 0) return 0;

    QFile file(diffFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法创建差异文件:" << diffFilePath;
        return 0;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    qint64 totalDiffs = 0;
    const int DATA_SIZE = VCDU::DATA_SIZE;  // 852
    const int DATA_OFFSET = VCDU::DATA_OFFSET; // 38

    for (int i = 0; i < count; ++i) {
        QByteArray rawA = readerA.readRawRecord(i);
        QByteArray rawB = readerB.readRawRecord(i);

        if (rawA.size() < VCDU::RECORD_SIZE || rawB.size() < VCDU::RECORD_SIZE) {
            continue;
        }

        // 提取数据区指针
        const char* dataA = rawA.constData() + DATA_OFFSET;
        const char* dataB = rawB.constData() + DATA_OFFSET;

        // ========== 哈希快速比较 ==========
        // 使用 xxHash64 计算数据区哈希
        XXH64_hash_t hashA = XXH64(dataA, DATA_SIZE, 0);
        XXH64_hash_t hashB = XXH64(dataB, DATA_SIZE, 0);

        if (hashA != hashB) {
            // 哈希不同，必然存在差异，逐字节定位
            for (int offset = DATA_OFFSET; offset < DATA_OFFSET + DATA_SIZE; ++offset) {
                if (rawA[offset] != rawB[offset]) {
                    DataDiff diff;
                    diff.recordIndex = i;
                    diff.byteOffsetInRecord = offset;
                    diff.valueA = (unsigned char)rawA[offset];
                    diff.valueB = (unsigned char)rawB[offset];
                    out << diff.recordIndex << diff.byteOffsetInRecord << diff.valueA << diff.valueB;
                    totalDiffs++;
                }
            }
        } else {
            // 哈希相同，极大概率相同，但我们仍用 memcmp 做最终确认（防御碰撞）
            if (memcmp(dataA, dataB, DATA_SIZE) != 0) {
                // 虽然哈希相同但数据不同（极小概率），此时仍需逐字节定位
                for (int offset = DATA_OFFSET; offset < DATA_OFFSET + DATA_SIZE; ++offset) {
                    if (rawA[offset] != rawB[offset]) {
                        DataDiff diff;
                        diff.recordIndex = i;
                        diff.byteOffsetInRecord = offset;
                        diff.valueA = (unsigned char)rawA[offset];
                        diff.valueB = (unsigned char)rawB[offset];
                        out << diff.recordIndex << diff.byteOffsetInRecord << diff.valueA << diff.valueB;
                        totalDiffs++;
                    }
                }
            }
            // 否则完全相同，跳过
        }

        if (progressCallback && (i % 100 == 0 || i == count - 1)) {
            int percent = (i * 100) / count;
            progressCallback(percent);
            QCoreApplication::processEvents();
        }
    }

    file.close();
    return totalDiffs;
}

// loadDiffRange 和 getDiffCount 保持不变
QVector<DataDiff> DiffEngine::loadDiffRange(const QString &diffFilePath,
                                            qint64 startIndex,
                                            int count)
{
    QVector<DataDiff> diffs;
    diffs.reserve(count);

    QFile file(diffFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开差异文件:" << diffFilePath;
        return diffs;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    qint64 currentIndex = 0;
    while (currentIndex < startIndex && !in.atEnd()) {
        DataDiff dummy;
        in >> dummy.recordIndex >> dummy.byteOffsetInRecord >> dummy.valueA >> dummy.valueB;
        currentIndex++;
    }

    while (diffs.size() < count && !in.atEnd()) {
        DataDiff diff;
        in >> diff.recordIndex >> diff.byteOffsetInRecord >> diff.valueA >> diff.valueB;
        diffs.append(diff);
    }

    file.close();
    return diffs;
}

qint64 DiffEngine::getDiffCount(const QString &diffFilePath)
{
    QFile file(diffFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    // 更好的方法是遍历计数，但为了快速估算，用文件大小除以每条记录的长度
    // 每条差异固定为：int + int + unsigned char + unsigned char = 4+4+1+1 = 10 字节
    // 但 QDataStream 会额外添加开销，所以实际略大，这里粗略估算
    qint64 size = file.size();
    file.close();
    // 每条差异约 12 字节（包括流标识）
    return size / 12;
}

QVector<FrameMatchError> DiffEngine::compareFrameSequences(const VCDUFileReader &readerA,
                                                            const VCDUFileReader &readerB)
{
    QVector<FrameMatchError> errors;
    int countA = readerA.recordCount();
    int countB = readerB.recordCount();

    if (countA != countB) {
        FrameMatchError err;
        err.type = FrameMatchError::CountMismatch;
        err.recordIndex = -1;
        err.frameA = countA;
        err.frameB = countB;
        errors.append(err);
    }

    int minCount = qMin(countA, countB);
    for (int i = 0; i < minCount; ++i) {
        quint32 frameA = readerA.records()[i].frameCount;
        quint32 frameB = readerB.records()[i].frameCount;
        if (frameA != frameB) {
            FrameMatchError err;
            err.recordIndex = i;
            err.frameA = frameA;
            err.frameB = frameB;
            if (frameA < frameB) {
                err.type = FrameMatchError::MissingInA;
            } else {
                err.type = FrameMatchError::MissingInB;
            }
            errors.append(err);
        }
    }

    if (countA > countB) {
        for (int i = minCount; i < countA; ++i) {
            FrameMatchError err;
            err.type = FrameMatchError::MissingInB;
            err.recordIndex = i;
            err.frameA = readerA.records()[i].frameCount;
            err.frameB = 0;
            errors.append(err);
        }
    } else if (countB > countA) {
        for (int i = minCount; i < countB; ++i) {
            FrameMatchError err;
            err.type = FrameMatchError::MissingInA;
            err.recordIndex = i;
            err.frameA = 0;
            err.frameB = readerB.records()[i].frameCount;
            errors.append(err);
        }
    }

    return errors;
}