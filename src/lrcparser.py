#!/usr/bin/env python
#-*-coding:utf-8-*-

import re

class LrcParser:
    def __init__(self, file_name=""):
        self.lrc_file = file_name
        self.lrc_source = []
        self.lrc_target = []
        self.lrc_write_back = []

        self.time_line = []

    def lrc_parser(self):
        self.lrc_line = []
        lrc_fd = open(self.lrc_file, 'rw')
        for each_line in lrc_fd:
            tmp = each_line.strip('\n\r')
            self.lrc_line.append(tmp)
        for key, item in enumerate(self.lrc_line):
            self.lrc_source.append(self.line_parser(key, item))
        for item in self.lrc_source:
            self.time_parser(item)
        self.sort_lrc()

    def is_time_token(self, it):
        ret_val = True
        for c in it:
            if (not c.isdigit()) and c != ':' and c != '.':
                ret_val = False
        return ret_val

    def transform_time_token_back(self, token):
        ret_val = None
        tail = token % 100
        token -= tail
        token /= 100
        second = token % 60
        token -= second
        token /= 60
        minute = token % 60
        token -= minute
        token /= 60
        hour = token
        if hour != 0:
            ret_val = "%02d" % hour + ':' + "%02d" % minute + ':'\
                    + "%02d" % second + '.' + "%02d" % tail
        else:
            ret_val = "%02d" % minute + ':' + "%02d" % second \
                    + '.' + "%02d" % tail
        return ret_val

    def transform_time_token(self, token):
        ret_val = 0
        if token.rfind('.'):
            ret_val += int(token[token.rfind('.') + 1 : ])
            token = token[ : token.rfind('.')]
        token = ':' + token
        step = 's'
        while token.rfind(':') != -1:
            if step == 's':
                ret_val += int(token[token.rfind(':') + 1 : ]) * 100
                token = token[ : token.rfind(':')]
                step = 'm'
            elif step == 'm':
                ret_val += int(token[token.rfind(':') + 1 : ]) * 6000
                token = token[ : token.rfind(':')]
                step = 'h'
            elif step == 'h':
                ret_val += int(token[token.rfind(':') + 1 : ]) * 360000
                token = token[ : token.rfind(':')]
                step = 'n'
            elif step == 'n':
                # Will never be here!!!
                break
        return ret_val

    def time_parser(self, item):
        tmp = []
        tmp2 = None
        line_num = item.pop(0)
        i = 0
        for it in item:
            if self.is_time_token(it):
                tmp.append(self.transform_time_token(it))
        if not self.is_time_token(item[0]):
            self.lrc_target.append([line_num, 0, item[0]])
        while len(tmp) != 0:
            if not self.is_time_token(item[-1]):
                self.lrc_target.append([line_num, tmp.pop(0), item[-1]])
            else:
                self.lrc_target.append([line_num, tmp.pop(0), ""])

    def line_parser(self, key, item):
        ret_val = []
        ret_val.append(key)
        while item.find('[') != -1:
            ret_val.append(item[item.find('[') + 1:item.find(']')])
            item = item[item.find(']') + 1 : ]
        if item.strip(' ') != '':
            ret_val.append(item)
        return ret_val

    def sort_lrc(self):
        self.lrc_target.sort(self.lrc_comp)

    def lrc_comp(self, it1, it2):
        ret_val = 0
        if it1[1] > it2[1]:
            ret_val = 1
        elif it1[1] < it2[1]:
            ret_val = -1
        else:
            ret_val = 0
        return ret_val

    def get_lrc_id(self, time):
        i = 0
        for i in range(0, len(self.lrc_target)):
            if time < self.lrc_target[i][1]:
                break;
            i += 1
        self.current_lrc_id = i - 1

    def get_current_content(self, time):
        ret_val = []
        next_val = []
        self.get_lrc_id(time)
        ret_val.append(self.lrc_target[self.current_lrc_id][1])
        ret_val.append(self.lrc_target[self.current_lrc_id + 1][1] - \
                self.lrc_target[self.current_lrc_id][1])
        ret_val.append(self.lrc_target[self.current_lrc_id][2])

        return (ret_val)
    
    def get_next_content(self, time):
        next_val = []
        self.get_lrc_id(time)
        next_val.append(self.lrc_target[self.current_lrc_id + 1][1])
        next_val.append(self.lrc_target[self.current_lrc_id + 2][1] - \
                self.lrc_target[self.current_lrc_id + 1][1])
        next_val.append(self.lrc_target[self.current_lrc_id + 1][2])
        return next_val

    

    def set_lrc_offset(self, offset):
        for item in self.lrc_target:
            if item[1] == 0:
                continue
            else:
                item[1] += soffset
                if item[1] < 0:
                    item[1] = 0

    def lrc_write(self):
        for i in range(0, len(self.lrc_source)):
            self.lrc_write_back.append('')
        for item in self.lrc_target:
            line_num = item[0]
            if item[1] != 0 or (item[1] == 0 and \
                    len(self.lrc_source[line_num]) != 1):
                self.lrc_write_back[line_num] += '[' + \
                        self.transform_time_token_back(item[1]) + ']'
            else:
                self.lrc_write_back[line_num] += '[' + item[-1] + ']'
        for i in range(0, len(self.lrc_source)):
            if len(self.lrc_source[i]) != 1:
                self.lrc_write_back[i] += self.lrc_source[i][-1]
                
    def set_lrc_file(self, filename):            
        self.lrc_file = filename
        self.lrc_parser()        
        
    def get_lrc_file(self):    
        return self.lrc_file
    
lrcparser = LrcParser()

if __name__ == '__main__':
    test = LrcParser('test.lrc')
    #for item in test.lrc_target:
    #test.lrc_write()
    '''
    for item in test.lrc_write_back:
        print item
    for item in test.lrc_source:
        print item
    '''
    print test.get_lrc_content(4000)

