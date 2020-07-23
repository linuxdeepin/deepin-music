#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    return RUN_ALL_TESTS();
    //return a.exec();
}
