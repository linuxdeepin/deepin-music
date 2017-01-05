#ifndef SHORTCUTEDITOR_H
#define SHORTCUTEDITOR_H

#include <QWidget>

class ShortcutEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ShortcutEditor(const QString &group, const QString &key, QWidget *parent = 0);
    void forceUpdate();

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    QSize stringSize(const QString &str);
    QRect drawTextRect(const QRect &lastRect, const QString &str, QPainter &painter);
    QString defaultValue();
    void updateValue();

private:
    int m_borderWidth;
    bool m_canSet;

    QColor m_borderColor;
    QString m_key;
    QString m_shortcut;
};

#endif // SHORTCUTEDITOR_H
