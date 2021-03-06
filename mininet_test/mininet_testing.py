

from mininet.net import Mininet
from mininet.cli import CLI
from mininet.util import pmonitor
from mininet.link import TCLink
import time

# bandwidth 
BW = 1000

popens = {}

# topology
net = Mininet(link=TCLink)

# client hosts
h = {}
for i in range(1,17):
    h[i] = net.addHost('h'+str(i))

# server host
h17 = net.addHost('h17')

s = {}
# switches
for i in range(1,5):
    s[i] = net.addSwitch('s'+str(i))

s5 = net.addSwitch('s5')

# controller
c0 = net.addController('c0')

# links
net.addLink(h17, s5, bw=BW)

for i in range(1,5):
    net.addLink(s5, s[i], bw=BW/4)
    for j in range(1,5):
        net.addLink(h[(4*(i-1))+j], s[i], bw=BW/16)

net.start()

# run scripts on hosts

popens[h17] = h17.popen("./server_src/server " + str(h17.IP()))

for i in range(1,17):
    popens[h[i]] = h[i].popen("./client_src/client " + str(h17.IP()) + " 8080 mininet_test/workloads/workload" + str(i-1) + ".txt")


time.sleep(13)

# monitoring outputs of scripts
for hst, line in pmonitor(popens, timeoutms=500):
    if hst:
        if "nvalid" in line:
            continue
        print(str(hst.name) +": " + str(line), end = '')
    else:
        break

# stopping
net.stop()