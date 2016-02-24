#!/usr/bin/env python
import socket
import time
import argparse
import sys


def main():


    """
    Parse the input arguments into arg
    """
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--voltage', help='peak to peak voltage',type=float,default=2.0)
    parser.add_argument('--delay', help='peak to peak voltage',type=float,default=.5)
    parser.add_argument('--numcycles', help='number of cycles in a burst',type=float,default=10)
    args=parser.parse_args()

    TCP_IP = '192.168.1.250'
    TCP_PORT = 5025
    BUFFER_SIZE = 64

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((TCP_IP, TCP_PORT))

    s.send('voltage %f\n'%(args.voltage))
    s.send('voltage?\n')
    d=s.recv(BUFFER_SIZE)
    print(d)
    sys.stdout.flush()

    s.send('burst:ncycles %f\n'%args.numcycles)
    s.send('burst:ncycles?\n')
    d=s.recv(BUFFER_SIZE)
    print(d)
    sys.stdout.flush()

    while 1:
        s.send("trigger\n")
        time.sleep(args.delay)

    #data = s.recv(1024)
    s.close()

    #print("received data:", data)


if __name__ == "__main__":
    main()
