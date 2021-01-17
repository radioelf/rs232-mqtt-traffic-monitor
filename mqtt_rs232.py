'''
Recepcion de topics a traves de MQTT de un ESP-01, para monitorizar el trafico RS-232 remotamente
'''

import ssl
import sys
import os
 
import paho.mqtt.client

broker_address = "localhost"
broker_port = 1883
topic = "ESP-a7267/#"
    
def on_message(client, userdata, message):
    rxpayload =  str(message.payload)
    if message.payload =="online":
        print("Conectado")
    else:
        print ('RX: %s' % message.payload)

def on_connect(client, userdata, flags, rc):
    print ("Escuchando %s" %topic)
    client.subscribe(topic)

def main():
    print ("Ctrl+c para salir")
    print ("Inicializando..")
    client = paho.mqtt.client.Client(client_id='Serial-monitor', clean_session=False)
    client.on_connect = on_connect
    client.on_message = on_message
    client.username_pw_set("usuario", "contrasenya")
    client.connect(host=broker_address, port=broker_port)
    
    client.loop_forever()
 
if __name__ == '__main__':
    main()


sys.exit(0)
