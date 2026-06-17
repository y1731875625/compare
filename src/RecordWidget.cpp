#include "RecordWidget.h"
#include "RecordConstants.h"
#include <QTextBlock>

RecordWidget::RecordWidget(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setFont(QFont("Courier New", 10));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);  // 水平滚动条
    setLineWrapMode(QPlainTextEdit::NoWrap);              // 不换行
}

void RecordWidget::setRecord(const QByteArray &rawRecord, bool /*showHeader*/)
{
    // qDebug() << "RecordWidget::setRecord";
    m_currentRaw = rawRecord;
    buildDisplayText(rawRecord);
    m_extraSelections.clear();
    setExtraSelections(m_extraSelections);
}

void RecordWidget::clear()
{
    QPlainTextEdit::clear();
    m_currentRaw.clear();
    m_extraSelections.clear();
    setExtraSelections(m_extraSelections);
}

void RecordWidget::buildDisplayText(const QByteArray &raw)
{
    if (raw.isEmpty()) {
        setPlainText("空记录 (长度 0)");
        return;
    }

    QString text;
    if (raw.size() < VCDU::RECORD_SIZE) {
        text += QString("警告: 记录长度不足 %1 字节 (实际 %2 字节)\n")
                    .arg(VCDU::RECORD_SIZE).arg(raw.size());
    }

    // 单行显示：所有字节在一行
    for (int i = 0; i < raw.size(); ++i) {
        text += QString("%1 ").arg((quint8)raw[i], 2, 16, QChar('0'));
        // qDebug() << "raw[" << i << "] = " << (quint8)raw[i];
    }
    setPlainText(text);
    // setPlainText("是否执行到此");
}

void RecordWidget::highlightByteRange(int startOffset, int length, const QColor &color)
{
    // 单行模式：计算起始列和结束列
    QString prefix = "警告: ...\n"; // 如果有警告，需考虑前缀行
    // 简单起见，我们假设没有警告行，或者忽略警告行
    // 更好的做法：存储当前显示的原始数据，并计算前缀长度
    // 但我们采用固定前缀：如果存在警告，第一行是警告，第二行是数据
    // 实际实现应检查文本内容
    // 为简化，我们假设数据从第一行开始（无警告）
    int prefixLen = 0; // 可根据实际情况动态获取
    // 若存在警告行，需要跳过第一行
    if (toPlainText().startsWith("警告")) {
        prefixLen = toPlainText().indexOf('\n') + 1;
    }
    int startCol = prefixLen + startOffset * 3;
    int endCol = prefixLen + (startOffset + length) * 3;

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, startCol);
    QTextCursor endCursor(cursor);
    endCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, endCol - startCol);

    QTextEdit::ExtraSelection sel;
    sel.cursor = endCursor;
    sel.format.setBackground(color);
    m_extraSelections.append(sel);
    setExtraSelections(m_extraSelections);
}

void RecordWidget::jumpToByteOffset(int byteOffset)
{
    int prefixLen = 0;
    if (toPlainText().startsWith("警告")) {
        prefixLen = toPlainText().indexOf('\n') + 1;
    }
    int col = prefixLen + byteOffset * 3;
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, col);
    setTextCursor(cursor);
    ensureCursorVisible();
}