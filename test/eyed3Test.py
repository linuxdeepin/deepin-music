#!/usr/bin/python
# -*- coding: utf-8 -*-

import eyed3

uris = [
'../music/1.mp3',
'/usr/share/deepin-sample-music/邓入比_我们的情歌.mp3',
'/usr/share/deepin-sample-music/郭一凡_说走就走的旅行.mp3',
'/usr/share/deepin-sample-music/胡彦斌_依然是你.mp3'
]


for uri in uris:
    audiofile = eyed3.load(uri.decode('utf-8'))
    song = {}
    tagKeys = ['artist', 'title', 'album', 'album_artist', 'track_num', 'play_count']

    for key in tagKeys:
        song.update({key: getattr(audiofile.tag, key)})


    audioKeys = ['bit_rate_str', 'mode', 'sample_freq', 'size_bytes', 'time_secs']
    for key in audioKeys:
        song.update({key: getattr(audiofile.info, key)})

    print song

    print audiofile.tag.images, len(audiofile.tag.images)
    if audiofile.tag.images and len(audiofile.tag.images) > 0:
        image = audiofile.tag.images[0]
        print image.description
        print image.picTypeToString(image.picture_type)
        print image.mime_type
        with open(image.makeFileName(song['title']), 'w') as f:
            f.write(image.image_data)

    # print audiofile.tag.lyrics, audiofile.tag.lyrics.description
