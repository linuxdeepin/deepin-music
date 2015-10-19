#!/usr/bin/python
# -*- coding: utf-8 -*-

from UserDict import DictMixin
import json


class LevelDict_leveldb(object, DictMixin):
    """Dict Wrapper around the Google LevelDB Database"""
    def __init__(self, path):
        """Constructor for LevelDict"""
        self.path = path

    def open(self):
        import leveldb
        self.db = leveldb.LevelDB(self.path)

    def __getitem__(self, key):
        return self.db.Get(key)

    def __setitem__(self, key, value):
        self.db.Put(key, value)

    def __delitem__(self, key):
        self.db.Delete(key)

    def __iter__(self):
        for k in self.db.RangeIter(include_value=False):
            yield k 

    def keys(self):
        return [k for k, _ in self.db.RangeIter()]

    def iteritems(self):
        return self.db.RangeIter()

    def rangescan(self, start=None, end=None):
        if start is None and end is None:
            return self.db.RangeIter()
        elif end is None:
            return self.db.RangeIter(start)
        else:
            return self.db.RangeIter(start, end)


class LevelJsonDict_leveldb(LevelDict_leveldb):
    """Dict Wrapper around the Google LevelDB Database with JSON serialization"""

    def __getitem__(self, key):
        return json.loads(LevelDict_leveldb.__getitem__(self, json.dumps(key)))

    def __setitem__(self, key, value):
        try:
            LevelDict_leveldb.__setitem__(self, json.dumps(key), json.dumps(value))
        except Exception, e:
            raise e
            print key, value

    def __delitem__(self, key):
        self.db.Delete(json.dumps(key))

    def __iter__(self):
        for k in self.db.RangeIter(include_value=False):
            yield json.loads(k)

    def keys(self):
        return [json.loads(k) for k in self.db.RangeIter(include_value=False)]

    def values(self):
        return [json.loads(v) for _, v in self.db.RangeIter()]

    def iteritems(self):
        for k, v in LevelDict_leveldb.iteritems(self):
            yield json.loads(k), json.loads(v)

    def rangescan(self, start=None, end=None):
        for k, v in LevelDict_leveldb.rangescan(self, json.dumps(start), json.dumps(end)):
            yield json.loads(k), json.loads(v)


class LevelDict_plyvel(object, DictMixin):
    """Dict Wrapper around the Google LevelDB Database"""
    def __init__(self, path):
        """Constructor for LevelDict"""
        self.path = path

    def open(self):
        import plyvel
        self.db = plyvel.DB(self.path, create_if_missing=True)

    def __getitem__(self, key):
        value = self.db.get(key)
        if value is None:
            raise KeyError
        else:
            return value

    def __setitem__(self, key, value):
        self.db.put(key, value)

    def __delitem__(self, key):
        self.db.delete(key)

    def __iter__(self):
        for k in self.db.iterator(include_value=False):
            yield k

    def keys(self):
        return [k for k, _ in self.db.iterator()]

    def iteritems(self):
        return self.db.iterator()

    def rangescan(self, start=None, end=None):
        if start is None and end is None:
            return self.db.iterator()
        elif end is None:
            return self.db.iterator(start=start)
        else:
            return self.db.iterator(start=start, stop=end)


class LevelJsonDict_plyvel(LevelDict_plyvel):
    """Dict Wrapper around the Google LevelDB Database with JSON serialization"""

    def __getitem__(self, key):
        return json.loads(LevelDict_plyvel.__getitem__(self, json.dumps(key)))

    def __setitem__(self, key, value):
        try:
            LevelDict_plyvel.__setitem__(self, json.dumps(key), json.dumps(value))
        except Exception, e:
            raise e
            print key, value

    def __delitem__(self, key):
        self.db.delete(json.dumps(key))

    def __iter__(self):
        for k in self.db.iterator(include_value=False):
            yield json.loads(k)

    def keys(self):
        return [json.loads(k) for k in self.db.iterator(include_value=False)]

    def values(self):
        return [json.loads(v) for _, v in self.db.iterator()]

    def iteritems(self):
        for k, v in LevelDict_plyvel.iteritems(self):
            yield json.loads(k), json.loads(v)

    def rangescan(self, start=None, end=None):
        for k, v in LevelDict_plyvel.rangescan(self, json.dumps(start), json.dumps(end)):
            yield json.loads(k), json.loads(v)


try:
    import leveldb
    LevelDict = LevelDict_leveldb
    LevelJsonDict = LevelJsonDict_leveldb
except ImportError:
    import plyvel
    LevelDict = LevelDict_plyvel
    LevelJsonDict = LevelJsonDict_plyvel


if __name__ == '__main__':


    db = LevelJsonDict('/tmp/artist/')

    # import time
    # t1 = time.time()
    # db.clear()
    # # t2 = time.time()
    # # print t2 - t1
    # i = 0
    # while 1:
    #     db['artist%d' % i] = "Leveldb是一个google实现的非常高效的kv数据库，目前的版本1.2能够支持billion级别的数据量了。 在这个数量级别下还有着非常高的性能，主要归功于它的良好的设计。特别是LSM算法。" * 10
    #     i = i + 1
    #     if time.time() - t1 > 1:
    #         break
    # print i
    # # print db.values()[0]
    # print time.time() - t1
    # # db['artista'] = '1'
    # # db['artistb'] = '2'
    # # db['artistc'] = '3'
    # # db['artistad'] = '1'
    # db['b'] = '2'
    # db['a'] = '3'
    # db[u"非常"] = "Leveldb是一个google实现的非常高效的kv数据库，目前的版本1.2能够支持billion级别的数据量了。 在这个数量级别下还有着非常高的性能，主要归功于它的良好的设计。特别是LSM算法。"
    # # db["非常"] = "Lev"
    # db[[1, 2, 3]] = [{1: 2}]
    # db[{1: 1, 2:2 ,3:3}] = [{1: 2}]
    # print len(db.keys())
    # print [1, 2 ,3] in db
    # print db[{1: 1, 2:2 ,3:3}][0]


    # d = {1: 2, 3:4, u'1': 5}
    # print json.dumps(d)
    # print json.loads(json.dumps(d))

    # print db
    # # db.clear()
    for k, v in db.iteritems():
        print k, v
    # for k, v in db.rangescan(start='artist1', end='artist10'):
    #     print k, v, type(k), type(v)

    # import time
    # time.sleep(10)
