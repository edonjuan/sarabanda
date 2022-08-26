import serial, time
puertoSerial = serial.Serial('/dev/ttyUSB0', 9600)
time.sleep(2)


while 1:
    try:
        datos = str(puertoSerial.readline())
        print(datos)
    except:
        print("closing port...")
        puertoSerial.close()
        break
