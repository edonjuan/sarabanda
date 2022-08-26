from datetime import datetime
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
import serial, time


puertoSerial = serial.Serial('/dev/ttyUSB1', 9600)
time.sleep(2)

# nodos máximos en la red mesh
limite = 4

medidas = ("temp=", "hum=", "mov=", "luz=", "gra=", "grb=")
sequence = []
ciclo = 0
devices = 0

# You can generate an API token from the "API Tokens Tab" in the UI
token = "EAu_F7W_qYllj1-vWsJ757LNEoVMyRx3qcEMK_gbK0ZoiH9r4iAgHwsx0mQJYuadUKS6ULZWsSHZno-ZCer7mA=="
org = "UTEQ"
bucket = "mesh"

def countSetBits(n):
    if (n == 0):
        return 0
    else:
        return (n & 1) + countSetBits(n >> 1)

with InfluxDBClient(url="127.0.0.1:8086", token=token, org=org) as client:
    write_api = client.write_api(write_options=SYNCHRONOUS)

    while 1:
        print("Inicio del while")
        try:
            datos = str(puertoSerial.readline())
            print("leyendo datos del puerto serie")

            separados = datos.split(",")
            print("datos separados por coma..")
            ciclo = ciclo + 1

            if(separados[0] == "b'3822979383"):
                print("bravo", end="-> ")
                separados[0]="bravo"
                if((devices & 1) == 1):
                    ciclo = limite
                else:
                    devices = devices | 1

            elif(separados[0] == "b'2440612283"):
                print("charly", end="-> ")
                separados[0]="charly"
                if((devices & 2) == 2):
                    ciclo = limite
                else:
                    devices = devices | 2

            elif(separados[0] == "b'3257178212"):
                print("delta", end="-> ")
                separados[0]="delta"
                if((devices & 4) == 4):
                    ciclo = limite
                else:
                    devices = devices | 4

            elif(separados[0] == "b'3257177579"):
                print("echo", end="-> ")
                separados[0]="echo"
                if((devices & 8) == 8):
                    ciclo = limite
                else:
                    devices = devices | 8

            sequence.clear()
            print("guardando datos en secuencia..")
            for x in range(1, 6):
                if(separados[x] != ""):
                    sequence.append(separados[0] + ",host=rpy3 " + medidas[x-1] + separados[x])
                else:
                    separados[x] = None

            if(ciclo>=limite):
                ciclo=0
                sequence.append("devices,host=rpy3 conectados=" + str(countSetBits(devices)))
                devices=0

            print (sequence)

            write_api.write(bucket, org, sequence)
            print ("insertados en base de datos")

        except KeyboardInterrupt:
            break
        except:
            print("Unknow data")

    puertoSerial.close()
client.close()
