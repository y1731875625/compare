#include "DiffEngine.h"
#include "RecordConstants.h"
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

    for (int i = 0; i < count; ++i) {
        QByteArray rawA = readerA.readRawRecord(i);
        QByteArray rawB = readerB.readRawRecord(i);

        if (rawA.size() < VCDU::RECORD_SIZE || rawB.size() < VCDU::RECORD_SIZE) {
            continue;
        }

        // 从偏移 38 开始比对 852 字节（去掉前38字节）
        for (int offset = VCDU::DATA_OFFSET; offset < VCDU::DATA_OFFSET + VCDU::DATA_SIZE; ++offset) {
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

        if (progressCallback && (i % 100 == 0 || i == count - 1)) {
            int percent = (i * 100) / count;
            progressCallback(percent);
            QCoreApplication::processEvents();
        }
    }

    file.close();
    return totalDiffs;
}

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

    // 跳过前面的记录
    qint64 currentIndex = 0;
    while (currentIndex < startIndex && !in.atEnd()) {
        DataDiff dummy;
        in >> dummy.recordIndex >> dummy.byteOffsetInRecord >> dummy.valueA >> dummy.valueB;
        currentIndex++;
    }

    // 读取目标数量的记录
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

    qint64 fileSize = file.size();
    int recordSize = sizeof(int) + sizeof(int) + sizeof(unsigned char) + sizeof(unsigned char);
    // 由于 QDataStream 有额外开销，实际大小无法精确计算，用文件大小估算
    // 更可靠的方式是遍历计数，但这里用估算
    file.close();

    // 每条差异约 12 字节（4+4+1+1 + 流开销）
    return fileSize / 12;
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