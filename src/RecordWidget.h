#ifndef RECORDWIDGET_H
#define RECORDWIDGET_H

#include <QPlainTextEdit>
#include <QVector>
#include <QColor>

class RecordWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit RecordWidget(QWidget *parent = nullptr);

    void setRecord(const QByteArray &rawRecord, bool showHeader = true);
    void clear();
    void highlightByteRange(int startOffset, int length, const QColor &color);
    void jumpToByteOffset(int byteOffset); // 滚动到指定字节偏移并选中

private:
    void buildDisplayText(const QByteArray &raw);
    QByteArray m_currentRaw;
    QVector<QTextEdit::ExtraSelection> m_extraSelections;
};

#endif // RECORDWIDGET_H