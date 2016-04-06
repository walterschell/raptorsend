import socket
import sys
from multiprocessing import Process, Queue
import struct

def process_data_q(q):
    while True:
        packet = q.get()
        address = packet[1]
        buff = packet[0]
        header_size = 4 + 8 + 4 + 4
        total_size, common, scheme_specific, symbol_id =\
            struct.unpack('=IQII', buff[0:header_size])
        filename = '%s_%d.%d.%d' % (address[0], address[1],total_size, symbol_id)
        with open(filename, 'wb') as out:
            out.write(buff)

def main():
    if len(sys.argv) != 2:
        print 'Usage: %s <port>' % sys.argv[0]
        return
    port = int(sys.argv[1])
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('', port))
    q = Queue()
    save_proc = Process(target=process_data_q, args=(q,))
    save_proc.start()
    while True:
        packet = s.recvfrom(2048)
        q.put(packet)

if __name__ == '__main__':
    main()
