import QtQuick 2.4
import QtTest 1.0

Rectangle {
    id: test
    width: 200
    height: 300

    TestCase {
        name: "MathTests"

        function test_math() {
            compare(2 + 2, 4, "2 + 2 = 4")
        }

        function test_fail() {
            compare(2 + 2, 5, "2 + 2 = 5")
        }
    }
}

//ArrowRectangle {

//}
