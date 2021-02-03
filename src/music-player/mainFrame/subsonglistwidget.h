#include <DDialog>
#include <DFrame>
#include <mediameta.h>

DWIDGET_USE_NAMESPACE

class InfoDialog;
class MusicImageButton;
class MusicTitleImageWidget;
class PlayListView;

#include <DLabel>
#include <DPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <DIconButton>

class SubSonglistWidget : public DFrame
{
    Q_OBJECT

public:
    explicit SubSonglistWidget(const QString &hash, QWidget *parent = Q_NULLPTR);
    ~SubSonglistWidget();

    void setThemeType(int type);

    // 更新Dialog UI信息
    void flushDialog(QMap<QString, MediaMeta> musicinfos, ListPageSwitchType listPageType);
public slots:
    void slotPlayAllClicked();
    void slotPlayRandomClicked();

private:
    void setTitleImage(QPixmap &img);
    void initUI();
protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
private:
    // 当前主题
    int m_themeType = 1;
    // 专辑或则演唱者的hash
    QString                             m_hash;

    QLabel                             *m_titleImage          = nullptr;
    DLabel                             *m_titleLabel          = nullptr;
    DLabel                             *m_infoLabel           = nullptr;
    DPushButton                        *m_btPlayAll           = nullptr;
    DPushButton                        *m_btRandomPlay        = nullptr;
    InfoDialog                         *m_infoDialog          = nullptr;
    PlayListView                       *m_musicListInfoView   = nullptr;
    QPixmap                             m_img;
};
