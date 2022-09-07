# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

lcov --directory ./CMakeFiles/deepin-music-test.dir --zerocounters
lcov --directory . --capture --output-file ./coverageResult/deepin-music-test_Coverage.info
echo \ ===================\ do\ filter\ begin\ ====================\ 
lcov --remove ./coverageResult/deepin-music-test_Coverage.info '*/deepin-music-test_autogen/*' '*/deepin-music-test_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '*/gtest/*' -o ./coverageResult/deepin-music-test_Coverage.info
echo \ ===================\ do\ filter\ end\ ====================\ 
genhtml -o ./coverageResult/report ./coverageResult/deepin-music-test_Coverage.info
