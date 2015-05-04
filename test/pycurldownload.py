# -*- coding: utf8 -*-

import sys
import os
import time
import logging
import urllib
import urlparse
import codecs
import traceback
from tempfile import *
# from dict4ini import DictIni
import pycurl


try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO

if os.name == 'posix':
    # 使用pycurl.NOSIGNAL选项时忽略信号SIGPIPE
    import signal
    signal.signal(signal.SIGPIPE, signal.SIG_IGN)
    del signal

# 支持的协议
VALIDPROTOCOL = ('http', 'ftp')
# HTTP状态码
STATUS_OK = (200, 203, 206)
STATUS_ERROR = range(400, 600)
# 最小数据片大小(128kb)
MINPIECESIZE = 131072
# 最大连接数
MAXCONCOUNT = 10
# 最大重试数
MAXRETRYCOUNT = 5
# 日志级别
# LOGLEVEL = logging.DEBUG
# 清屏命令
CLS = 'cls' if os.name == 'nt' else 'clear'

# 下载日志文件
DLOG = 'download.log'


def Traceback():
    s = StringIO()
    traceback.print_exc(file=s)
    return s.getvalue()


class Connection:

    def __init__(self, url):
        self.curl = pycurl.Curl()
        self.curl.setopt(pycurl.FOLLOWLOCATION, 1)
        self.curl.setopt(pycurl.MAXREDIRS, 5)
        self.curl.setopt(pycurl.CONNECTTIMEOUT, 30)
        self.curl.setopt(pycurl.TIMEOUT, 300)
        self.curl.setopt(pycurl.NOSIGNAL, 1)
        self.curl.setopt(pycurl.WRITEFUNCTION, self.write_cb)
        self.curl.setopt(pycurl.URL, url)
        self.curl.connection = self

        # 合计下载字节数
        self.total_downloaded = 0

    def start(self, result, piece):
        if isinstance(piece, list):
            self.id = piece[0]
            self.name = 'Piece % 02d' % piece[0]
            # 设置断点范围
            self.curl.setopt(pycurl.RANGE, '% d - %d' % (piece[1], piece[2]))
            self.piece_size = piece[2] - piece[1] + 1
            self.piece = piece
        else:
            self.id = 0
            self.name = 'TASK'
            self.piece_size = piece
            self.piece = None

        # 一次连接的已下载字节数
        self.link_downloaded = 0
        # 一个片断的已下载字节数
        self.piece_downloaded = 0
        # 连接重试数
        self.retried = 0
        # 下载中止标志
        self.is_stop = False
        # 结果输出文件对象
        self.result = result
        self.piece = piece

    def retry(self):
        self.curl.setopt(pycurl.RANGE, '% d - %d' % (self.piece[1] +
                                                     self.piece_downloaded, self.piece[2]))
        if self.link_downloaded:  # 上次连接中有数据返回？
            self.link_downloaded = 0
        else:
            self.retried += 1

    def close(self):
        self.curl.close()

    def write_cb(self, data):
        if self.piece:
            self.result.seek(self.piece[1] + self.piece_downloaded, 0)
            self.result.write(data)
            self.result.flush()
            size = len(data)
            self.link_downloaded += size
            self.piece_downloaded += size
            self.total_downloaded += size
        if self.is_stop:
            return -1

# FastDownload


class FastDownload:

    def __init__(self):
        file(DLOG, 'w')
        # logging.basicConfig(level=LOGLEVEL,
                            # format='[% (asctime)s][%(levelname)s] % (message)s',
                            # filename='download.log',
                            # filenmode='w')

        self.mcurl = pycurl.CurlMulti()

    def execute(self, url):
        self.url_info = self.url_check(url)
        if self.url_info:
            print 'Download % s, Size % d' % (self.url_info['file'],
                                              self.url_info['size'])
            self.pieces = self.make_pieces()
            self.allocate_space()
            self.download()

    # ***************************************************************

    def url_check(self, url):
        '''下载地址检查'''

        url_info = {}
        proto = urlparse.urlparse(url)[0]
        if proto not in VALIDPROTOCOL:
            print 'Valid protocol should be http or ftp, but % s found < %s >!' % (proto, url)
        else:
            ss = StringIO()
            curl = pycurl.Curl()
            curl.setopt(pycurl.FOLLOWLOCATION, 1)
            curl.setopt(pycurl.MAXREDIRS, 5)
            curl.setopt(pycurl.CONNECTTIMEOUT, 30)
            curl.setopt(pycurl.TIMEOUT, 300)
            curl.setopt(pycurl.NOSIGNAL, 1)
            curl.setopt(pycurl.NOPROGRESS, 1)
            curl.setopt(pycurl.NOBODY, 1)
            curl.setopt(pycurl.HEADERFUNCTION, ss.write)
            curl.setopt(pycurl.URL, url)

        try:
            curl.perform()
        except:
            pass

        if curl.errstr() == '' and curl.getinfo(pycurl.RESPONSE_CODE) in STATUS_OK:
            url_info['url'] = curl.getinfo(pycurl.EFFECTIVE_URL)
            url_info['file'] = os.path.split(url_info['url'])[1]
            url_info['size'] = int(
                curl.getinfo(pycurl.CONTENT_LENGTH_DOWNLOAD))
            url_info['partible'] = (ss.getvalue().find('Accept - Ranges') != -1)

        return url_info

    def make_pieces(self):
        '''分段信息生成'''
        if self.url_info['partible']:
            file_size = self.url_info['size']
            num = MAXCONCOUNT
            while num * MINPIECESIZE > file_size and num > 1:
                num -= 1
                piece_size = int(
                    round(self.url_info['size'] * 1.0 / num + 0.5))
                pieces = [[i, i * piece_size, (i + 1) * piece_size - 1] for i in
                          range(num)]
                pieces[-1][2] = self.url_info['size'] - 1
        else:
            pieces = [self.url_info['size']]
        return pieces

    def allocate_space(self):
        '''
        预分配文件空间(通用？)
        '''
        afile = file(self.url_info['file'], 'wb')
        afile.truncate(self.url_info['size'])
        afile.close()

    # ***************************************************************

    def show_progress(self, downloaded, elapsed):
        '''
        显示下载进度
        '''
        percent = min(100, downloaded * 100.0 / self.url_info['size'])
        if elapsed == 0:
            rate = 0
        else:
            rate = downloaded * 1.0 / 1024.0 / elapsed
            info = ' D / L:%d / %d ( % 6.2f%%) - Avg:%4.1fkB / s' % (downloaded, self.url_info['size'], percent, rate)
            space = ' ' * (60 - len(info))

            prog_len = int(percent * 20 / 100)
            prog = '|' + 'o' * prog_len + '.' * (20 - prog_len) + '|'

            sys.stdout.write(info + space + prog)
            sys.stdout.flush()
            sys.stdout.write('\b' * 82)

    def close_connection(self, c):
        '''
        关闭连接
        '''
        self.connections.remove(c)
        c.close()

    def process_curl(self, curl):
        '''
        下载结果处理
        '''
        self.mcurl.remove_handle(curl)
        c = curl.connection
        c.errno = curl.errno
        c.errmsg = curl.errmsg
        self.working_connections.remove(c)
        if c.errno == pycurl.E_OK:
            c.code = curl.getinfo(pycurl.RESPONSE_CODE)
            d = self.process_ok(c)
        else:
            d = self.process_error(c)
        return d

    def process_ok(self, c):
        '''
        下载成功处理
        '''
        if c.code in STATUS_OK:
            assert c.piece_downloaded == c.piece_size
            msg = '%s: Download successed' % c.name
            # logging.info(msg)
            msg = '%s:Download % s out of % d' % (c.name, c.piece_downloaded,
                                            c.piece_size)
            # logging.debug(msg)
            self.free_connections.append(c)
        elif c.code in STATUS_ERROR:
            msg = '%s:Error < %d >! Connection will be closed' % (c.name,
                                                            c.code)
            # logging.warning(msg)
            self.close_connection(c)
            self.pieces.append(c.piece)
        else:
            raise Exception('% s: Unhandled http status code % d' % (c.name, c.code))

    def process_error(self, c):
        '''
        下载失败处理
        '''
        msg = '%s:Download failed < %s >' % (c.name, c.errmsg)
        # logging.error(msg)
        if self.url_info['partible'] and c.retried < MAXRETRYCOUNT:
            c.retry()
            self.working_connections.append(c)
            self.mcurl.add_handle(c.curl)
            msg = '%s:Try again' % c.name
            # logging.warning(msg)
        else:
            raise Exception('Download abort~~')

    def download(self):
        '''
        下载主过程
        '''
        self.result = file(self.url_info['file'], 'r + b')
        self.connections = []
        for i in range(len(self.pieces)):
            c = Connection(self.url_info['url'])
            self.connections.append(c)
            self.free_connections = self.connections[:]
            self.working_connections = []

        ok = True
        start_time = time.time()
        try:
            while 1:
                while self.pieces and self.free_connections:
                    p = self.pieces.pop(0)
                    c = self.free_connections.pop(0)
                    c.start(self.result, p)
                    self.working_connections.append(c)
                    self.mcurl.add_handle(c.curl)
                    msg = '%s:Start downloading' % c.name
                    # logging.debug(msg)

                while 1:
                    ret, handles_num = self.mcurl.perform()
                    if ret != pycurl.E_CALL_MULTI_PERFORM:
                        break

                while 1:
                    queue_num, ok_list, err_list = self.mcurl.info_read()
                    for curl in ok_list:
                        curl.errno = pycurl.E_OK
                        curl.errmsg = ''
                        self.process_curl(curl)
                    for curl, errno, errmsg in err_list:
                        curl.errno = errno
                        curl.errmsg = errmsg
                        self.process_curl(curl)
                    if queue_num == 0:
                        break

                elapsed = time.time() - start_time
                downloaded = sum([c.total_downloaded for c in
                                  self.connections])
                self.show_progress(downloaded, elapsed)

                if not self.working_connections:
                    break

                self.mcurl.select(1.0)
        except:
            # logging.error('Error: ' + Traceback())
            ok = False
        finally:
            for c in self.connections:
                c.close()
                self.mcurl.close()

        if ok:
            msg = 'Download Successed! Total Elapsed % ds' % elapsed
        else:
            msg = 'Download Failed!'
        print '\n', msg
        # logging.info(msg)

if __name__ == '__main__':
    
    if len(sys.argv) > 1:
        url = sys.argv[1]
    else:
        url = 'http://www.python.org/ftp/python/2.5.2/python-2.5.2.msi'
        fd = FastDownload()
        fd.execute(url)
