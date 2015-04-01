#!/usr/bin/python
# -*- coding: utf-8 -*-

from UserDict import DictMixin
import json
import leveldb


class LevelDict(object, DictMixin):
    """Dict Wrapper around the Google LevelDB Database"""
    def __init__(self, path):
        """Constructor for LevelDict"""
        self.path = path
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
        return [k for k, v in self.db.RangeIter()]

    def iteritems(self):
        return self.db.RangeIter()

    def rangescan(self, start=None, end=None):
        if start is None and end is None:
            return self.db.RangeIter()
        elif end is None:
            return self.db.RangeIter(start)
        else:
            return self.db.RangeIter(start, end)

class LevelJsonDict(LevelDict):
    """Dict Wrapper around the Google LevelDB Database with JSON serialization"""
    def __getitem__(self, key):
        return json.loads(LevelDict.__getitem__(self, json.loads(key)))

    def __setitem__(self, key, value):
        LevelDict.__setitem__(self, json.dumps(key), json.dumps(value))

    def iteritems(self):
        for k, v in LevelDict.iteritems(self):
            yield k, json.loads(v)

    def rangescan(self, start=None, end=None):
        for k, v in LevelDict.rangescan(self, start, end):
            yield k, json.loads(v)


if __name__ == '__main__':
    db = LevelJsonDict('/tmp/testdb/')
    db['c'] = '1'
    db['b'] = '2'
    db['a'] = '3'
    db[u"非常"] = u"Leveldb是一个google实现的非常高效的kv数据库，目前的版本1.2能够支持billion级别的数据量了。 在这个数量级别下还有着非常高的性能，主要归功于它的良好的设计。特别是LSM算法。"
    db["非常"] = "Lev"
    db[[1, 2, 3]] = [1011]
    for k, v in db.iteritems():
    	print k, v
