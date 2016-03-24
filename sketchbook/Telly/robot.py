#!/usr/bin/env python

# Telly's Pi program, listening for commands over a TCP socket
#
# (c) Copyright 2016 Let's Robot.

import socket
import time
import select
import sys
import os
import argparse
import errno
import serial
import re

queue = None

class CommandQueue(object):
    slow = []
    fast = []
    fast_running = None
    slow_running = None

    class QueuedItem(object):
        def __init__(self, client, id, command):
            self.client = client
            self.id = id
            self.command = command

    def __init__(self, teensy):
        self.teensy = teensy

    def run_fast(self):
        if self.fast_running:
            return

        if not self.fast:
            return

        self.fast_running = self.fast.pop(0)
        self.teensy.write(self.fast_running.command)

    def run_slow(self):
        if self.slow_running:
            return

        if not self.slow:
            return

        self.slow_running = self.slow.pop(0)
        self.teensy.write(self.slow_running.command)

    def run_queue(self):
        self.run_fast()
        self.run_slow()

    def enqueue_fast(self, client, id, command):
        q = self.QueuedItem(client, id, command.rstrip())
        self.fast.append(q)
        if self.fast_running:
            q.client.write('%s QUEUED %s' % (q.id, q.command))
        self.run_queue()

    def enqueue_slow(self, client, id, command):
        q = self.QueuedItem(client, id, command.rstrip())
        self.slow.append(q)
        q.client.write('%s QUEUED %s' % (q.id, q.command))
        self.run_queue()

    def response(self, output):
        if output.startswith('ASYNC_'):
            output = re.sub(r'^ASYNC_', '', output)

            if not self.slow_running:
                print 'Received serial data when no command running?'
                return

            if output.startswith('RUNNING'):
                return

            self.slow_running.client.write('%s %s' % (self.slow_running.id, output))
            self.slow_running.client.write('')
            self.slow_running = None
            print

        elif re.match(r'^(OK|ERR) *', output):
            self.fast_running.client.write('%s %s' % (self.fast_running.id, output))
            self.fast_running.client.write('')
            self.fast_running = None
            print

        else:
            # For now, we can asume that any output without an "ASYNC" prefix
            # is associated with a fast command
            output = 'DEBUG ' + output
            self.fast_running.client.write('%s %s' % (self.fast_running.id, output))

        self.run_queue()

class Commands(object):
    readbuf = ''
    max_readbuf = 1024*16
    last_unassigned_id = 0

    def dispatch(self, words):
        translations = {
                        'f' : 'forward',
                        'b' : 'back',
                        'l' : 'left',
                        'r' : 'right',
                        'backward' : 'back',
                        'q' : 'queue',
                    }

        if not words:
            return

        if re.match(r'^\d+$', words[0]):
            id = words.pop(0)
        else:
            id = '%04d' % self.last_unassigned_id
            self.last_unassigned_id += 1

        if not words:
            return

        cmd, args = words[0], words[1:]
        cmd = translations.get(cmd, cmd)

        if hasattr(self, 'cmd_' + cmd):
            getattr(self, 'cmd_' + cmd)(id, args)
        else:
            self.unknown(cmd, id, args)

    def parse(self, prefix, buf):
        self.readbuf += buf

        while True:
            self.readbuf = self.readbuf.replace('\r', '')
            head, sep, tail = self.readbuf.partition('\n')
            if sep != '\n':
                break
            if head:
                print prefix, repr(head)
            self.dispatch(head.split())
            self.readbuf = tail

        if len(self.readbuf) > self.max_readbuf:
            print 'readbuf too large, closing'
            self.close()

    def unknown(self, cmd, id, args):
        self.write('%s ERR Unknown command' % id)
        print

    def cmd_queue(self, id, arg):
        fast = list(queue.fast)
        slow = list(queue.slow)

        if queue.fast_running: fast.insert(0, queue.fast_running)
        if queue.slow_running: slow.insert(0, queue.slow_running)

        for q in fast: self.write('%s DEBUG Fast: %s %s' % (id, q.id, repr(q.command)))
        for q in slow: self.write('%s DEBUG Slow: %s %s' % (id, q.id, repr(q.command)))
        self.write('%s OK' % id)
        self.write('')
        print

    def cmd_ping(self, id, arg):
        queue.enqueue_fast(self, id, 'ping')

    def cmd_hostname(self, id, arg):
        pass

    def cmd_quit(self, id, arg):
        self.close()
        print

    def cmd_forward(self, id, args):
        args.insert(0, 'forward')
        queue.enqueue_slow(self, id, ' '.join(args))

    def cmd_back(self, id, args):
        args.insert(0, 'back')
        queue.enqueue_slow(self, id, ' '.join(args))

    def cmd_left(self, id, args):
        args.insert(0, 'left')
        queue.enqueue_slow(self, id, ' '.join(args))

    def cmd_right(self, id, args):
        args.insert(0, 'right')
        queue.enqueue_slow(self, id, ' '.join(args))

    def cmd_led(self, id, args):
        translations = {
                        'white' : [ '255 255 255' ],
                        'red'   : [ '255 0 0'     ],
                        'green' : [ '0 255 0'     ],
                        'blue'  : [ '0 0 255'     ],
                        'black' : [ '0 0 0'       ],
                        'off'   : [ '0 0 0'       ],
                        'on'    : [ '255 255 255' ],
                    }

        if len(args) < 2:
            self.write('ERR not enough arguments')
            return

        if translations.has_key(args[1]):
            args[1:] = translations[args[1]]

        if args[0] == 'all':
            args[0] = '0xff'

        command = 'led ' + ' '.join(args)
        queue.enqueue_fast(self, id, command)

class Client(Commands):
    SO_RCVBUF = 1024*16
    SO_SNDBUF = 1024*16

    def __init__(self, fd_set, sock):
        print 'Connection from', sock.getpeername()
        sock.setblocking(False)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, self.SO_RCVBUF)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, self.SO_SNDBUF)

        self.sock = sock
        self.peername = sock.getpeername()
        self.fd_set = fd_set
        self.fd_set.add(self)
        self.closed = False

    def fileno(self):
        return self.sock.fileno()

    def close(self):
        if self.closed:
            return
        self.sock.close()
        if self in self.fd_set:
            self.fd_set.remove(self)
        self.closed = True

    def read(self):
        try:
            buf = self.sock.recv(self.max_readbuf)
            print "read %d: [%s]  [%s]" % (len(buf), repr(buf), repr(buf.strip('\0')))
        except socket.error as e:
            if e.errno == errno.ECONNRESET:
                print 'Connection reset by peer'
                self.close()
            else:
                print 'Unhandled socket error: ' + e
                self.close()
            return

        if not buf:
            print 'Client EOF'
            print
            self.close()
            return

        self.parse('   TCP <--', buf)

    def write(self, buf):
        print '   TCP -->', repr(buf)
        buf += '\n'

        try:
            sent = self.sock.send(buf)
            buf = buf[sent:]

        except socket.error as e:
            if e.errno in [errno.EAGAIN, errno.EWOULDBLOCK]:
                print 'SNDBUF full, closing', self.peername
                self.close()

            if e.errno == errno.ECONNRESET:
                print 'Connection reset by peer', self.peername
                self.close()

            else:
                print 'Unhandled socket error: %s: %s' % (self.peername, e)
                self.close()

        finally:
            if buf:
                print 'Partial write?  Closing', self.peername
                self.close()

class Stdin(Commands):
    def __init__(self, fd_set):
        fd_set.add(self)

    def fileno(self):
        return sys.stdin.fileno()

    def read(self):
        buf = sys.stdin.readline()

        if not buf:
            #print 'EOF from stdin, exiting'
            sys.exit(0)

        self.parse(' stdin <--', buf)

    def write(self, buf):
        print ' stdin -->', repr(buf)

    def close(self):
        sys.exit(0)

class SerialPort(object):
    readbuf = ''

    def __init__(self, fd_set, device, baud, sleep=1):
        self.serial = serial.Serial(device, baud, timeout=0)
        self.serial.nonblocking()
        print 'Opened %s, baud %d' % (device, baud)

        self.write('\r\n!\r\necho off\r\n')
        time.sleep(sleep)
        self.serial.flushInput()
        fd_set.add(self)

    def fileno(self):
        return self.serial.fileno()

    def read(self):
        self.readbuf += self.serial.read(1024*16)
        self.parse()

    def write(self, buf):
        print 'Teensy -->', repr(buf)
        self.serial.write(buf + '\r\n')

    def parse(self):
        global queue
        while True:
            self.readbuf = self.readbuf.replace('\r', '')
            head, sep, tail = self.readbuf.partition('\n')
            if sep != '\n':
                break
            print 'Teensy <--', repr(head)
            queue.response(head)
            self.readbuf = tail

        if len(self.readbuf) > 256:
            print 'Something seems wrong, serial readbuf is', len(self.readbuf), ':', repr(self.readbuf)

class Listener(object):
    def __init__(self, fd_set, port=0, host='', backlog=4):
        sock = socket.socket()
        sock.setblocking(False)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        sock.listen(backlog)
        self.sock = sock
        self.fd_set = fd_set
        self.fd_set.add(self)
        print 'Listening on', self.sock.getsockname()

    def fileno(self):
        return self.sock.fileno()

    def read(self):
        ret = self.sock.accept()

        if ret is None:
            return None

        sock, addr = ret
        con = Client(fd_set, sock)
        print "Created", con

def unbuffer_stdout():
    # Unbuffer stdout (simulating python's "-u" flag)
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)

    # Redirect stderror to stdout
    old = sys.stderr
    sys.stderr = sys.stdout
    old.close()

if __name__ == '__main__':
    unbuffer_stdout()

    parser = argparse.ArgumentParser()
    parser.add_argument('--port',  '-p', type=int, default=27762,          help='TCP Port to listen on')
    parser.add_argument('--line',  '-l', type=str, default='/dev/ttyACM0', help='Serial device')
    parser.add_argument('--baud',  '-s', type=int, default=19200,          help='BAUD rate')
    args = parser.parse_args()

    fd_set = set()

    Listener(fd_set, args.port)
    Stdin(fd_set)

    teensy = SerialPort(fd_set, args.line, args.baud, 0.2)
    queue = CommandQueue(teensy)

    print

    while True:
        readable, _, _ = select.select(list(fd_set), [], [], 1)
        for i in readable:
            i.read()
