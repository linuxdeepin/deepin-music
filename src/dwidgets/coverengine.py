#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import random
from collections import OrderedDict
import json
from PyQt5.QtCore import (QObject, pyqtSignal, pyqtSlot,
      pyqtProperty, QUrl, QFile, QIODevice, QTimer, QRunnable, QThread, QThreadPool
      )
from PyQt5.QtGui import QGuiApplication
import requests
import urllib
import urllib2
import StringIO
import gzip
import re
from coverlrc.cover_query import multi_query_album_engine, multi_query_artist_engine
from log import logger


class XiamiTingEngine(object):

    def __init__(self):
        super(XiamiTingEngine, self).__init__()

    def searchCoverByArtistName(self, artist):
        try:
            return multi_query_artist_engine(artist)
        except Exception, e:
            raise
            return False

    def searchCoverByAlbumName(self, artist, album):
        try:
            return multi_query_album_engine(artist, album)
        except Exception, e:
            return False

class NetEaseEngine(object):
    def __init__(self):
        self.header = {
            'Accept': '*/*',
            'Accept-Encoding': 'gzip,deflate,sdch',
            'Accept-Language': 'zh-CN,zh;q=0.8,gl;q=0.6,zh-TW;q=0.4',
            'Connection': 'keep-alive',
            'Content-Type': 'application/x-www-form-urlencoded',
            'Host': 'music.163.com',
            'Referer': 'http://music.163.com/search/',
            'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.152 Safari/537.36'
        }
        self.cookies = {
            'appver': '1.5.2'
        }
        self.playlist_class_dict = {}

    def httpRequest(self, method, action, query=None, urlencoded=None, callback=None, timeout=None):
        connection = json.loads(self.rawHttpRequest(method, action, query, urlencoded, callback, timeout))
        return connection

    def rawHttpRequest(self, method, action, query=None, urlencoded=None, callback=None, timeout=None):
        if (method == 'GET'):
            url = action if (query == None) else (action + '?' + query)
            connection = requests.get(url, headers=self.header, timeout=10)

        elif (method == 'POST'):
            connection = requests.post(
                action,
                data=query,
                headers=self.header,
                timeout=timeout
            )

        connection.encoding = "UTF-8"
        return connection.text

    # song id --> song url ( details )
    def getSongDetail(self, music_id):
        action = "http://music.163.com/api/song/detail/?id=" + str(music_id) + "&ids=[" + str(music_id) + "]"
        try:
            data = self.httpRequest('GET', action)
            return data
        except:
            return []

    # 搜索单曲(1)，歌手(100)，专辑(10)，歌单(1000)，用户(1002) *(type)*
    def search(self, s, stype=1, offset=0, total='true', limit=60):
        action = 'http://music.163.com/api/search/get/web'
        data = {
            's': s,
            'type': stype,
            'offset': offset,
            'total': total,
            'limit': 60
        }
        return self.httpRequest('POST', action, data)

    def searchCoverByArtistName(self, artist):
        '''
        name: 谢霆锋
        {
            "code": 200, 
            "result": {
                "artistCount": 1, 
                "artists": [
                    {
                        "mvSize": 59, 
                        "img1v1Url": "http://p4.music.126.net/x25AINe4TNwoU01kKQzI8w==/639915767391187.jpg", 
                        "followed": false, 
                        "name": "\u8c22\u9706\u950b", 
                        "albumSize": 34, 
                        "alias": [], 
                        "picId": 237494511616563, 
                        "img1v1": 639915767391187, 
                        "picUrl": "http://p3.music.126.net/4WK8zw33vRD_W54rBiXSug==/237494511616563.jpg", 
                        "trans": null, 
                        "id": 5773
                    }
                ]
            }
        }
        '''
        try:
            ret = self.search(artist, stype=100)
            picUrl = ''
            if ret['code'] == 200 and ret['result'] and ret['result']['artistCount'] >= 1:
                picUrl = ret['result']['artists'][0]['picUrl']
        except Exception, e:
            picUrl = False
        return picUrl


    def searchCoverByAlbumName(self, artist, album):
        '''
        name = 刘德华演唱会99
        {
            "code": 200, 
            "result": {
                "albumCount": 1, 
                "albums": [
                    {
                        "status": 1, 
                        "blurPicUrl": "http://p4.music.126.net/LRWWw0_d0wZWWicIKQIkfA==/65970697667341.jpg", 
                        "commentThreadId": "R_AL_3_10945", 
                        "name": "\u7231\u4f60\u4e00\u4e07\u5e74 99\u6f14\u5531\u4f1a", 
                        "companyId": 0, 
                        "tags": "", 
                        "description": "", 
                        "artist": {
                            "img1v1Url": "http://p4.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg", 
                            "name": "\u5218\u5fb7\u534e", 
                            "briefDesc": "", 
                            "albumSize": 98, 
                            "img1v1Id": 0, 
                            "musicSize": 1517, 
                            "alias": [
                                "Andy Lau"
                            ], 
                            "picId": 40681930245654, 
                            "picUrl": "http://p4.music.126.net/3ASmrU2Nej5t6Hj_7JkQOw==/40681930245654.jpg", 
                            "trans": "", 
                            "id": 3691
                        }, 
                        "pic": 65970697667341, 
                        "publishTime": 938707200000, 
                        "briefDesc": "", 
                        "alias": [], 
                        "picId": 65970697667341, 
                        "copyrightId": 5003, 
                        "picUrl": "http://p3.music.126.net/LRWWw0_d0wZWWicIKQIkfA==/65970697667341.jpg", 
                        "company": "BMA", 
                        "artists": [
                            {
                                "img1v1Url": "http://p4.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg", 
                                "name": "\u5218\u5fb7\u534e", 
                                "briefDesc": "", 
                                "albumSize": 0, 
                                "img1v1Id": 0, 
                                "musicSize": 0, 
                                "alias": [], 
                                "picId": 0, 
                                "picUrl": "http://p4.music.126.net/6y-UleORITEDbvrOLV0Q8A==/5639395138885805.jpg", 
                                "trans": "", 
                                "id": 3691
                            }
                        ], 
                        "songs": [], 
                        "type": "\u4e13\u8f91", 
                        "id": 10945, 
                        "size": 32
                    }
                ]
            }
        }
        '''
        try:
            ret = self.search(album, stype=10)
            picUrl = ''
            if ret['code'] == 200 and ret['result'] and ret['result']['albumCount'] >= 1:
                picUrl = ret['result']['albums'][0]['picUrl']
        except Exception, e:
            picUrl = False
        return picUrl

class DoubanEngine(object):

    _doubanSearchApi    = 'http://music.douban.com/subject_search?search_text={0}&cat=1001'
    _doubanCoverPattern = '<img src="http://img3.douban.com/spic/s(\d+).jpg"'
    _doubanConverAddr   = 'http://img3.douban.com/lpic/s{0}.jpg'

    def __init__(self):
        super(DoubanEngine, self).__init__()

    def urlread(self, url, need_gzip = True, host = "www.douban.com"):
        req = urllib2.Request(url)
        req.add_header("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:15.0) Gecko/20100101 Firefox/15.0.1")
        req.add_header("Referer", "http://music.douban.com/")
        req.add_header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8")
        req.add_header("Accept-Encoding", "gzip, deflate")
        req.add_header("Accept-Language", "zh-cn,zh;q=0.8,en-us;q=0.5,en;q=0.3")
        req.add_header("Connection", "keep-alive")
        req.add_header("Host", host)
        req.add_header("Cookie", 'bid="9vSv1w9XJNs"; report=ref=%2Fsubject_search&mus_msc=musmsc_1') 

        source = urllib2.urlopen(req).read()

        if need_gzip == True:
            data = StringIO.StringIO(source)
            gzipper = gzip.GzipFile(fileobj=data)
            source = gzipper.read()
        return source

    def searchCover(self, artist, album='', title=''):
        if isinstance(artist, unicode):
            artist = artist.encode('utf-8')
        if isinstance(album, unicode):
            album = album.encode('utf-8')
        if isinstance(title, unicode):
            title = title.encode('utf-8')

        keywords = artist + ' ' + (album or title)
        request = self._doubanSearchApi.format(urllib.quote(keywords))
        try:
            result  = self.urlread(request)
            if not len(result):
                return False

            match = re.compile(self._doubanCoverPattern, re.IGNORECASE).search(result)
            if match:
                return self._doubanConverAddr.format(match.groups()[0])
            else:
                return False
        except Exception, e:
            False

    def searchCoverByArtistName(self, artist):
        return self.searchCover(artist)

    def searchCoverByAlbumName(self, artist, album):
        return self.searchCover(artist, album)


class CoverRunnable(QRunnable):

    def __init__(self, coverWorker, artist, album='' ,qtype='artist', **kwargs):
        super(CoverRunnable, self).__init__()
        self.coverWorker = coverWorker
        self.artist = artist
        self.album = album
        self.qtype = qtype

    def run(self):
        netEaseEngine = NetEaseEngine()
        xiamiTingEngine = XiamiTingEngine()
        doubanEngine = DoubanEngine()
        engines = [xiamiTingEngine, doubanEngine, netEaseEngine]

        try:
            if self.qtype == "artist":
                url = engines[0].searchCoverByArtistName(self.artist)
                if not url:
                    url = engines[1].searchCoverByArtistName(self.artist)
                if not url:
                    url = engines[2].searchCoverByArtistName(self.artist)
                if url:
                    localUrl = self.coverWorker.artistCoverPath(self.artist)
                    flag = self.downloadCoverByUrl(url, localUrl)
                    if flag:
                        self.coverWorker.downloadArtistCoverSuccessed.emit(self.artist, localUrl)
                    else:
                        self.coverWorker.downloadArtistCoverSuccessed.emit(self.artist, '')
                else:
                    self.coverWorker.downloadArtistCoverSuccessed.emit(self.artist, '')

            elif self.qtype == "album":
                url = engines[0].searchCoverByAlbumName(self.artist, self.album)
                if not url:
                    url = engines[1].searchCoverByAlbumName(self.artist, self.album)
                if not url:
                    url = engines[2].searchCoverByAlbumName(self.artist, self.album)
                if url:
                    localUrl = self.coverWorker.albumCoverPath(self.artist, self.album)
                    flag = self.downloadCoverByUrl(url, localUrl)
                    if flag:
                        self.coverWorker.downloadAlbumCoverSuccessed.emit(self.artist, self.album, localUrl)
                    else:
                        self.coverWorker.downloadAlbumCoverSuccessed.emit(self.artist, self.album, '')
                else:
                    self.coverWorker.downloadAlbumCoverSuccessed.emit(self.artist, self.album, '')
        except Exception, e:
            logger.error(e)

    def downloadCoverByUrl(self, url, localUrl):
        try:
            if '\/' in url:
                url = url.replace('\\', '')
            r = requests.get(url)
            with open(localUrl, "wb") as f:
                f.write(r.content)
            return True
        except:
            return False


class CoverWorker(QObject):

    downloadArtistCoverSuccessed = pyqtSignal('QString', 'QString')
    downloadAlbumCoverSuccessed = pyqtSignal('QString', 'QString', 'QString')

    def __init__(self):
        super(CoverWorker, self).__init__()
        QThreadPool.globalInstance().setMaxThreadCount(4)
        self.downloadArtistCoverSuccessed.connect(self.cacheArtistCover)
        self.downloadAlbumCoverSuccessed.connect(self.cacheAlbumCover)
        self.artistCovers = {}
        self.albumCovers = {}

    def cacheArtistCover(self, artist, url):
        # print artist, url
        self.artistCovers[artist]  = url

    def cacheAlbumCover(self, artist, album, url):
        # print artist, album, url
        self.albumCovers[album] = url

    def downloadArtistCover(self, artist):
        d = CoverRunnable(self, artist, qtype="artist")
        QThreadPool.globalInstance().start(d)

    def downloadAlbumCover(self, artist, album):
        d = CoverRunnable(self, artist, album, qtype="artist")
        QThreadPool.globalInstance().start(d)

    @classmethod
    def getArtistCoverPath(cls, artist):
        ArtistCoverPath =  '/home/djf/.config/DeepinMusic3/cover/artist'
        filepath = os.path.join(ArtistCoverPath, artist)
        return filepath

    @classmethod
    def getAlbumCoverPath(cls, artist, album):
        ArtistCoverPath =  '/home/djf/.config/DeepinMusic3/cover/album'
        filepath = os.path.join(ArtistCoverPath, '%s-%s' % (artist, album))
        return filepath


if __name__ == '__main__':
    app = QGuiApplication(sys.argv)
    QThreadPool.globalInstance().setMaxThreadCount(10)
    from peewee import *
    MusicDBFile = '/home/djf/.config/DeepinMusic3/music.db'
    db = SqliteDatabase(MusicDBFile, threadlocals=True)
    db.connect()

    coverWorker = CoverWorker()
    artists = []
    for artist in db.get_cursor().execute('Select name from artist').fetchall():
        artists.append(artist[0])
    for artist in artists:
        coverWorker.downloadArtistCover(artist)

    albums = []
    for artist, album in db.get_cursor().execute('Select artist, name from album').fetchall():
        albums.append((artist, album))
    for _album in albums:
        artist, album = _album
        coverWorker.downloadAlbumCover(artist, album)

    print(QThreadPool.globalInstance().maxThreadCount())

    exitCode = app.exec_()
    sys.exit(exitCode)
