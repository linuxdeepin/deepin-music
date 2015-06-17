#author:ted
#version:windows, python 2.7

from os.path import basename
from urlparse import urlsplit
import urllib2
import threading
import sys,os,time


class WorkerThread(threading.Thread):
    def __init__(self, nm, url, filename, ranges=0):
        threading.Thread.__init__(self, name=nm)
        self.url = url
        self.filename = filename
        self.range = ranges
        self.size = ranges[1] - ranges[0] +1
        self.down = 0
        self.percent = self.down/float(self.size)*100
        self.headerrange = (self.range[0]+self.down, self.range[1])
        self.bufferSize = 8192


    def run(self):

        alldone = False
        retries = 1
        while not alldone:
            if retries > 10:
                break
            try:
                self.headerrange = (self.range[0]+self.down, self.range[1])
                request = urllib2.Request(self.url)
                request.add_header('Range', 'bytes=%d-%d' %self.headerrange)
                response = urllib2.urlopen(request)
                data = response.read(self.bufferSize)
                f = open(self.filename, 'ab')
                while data:
                    f.write(data)
                    self.down += len(data)
                    self.percent = self.down/float(self.size) *100             
                    data = response.read(self.bufferSize)
                alldone = True
                f.close()
            except Exception, err:
                retries += 1
                time.sleep(1)
                continue

def JobAssign(size, workers ):
    """
        size : total size of download job
        workers: number of working threads
        return: job for each one worker
    """
    assert workers > 0
    joblist = []
    workload  = size / workers
    for i in range(workers -1):
        joblist.append((i*workload, i*workload + workload -1))
    joblist.append((workload*(workers-1), size -1))              
    return joblist


def GetFileSize(url):
    length = 0
    try:
        r = urllib2.urlopen(url,timeout=5)
        info = r.info()
        if 'Content-Length' in info:
            length = int(info['Content-Length'])
    except urllib2.URLError,err:
        print err
    return length

def CheckWorker(work_queue):
    for worker in work_queue:
        if worker.isAlive():
            return True
    return False

def Dig(url, filename=None, workers= None):
    if filename == None:
        filename = basename(urlsplit(url)[2])
    if workers == None:
        workers = 1

    total_size = GetFileSize(url)
    if total_size == 0:
        raise URLUnreachable
    ranges = JobAssign(total_size, workers)
    names = ["%s_%d" %("tmp",i) for i in range(workers)]
      
    worker_queue =[]
    for i in range(1,workers+1):
        w = WorkerThread(i,url,names[i-1],ranges[i-1])
        w.setDaemon(True)
        w.start()
        worker_queue.append(w)
    
    working = CheckWorker(worker_queue)
    time_diff = 0
    start_pos = [worker.down for worker in worker_queue]
    start_time = time.time()
    while working:
        try:
            end_pos = [worker.down for worker in worker_queue]
            time_diff = time.time() - start_time

            for i,worker in enumerate(worker_queue):
                down = end_pos[i] -start_pos[i]
                try:
                    rate = down/float(time_diff)/1024
                except:
                    rate = 100.0
                report ='(%.2fM) %s has down: %.2f ,Avg rate: %.1fKB/s\n' \
                            %(worker.size/1024.0/1024.0, worker.getName(),worker.percent, rate)
                sys.stdout.write(report)
                sys.stdout.flush()
            time.sleep(0.5)
            os.system("cls")
            working = CheckWorker(worker_queue)
        except KeyboardInterrupt:
            print "exit...."
            fileobj.close()
            sys.exit(1)

    here = os.path.dirname(__file__)
    filepath = os.path.join(here,filename)
    f = open(filepath, 'wb')
    for n in names:
        f.write(open(n,'rb').read())
        try:
            os.remove(n)
        except:
            pass
    f.close()
    print "down !"


                   
if __name__ == '__main__':
    url=r"http://118.228.18.61/youku/6772195ABB43582F807466584C/0300020100514FD6C38EF5078CE90538A8446C-30D0-7AEF-592F-2A7AF2D6301C.flv"
    Dig(url,workers= 6)
