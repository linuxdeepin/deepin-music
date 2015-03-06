#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from mutagen.mp3 import MP3
from mutagen.easyid3 import EasyID3
f = os.sep.join([os.path.dirname(os.getcwd()), 'music', '1.mp3'])

from mutagen.mp3 import MP3
audio = MP3(f)
print audio.mime
print audio.info.__dict__
# print audio.size
# print EasyID3.valid_keys.keys()
# # audio["title"] = u"An example"
# # audio.save()

# audio = MP3(f, ID3=EasyID3)
# print audio.keys()
# print audio.pprint()
# print audio.filename
# print audio.List