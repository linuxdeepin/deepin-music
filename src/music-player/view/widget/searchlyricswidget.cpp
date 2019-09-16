#include "searchlyricswidget.h"

#include <QVBoxLayout>

#include <DLabel>
#include <DLineEdit>
#include <DCheckBox>

#include <QDir>

#include "util/musiclyric.h"
#include <DPalette>
DGUI_USE_NAMESPACE

SearchLyricsWidget::SearchLyricsWidget(QString path, QWidget *parent)
    : DFloatingWidget(parent),
      m_search(new DPushButton),
      m_listWidget(new DListWidget),
      m_keyWord(new DLineEdit),
      m_singer(new DLineEdit),
      m_group(new QButtonGroup),
      m_path(path)
{
    initUI();
}

void SearchLyricsWidget::setDefault(QString song, QString singer)
{
    m_keyWord->lineEdit()->setText(song);
    m_singer->lineEdit()->setText(singer);
}

void SearchLyricsWidget::initUI()
{
    setFixedWidth(370);

    DPalette WidgetPl = palette();
    WidgetPl.setColor(DPalette::Background, QColor("#FFFFFF"));
    setPalette(WidgetPl);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(10, 10, 10, 6);
    DLabel *title = new DLabel;
    QFont font;
    font.setFamily("SourceHanSansSC-Bold");
    font.setPixelSize(17);
    title->setFont(font);
    title->setText(tr("Search Lyrics"));
    title->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    title->setFixedHeight(30);
    DPalette pa = title->palette();
    pa.setColor(DPalette::WindowText, "#001A2E");
    title->setPalette(pa);

    m_keyWord->setClearButtonEnabled(false);
    m_singer->setClearButtonEnabled(false);
    m_keyWord->lineEdit()->setPlaceholderText(tr("Song name"));
    m_singer->lineEdit()->setPlaceholderText(tr("Singer name"));
    m_keyWord->setFixedHeight(36);
    m_singer->setFixedHeight(36);
    m_search->setText(tr("Search"));
    m_search->setFixedHeight(36);

    DPalette pl = m_search->palette();
    pl.setColor(DPalette::ButtonText, QColor("#414D68"));
    pl.setColor(DPalette::Light, QColor("#E6E6E6"));
    pl.setColor(DPalette::Dark, QColor("#E3E3E3"));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0.08);
    pl.setColor(DPalette::Shadow, sbcolor);
    m_search->setPalette(pl);
    // m_search->setFlat(true);
    connect(m_search, &DPushButton::clicked, this, &SearchLyricsWidget::searchLyrics);

    layout->addWidget(title);
    layout->addSpacing(10);
    layout->addWidget(m_keyWord);
    layout->addSpacing(10);
    layout->addWidget(m_singer);
    layout->addSpacing(20);
    layout->addWidget(m_search);
    layout->addSpacing(30);
    layout->addWidget(m_listWidget);
    m_listWidget->setFrameShape(QListWidget::NoFrame);
    m_listWidget->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    connect(m_listWidget, &DListWidget::clicked, this, &SearchLyricsWidget::mySelection);
    connect(m_group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
    [ = ](int id) {
        mySelectionIndex(id);
    });
}

void SearchLyricsWidget::createList()
{
    for (int i = 0; i < m_lyricList.size(); ++i) {
        QListWidgetItem *listItem = new QListWidgetItem;
        listItem->setSizeHint(QSize(330, 64));
        m_listWidget->addItem(listItem);
        DFrame *frame = new DFrame;
        frame->setFixedSize(330, 64);

        // DPalette framepl = frame->palette();
        //framepl.setColor(DPalette::Window, DPalette::Window);
        //frame->setPalette(framepl);

        DLabel *lyricNmae = new DLabel(frame);
        DLabel *singerName = new DLabel(frame);
        DLabel *duration = new DLabel(frame);

        DPalette lyricNamepl = lyricNmae->palette();
        lyricNamepl.setColor(DPalette::WindowText, QColor("#414D68"));
        lyricNmae->setPalette(lyricNamepl);

        DPalette singerNamepl = singerName->palette();
        singerNamepl.setColor(DPalette::WindowText, QColor("#526A7F"));
        singerName->setPalette(singerNamepl);
        duration->setPalette(singerNamepl);

        lyricNmae->setText(m_lyricList[i].lyricsName);
        QFont font;
        font.setFamily("SourceHanSansSC-Medium");
        font.setPixelSize(14);
        lyricNmae->setFont(font);
        singerName->setText(m_lyricList[i].singerName);
        QFont font2;
        font2.setFamily("SourceHanSansSC-Medium");
        font2.setPixelSize(12);
        singerName->setFont(font2);
        duration->setText(m_lyricList[i].duration);
        QFont font1;
        font1.setFamily("SourceHanSansSC-Medium");
        font1.setPixelSize(12);
        duration->setFont(font1);

        lyricNmae->move(15, 11);
        singerName->move(15, 35);
        duration->move(269, 23);
        DPalette pl = frame->palette();
        pl.setColor(DPalette::Background, QColor(0, 0, 0, 0.03 * 255));
        frame->setPalette(pl);

        DCheckBox *checkBox = new DCheckBox(frame);
        checkBox->setText("");
        checkBox->setFixedSize(16, 16);
        checkBox->move(300, 24);
        m_group->addButton(checkBox, i);
        m_listWidget->setItemWidget(listItem, frame);
    }
}

void SearchLyricsWidget::searchLyrics()
{
    if (m_singer->text().isEmpty() && m_keyWord->text().isEmpty())
        return;
    m_lyricList.clear();
    m_listWidget->clear();
    QList<QAbstractButton *> buttonList = m_group->buttons();
    for (int i = 0; i < buttonList.size(); ++i)
        m_group->removeButton(buttonList.at(i));
    QDir dir(m_path);
    if (!dir.exists())
        return;

    //查看路径中后缀为.lrc格式的文件
    QStringList filters;
    filters << QString("*.lrc");
    dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
    dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式

    //统计lrc格式的文件个数
    int dir_count = dir.count();
    if (dir_count <= 0)
        return;

    for (int i = 0; i < dir_count; ++i) {
        MusicLyric lyc;
        bool right = false;
        int singerIndex = -1;
        if (!lyc.getHeadFromFile(m_path + dir[i]))
            continue;
        if (m_keyWord->text() != "") {
            QString ti = lyc.getLineAt(0);
            if (ti.indexOf(m_keyWord->text()) != -1) {
                if (m_singer->text() != "") {
                    QString ar;
                    for (int i = 0; i < 5; ++i) {
                        ar = lyc.getLineAt(i);
                        if (ar.indexOf(m_singer->text()) != -1) {
                            right = true;
                            singerIndex = i;
                        }
                    }
                } else {
                    right = true;
                }
            }
        } else {
            if (m_singer->text() != "") {
                QString ar;
                for (int i = 0; i < 5; ++i) {
                    ar = lyc.getLineAt(i);
                    if (ar.indexOf(m_singer->text()) != -1) {
                        right = true;
                        singerIndex = i;
                    }
                }
            }
        }
        if (right) {
            lyric lycr;
            QString nameLine = lyc.getLineAt(0);
            if (nameLine.indexOf("[") != -1 && nameLine.indexOf("]") != -1 && nameLine.indexOf("ti") != -1) {
                QString name = nameLine.split(":").last();
                name.chop(2);
                lycr.lyricsName = name;
            } else {
                lycr.lyricsName = nameLine;
            }

            if (singerIndex != -1) {
                QString singerLine = lyc.getLineAt(singerIndex);
                if (singerLine.indexOf("[") != -1 && singerLine.indexOf("]") != -1 && singerLine.indexOf("ar") != -1) {
                    QString name = singerLine.split(":").last();
                    name.chop(2);
                    lycr.singerName = name;
                } else {
                    lycr.singerName = singerLine;
                }
            } else {
                for (int i = 0; i < 5; ++i) {
                    QString singerLine = lyc.getLineAt(i);
                    if (singerLine.indexOf("[ar:") != -1) {
                        QString name = singerLine.split(":").last();
                        name.chop(2);
                        lycr.singerName = name;
                    }
                }
            }
            lycr.path = m_path + QDir::separator() + dir[i];
            bool endTime = true;
            int lineCount = lyc.getCount() - 1;
            while (endTime) {
                QString timeLine = lyc.getLineAt(lineCount);
                if (timeLine.indexOf("[") == -1 || timeLine.indexOf("]") == -1) {
                    --lineCount;
                    continue;
                }
                QString time = timeLine.mid(timeLine.lastIndexOf("[") + 1, timeLine.lastIndexOf("]") - timeLine.lastIndexOf("[") - 1);
                time = time.left(5);
                lycr.duration = time;
                endTime = false;
            }
            m_lyricList.push_back(lycr);
        }
    }
    createList();
}

void SearchLyricsWidget::mySelection(QModelIndex index)
{
    QString path = m_lyricList[index.row()].path;
    emit lyricPath(path);
}

void SearchLyricsWidget::mySelectionIndex(int index)
{
    QString path = m_lyricList[index].path;
    emit lyricPath(path);
}
