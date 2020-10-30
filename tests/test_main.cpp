#include <QtTest>
#include <QCoreApplication>
#include <QTest>

#include <QObject>
#include <QScopedPointer>
#include <util/singleton.h>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <DApplication>
//#include <QApplication>

#include "musicapp.h"
#include "mainframe.h"
#include "core/musicsettings.h"
#include "presenter.h"

using namespace Dtk::Core;
using namespace Dtk::Widget;

#define QMYTEST_MAIN(TestObject) \
    QT_BEGIN_NAMESPACE \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
    QT_END_NAMESPACE \
    int main(int argc, char *argv[]) \
    { \
        DApplication app(argc, argv); \
        app.setAttribute(Qt::AA_UseHighDpiPixmaps); \
        MusicSettings* settings = MusicSettings::instance(); \
        MusicSettings::init(); \
        MusicSettings::sync(); \
        QTestMain::getMainwindow(); \
        Q_UNUSED(settings); \
        app.setSingleInstance("deepinmusic"); \
        QTEST_DISABLE_KEYPAD_NAVIGATION \
        QTEST_ADD_GPU_BLACKLIST_SUPPORT \
        TestObject tc; \
        QTEST_SET_MAIN_SOURCE_PATH \
        RUN_ALL_TESTS();\
        return QTest::qExec(&tc, argc, argv); \
    } \

class QTestMain : public QObject
{
    Q_OBJECT

public:
    QTestMain();
    ~QTestMain();

    static MusicApp *getMainwindow();
private slots:
//    void initTestCase();
//    void cleanupTestCase();

    void testQString_data();
    void testQString();

    void testFloat_data();
    void testFloat();

    void testGui_data();
};

QTestMain::QTestMain()
{

}

QTestMain::~QTestMain()
{

}

void QTestMain::testQString_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("result");
    QTest::newRow("lower") << "hello" << "HELLO";
    QTest::newRow("mix") << "heLLo" << "HELLO";
    QTest::newRow("upper") << "HELLO" << "HELLO";
}

void QTestMain::testQString()
{
    QFETCH(QString, string);
    QFETCH(QString, result);
    QCOMPARE(string.toUpper(), result);
    QBENCHMARK{
        Q_UNUSED(string.toUpper())
    }

}

void QTestMain::testFloat_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<double>("score");
    QTest::newRow("zhangsan") << "zhangsan" << 60.0;
    QTest::newRow("lisi") << "lisi" << 56.0;
    QTest::newRow("wanger") << "wanger" << 48.0;
}

void QTestMain::testFloat()
{
    QFETCH(QString, name);
    QFETCH(double, score);
    QVERIFY2(score >= 30.0, name.toLocal8Bit() + " score: " + QString::number(score).toLocal8Bit());
}

void QTestMain::testGui_data()
{
    QTest::addColumn<QTestEventList>("event");
    QTest::addColumn<QString>("result");

    QTestEventList list1;
    list1.addKeyClicks("hello world");
    QTest::newRow("item 0 ") << list1 << QString("hello world");

    QTestEventList list2;
    list2.addKeyClicks("abs0");
    list2.addKeyClick(Qt::Key_Backspace);
    QTest::newRow("item 1") << list2 << QString("abs");
}

MusicApp *QTestMain::getMainwindow()
{
    MusicSettings::init();
    MainFrame *frame = new MainFrame;
//    frame->setCoverBackground("");
//    frame->onSelectImportDirectory();
//    frame->onSelectImportFiles();
//    frame->onViewShortcut();
    frame->slotTheme(1);
    static MusicApp *w = new MusicApp(frame);
    auto showflag = MusicSettings::value("base.play.showFlag").toBool();

    frame->initUI(true);
    Q_EMIT frame->triggerShortcutAction("shortcuts.all.volume_up");
    Q_EMIT frame->triggerShortcutAction("shortcuts.all.volume_down");
    Q_EMIT frame->triggerShortcutAction("shortcuts.all.next");
    Q_EMIT frame->triggerShortcutAction("shortcuts.all.play_pause");
    Q_EMIT frame->triggerShortcutAction("shortcuts.all.previous");
    w->initConnection(showflag);
    w->initUI(true);
    QStringList list1;
    list1 << "1" << "2";
    w->onStartImport(list1);

    QStringList list;
    list << "/usr/share/music/bensound-sunny.mp3";
//    Q_EMIT w->sigStartImport(list);
    w->show();
    //testSig_data();
//    w->quit();
    return w;
}


QMYTEST_MAIN(QTestMain)

#include "test_main.moc"
