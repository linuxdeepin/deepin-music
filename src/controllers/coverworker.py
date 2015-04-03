#!/usr/bin/python
# -*- coding: utf-8 -*-


import os
import sys
import copy
import json
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot
import requests
from log import logger
from .utils import registerContext
from dwidgets import dthread
from config.constants import CoverPath



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

    def searchCoverByArtistName(self, name):
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
            ret = self.search(name, stype=100)
            picUrl = ''
            if ret['code'] == 200 and ret['result'] and ret['result']['artistCount'] >= 1:
                picUrl = ret['result']['artists'][0]['picUrl']
        except Exception, e:
            picUrl = ''
        return picUrl


    def searchCoverByAlbumName(self, name):
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
            ret = self.search(name, stype=10)
            picUrl = ''
            if ret['code'] == 200 and ret['result'] and ret['result']['albumCount'] >= 1:
                picUrl = ret['result']['albums'][0]['picUrl']
        except Exception, e:
            picUrl = ''
        return picUrl


class XiaMiEngine(object):

    def __init__(self):
        pass

    def searchCoverByArtistName(self, name):
        pass

    def searchCoverByAlbumName(self, name):
        pass


class CoverWorker(QObject):

    __contextName__ = "CoverWorker"

    downloadCoverSuccessed = pyqtSignal('QString', 'QString')

    @registerContext
    def __init__(self, parent=None):
        super(CoverWorker, self).__init__(parent)

        self.netEaseEngine = NetEaseEngine()
        # url = self.netEaseEngine.searchCoverByArtistName(u'谢霆锋')
        # print url
        # url = self.netEaseEngine.searchCoverByAlbumName(u'刘德华演唱会99')
        # print url
        
        self.initConnect()

    def initConnect(self):
        pass

    @classmethod
    def md5(cls, string):
        import hashlib
        md5Value = hashlib.md5(string)
        return md5Value.hexdigest()

    @classmethod
    def getCoverPathByMediaUrl(cls, url):
        if isinstance(url, unicode):
            url = url.encode('utf-8')
        coverID = cls.md5(url)
        filename = '%s' % coverID
        filepath = os.path.join(CoverPath, filename)
        return filepath

    @pyqtSlot('QString', 'QString')
    @dthread
    def downloadCoverByUrl(self, mediaUrl, coverUrl):
        filepath = self.getCoverPathByMediaUrl(mediaUrl)
        try:
            r = requests.get(coverUrl)
            with open(filepath, "wb") as f:
                f.write(r.content)
            self.downloadCoverSuccessed.emit(mediaUrl, filepath)
        except:
            pass

    def getCoverBySongName(self, name):
        pass

    def getCoverByAlbumName(self, name):
        pass

    def getCoverBySignerName(self, name):
        pass 
