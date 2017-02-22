#pragma once

#include <QScopedPointer>
#include <QListView>

class ListViewPrivate;
class ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = 0);
    ~ListView();

protected:
    void updateScrollbar();

    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<ListViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ListView)
};

