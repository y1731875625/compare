#ifndef DIFFENGINE_H
#define DIFFENGINE_H

#include <QVector>
#include <functional>
#include <QString>
#include "VCDUFileReader.h"

struct DataDiff {
    int recordIndex;
    int byteOffsetInRecord;
    unsigned char valueA;
    unsigned char valueB;
};

struct FrameMatchError {
    enum Type {
        MissingInA,
        MissingInB,
        CountMismatch
    };
    Type type;
    int recordIndex;
    quint32 frameA;
    quint32 frameB;
};

class DiffEngine
{
public:
    // 生成差异文件，返回差异总数
    static qint64 generateDiffFile(const VCDUFileReader &readerA,
                                   const VCDUFileReader &readerB,
                                   const QString &diffFilePath,
                                   std::function<void(int)> progressCallback = nullptr);

    // 从差异文件读取指定范围的差异
    static QVector<DataDiff> loadDiffRange(const QString &diffFilePath,
                                           qint64 startIndex,
                                           int count);

    // 获取差异文件中的总差异数
    static qint64 getDiffCount(const QString &diffFilePath);

    static QVector<FrameMatchError> compareFrameSequences(const VCDUFileReader &readerA,
                                                          const VCDUFileReader &readerB);
};

#endif // DIFFENGINE_H