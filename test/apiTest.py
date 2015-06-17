#!/usr/bin/python
# -*- coding: utf-8 -*-

import requests

# r = requests.get('http://s.music.haosou.com/player/songForPartner?id=14120&src=linuxdeepin&&sign=7187e529899bebd38c165d6998fe3ccf')

params = {
	'id': 14120,
	'src': 'linuxdeepin',
	'sign': '7187e529899bebd38c165d6998fe3ccf'
}

r = requests.get("http://s.music.haosou.com/player/songForPartner", params=params)

print(r.json())
