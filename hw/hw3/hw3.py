'''
import matplotlib.pyplot as plt
import numpy as np

import paho.mqtt.client as paho
from time import sleep
import erpc
from hw3 import *
import sys



ax, ay, az = [], [], []

plt.ion()
fig, axes = plt.subplots(3, 1)
#plt.show()
def plot(ax, ay, az):
    t = np.linspace(1, 100, len(ax))
    #print("[plot()] ax = ", ax)
    axes[0].clear()
    axes[1].clear()
    axes[2].clear()

    axes[0].scatter(t,ax)
    axes[0].set_xlabel('Time')
    axes[0].set_ylabel('Amplitude')

    axes[1].scatter(t,ay)
    axes[1].set_xlabel('Time')
    axes[1].set_ylabel('Amplitude')

    axes[2].scatter(t,az)
    axes[2].set_xlabel('Time')
    axes[2].set_ylabel('Amplitude')

    fig.tight_layout()
    




# <MQTT>

mqttc = paho.Client()

# Settings for connection
# TODO: revise host to your IP
host = "192.168.137.1"
topic = "Mbed"



# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

    # b'(  -2,    2, 1006) \n\x00'

    s = msg.payload.decode('UTF-8')[1:-4].split(",")
    s = [int(i) for i in s]
    ax.append(s[0])
    ay.append(s[1])
    az.append(s[2])
    plot(ax, ay, az)
    

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)









# <eRPC>
if len(sys.argv) != 2:
    print("Usage: python led_test_client.py <serial port to use>")
    exit()

# Initialize all erpc infrastructure
xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
client = client.LEDBlinkServiceClient(client_mgr)

print("call start() to begin publishing")
client.start()

mqttc.loop_start()
sleep(5)
mqttc.loop_stop()

print("call stop() to stop publishing")
client.stop()



        
# <eRPC>        
# Loop forever, receiving messages
mqttc.loop_forever()        
'''
        




import matplotlib.pyplot as plt
import numpy as np

import paho.mqtt.client as paho
from time import sleep
import erpc
from hw3 import *
import sys




# <MQTT>

mqttc = paho.Client()

# Settings for connection
# TODO: revise host to your IP
host = "192.168.137.1"
topic = "Mbed"

accel_data = []

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")

    # b'(  -2,    2, 1006) \n\x00'

    s = msg.payload.decode('UTF-8')[1:-4]
    #print("s = ",s)
    s = s.split(",")
    #print("s = ",s)
    s = [int(i) for i in s]
    #print("s = ",s)
    
    accel_data.append(s)
    #print("accel_data = ", accel_data)

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)









# <eRPC>
if len(sys.argv) != 2:
    print("Usage: python led_test_client.py <serial port to use>")
    exit()

# Initialize all erpc infrastructure
xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
client = client.LEDBlinkServiceClient(client_mgr)

print("call start() to begin publishing")
client.start()

mqttc.loop_start()
sleep(8)
mqttc.loop_stop()

print("call stop() to stop publishing")
client.stop()





def post_process(accel_data):
    ax = []
    ay = []
    az = []
    for a in accel_data:
        ax.append(a[0])
        ay.append(a[1])
        az.append(a[2])
    return ax, ay, az



fig, axes = plt.subplots(3, 1)

def plot(accel_data):

    ax, ay, az = post_process(accel_data)
    t = np.linspace(1, 100, len(accel_data))

    axes[0].scatter(t,ax)
    axes[0].set_xlabel('Time')
    axes[0].set_ylabel('Amplitude')

    axes[1].scatter(t,ay)
    axes[1].set_xlabel('Time')
    axes[1].set_ylabel('Amplitude')

    axes[2].scatter(t,az)
    axes[2].set_xlabel('Time')
    axes[2].set_ylabel('Amplitude')

    plt.show()
    sleep(2)
    plt.close()

plot(accel_data)


        
# <eRPC>        
# Loop forever, receiving messages
mqttc.loop_forever()        
        
        
