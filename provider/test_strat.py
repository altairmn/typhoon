import sys
import time
import zmq

context = zmq.Context()

# Socket to receive messages on
receiver = context.socket(zmq.PULL)
receiver.connect("tcp://localhost:5560")

# Wait for start of batch
s = receiver.recv()

# Start our clock now
tstart = time.time()

# Process 100 confirmations
while True:
#for task_nbr in range(100):
    s = receiver.recv()
    print(s)
    # if task_nbr % 10 == 0:
    #     sys.stdout.write(':')
    # else:
    #     sys.stdout.write('.')
    # sys.stdout.flush()

# Calculate and report duration of batch
tend = time.time()
print("Total elapsed time: %d msec" % ((tend-tstart)*1000))
