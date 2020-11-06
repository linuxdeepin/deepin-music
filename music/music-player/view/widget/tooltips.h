#ifndef TOOLTIPS_H
#define TOOLTIPS_H
#include <QFrame>
#include <QGuiApplication>
#include <DPalette>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <QApplication>
#include <QDesktopWidget>

class ToolTipsPrivate;
class ToolTips : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int radius READ radius WRITE setRadius)
    //Q_PROPERTY(QBrush background READ background WRITE setBackground)
    //Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
public:
    explicit ToolTips(const QString &text,
                      QWidget *parent = nullptr);
    ~ToolTips() override;

    //void pop(QPoint center);

    int radius() const;
    QColor borderColor() const;
    QBrush background() const;

public slots:
    void setText(const QString text);
    //void setBackground(QBrush background);
    void setRadius(int radius);
    //void setBorderColor(QColor borderColor);

protected:
    virtual void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) override;
    virtual void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;

public:
    //void resetSize(const int maxWidth);

private:
    QScopedPointer<ToolTipsPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ToolTips)
    QString m_strText;
};


#endif // TOOLTIPS_H
