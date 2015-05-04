
深度音乐３.0开发环境搭建

###1. 安装Qt5.4.1
[Qt 5.4.1](http://download.qt-project.org/official_releases/qt/5.4/5.4.1/qt-opensource-linux-x64-5.4.1.run)下载安装

###2. 安装python2.7虚拟环境搭建：
**virtualenv**
virtualenv用于创建独立的Python环境，多个Python相互独立，互不影响，它能够：
1. 在没有权限的情况下安装新套件
2. 不同应用可以使用不同的套件版本
3. 套件升级不影响其他应用

安装
	sudo apt-get install python-virtualenv
	sudo easy_install virtualenvwrapper

###3.安装PyQt5.4.0

下载安装[sip](http://www.riverbankcomputing.com/software/sip/download)
下载安装[PyQt5.4](http://www.riverbankcomputing.com/software/pyqt/download5)
	

###4.安装deepin-qml-widgets
安装依赖：

	sudo apt-get build-dep deepin-qml-widgets

安装：
	
	git clone git@github.com:linuxdeepin/deepin-qml-widgets.git


###5. 安装WebEngineWrapper
	git clone git@github.com:rodrigogolive/WebEngineWrapper.git


###6. deepin_menu
	git clone git@github.com:linuxdeepin/deepin-menu.git

###7. pyquery
	sudo apt-get install libxml2-dev libxslt1-dev python-dev
	sudo apt-get install zlib1g-dev
	pip install pyquery

###8.pycurl
	sudo apt-get install libcurl4-openssl-dev
	pip install pycurl
