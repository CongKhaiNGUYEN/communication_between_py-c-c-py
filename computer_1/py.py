from genericpath import exists
import os
import threading



def handle_recv():
    input_pipe = "CTOPYTHON"
    os.mkfifo(input_pipe)
    while True:
        fifo = os.open(input_pipe, 0o640)
        stri=os.read(fifo, 80)
        stri = stri.decode('utf-8')
        if stri=='exit' or stri=='end':
            # os.system('rm CTOPYTHON')
            # os.system('rm PYTHONTOC')
            break
        print("\nReceive the string:  ", stri)
    # fifo.close()

def handle_send():
    path = "PYTHONTOC"
    os.mkfifo(path)
    while True:
        fifo=open(path,'w')
        string=input("\nEnter String:\t ")
        string = string + '\n\0'
        fifo.write(string)
        # for c in string: print(c)
        print('sent to c')
        if (string=='end' or string=='exit'):
            # os.system('rm CTOPYTHON')
            # os.system('rm PYTHONTOC')
            break
    # fifo.close()
def main():


    t1 = threading.Thread(target=handle_send)
    t2 = threading.Thread(target=handle_recv)

    t1.start()
    t2.start()

    t1.join()
    t2.join()


if __name__ == "__main__":
    main()
