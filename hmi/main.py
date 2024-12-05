from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QWidget
from PyQt5.uic import loadUi
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice, QPoint
from PyQt5 import QtCore, QtWidgets
import pyqtgraph as pg
import numpy as np
import sys
from PyQt5.QtGui import QKeyEvent
from PyQt5.QtCore import Qt
import threading
from PyQt5.QtWebEngineWidgets import QWebEngineView
import csv

csv_file = "data.csv"

with open(csv_file, 'w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['Temp1', 'Hum1', 'Temp2', 'Hum2', 'Lat1','Long1', 'Lat2', 'Lon2'])


class myApp(QMainWindow):
    def __init__(self):
        super(myApp, self).__init__()
        loadUi("hmi.ui", self)
        self.setWindowTitle("HMI-Swarm Robotics")

        self.setWindowFlag(QtCore.Qt.FramelessWindowHint)
        self.setWindowOpacity(1)
        self.setWindowFlags(QtCore.Qt.FramelessWindowHint)
        self.setAttribute(QtCore.Qt.WA_TranslucentBackground)

        self.active_keys = set()  # Almacena las teclas actualmente presionadas



        #*Iniciamos el grip para redimensionar la ventana
        self.gripsize = 10
        self.grip= QtWidgets.QSizeGrip(self)
        self.grip.resize(self.gripsize,self.gripsize)


        self.serial = QSerialPort()
        self.bt_update.clicked.connect(self.read_ports)
        self.bt_connect.clicked.connect(self.serial_connect)
        self.baudrate=115200

        self.serial.readyRead.connect(self.read_data)

        self.x=list(np.linspace(0,100,100))
        self.y=list(np.linspace(0,0,100))
        self.temp1=list(np.linspace(0,0,100))
        self.temp2=list(np.linspace(0,0,100))
        self.hum1=list(np.linspace(0,0,100))
        self.hum2=list(np.linspace(0,0,100))
        # self.lat1=list(np.linspace(0,0,100))
        # self.lat2=list(np.linspace(0,0,100))
        # self.lon1=list(np.linspace(0,0,100))
        # self.lon2=list(np.linspace(0,0,100))

        pg.setConfigOption('background', '#7A1CAC')
        pg.setConfigOption('foreground', '#ffffff')
        
        #% Plot Temperature
        self.plt_temp=pg.PlotWidget(title="Temperature")
        self.temp_layout.addWidget(self.plt_temp)
        self.plt_temp.setYRange(-12,50)
        self.plt_temp.setXRange(0,100)
        self.plt_temp.showGrid(x=False,y=True)
        self.plt_temp.setLabel('left', "Temperature", units='°C')
        self.plt_temp.setLabel('bottom', "Time", units='s')
        self.plt_temp.addLegend()
        self.plt_temp.setMouseEnabled(x=False,y=True)

        #% Plot Humidity
        self.plt_hum=pg.PlotWidget(title="Humidity")
        self.hum_layout.addWidget(self.plt_hum)
        self.plt_hum.setYRange(-10,110)
        self.plt_hum.setXRange(0,100)
        self.plt_hum.showGrid(x=False,y=True)
        self.plt_hum.setLabel('left', "Angle", units='%')
        self.plt_hum.setLabel('bottom', "Time", units='s')
        self.plt_hum.addLegend()
        self.plt_hum.setMouseEnabled(x=False,y=True)


        self.read_ports()

        #% Map
        self.map_view = QWebEngineView()
        self.map_layout.addWidget(self.map_view)
        self.map_view.setUrl(QtCore.QUrl("https://www.google.com/maps/@0,0,2z"))

        
        


    

        


    def keyPressEvent(self, event):
        key = event.key()
        if (event.key() == Qt.Key_Escape and event.modifiers() == Qt.ControlModifier)or (event.key() == Qt.Key_Q and event.modifiers() == Qt.ControlModifier):
            self.close()
        if key == Qt.Key_W:
            self.serial.write(b'f')
        elif key == Qt.Key_S:
            self.serial.write(b'b')
        elif key == Qt.Key_D:
            self.serial.write(b'r')
        elif key == Qt.Key_A:
            self.serial.write(b'l')
        elif key == Qt.Key_8:
            self.serial.write(b'z')
        elif key == Qt.Key_2:
            self.serial.write(b'x')
        elif key == Qt.Key_6:
            self.serial.write(b'v')
        elif key == Qt.Key_4:
            self.serial.write(b'c')

    def keyReleaseEvent(self, event):
        key = event.key()
        if key == Qt.Key_W:
            self.serial.write(b's')
        elif key == Qt.Key_S:
            self.serial.write(b's')
        elif key == Qt.Key_A:
            self.serial.write(b's')
        elif key == Qt.Key_D:
            self.serial.write(b's')
        elif key == Qt.Key_8:
            self.serial.write(b's')
        elif key == Qt.Key_2:
            self.serial.write(b's')
        elif key == Qt.Key_4:
            self.serial.write(b's')
        elif key == Qt.Key_6:
            self.serial.write(b's')


 

    def read_ports(self):
        portList=[]
        ports = QSerialPortInfo.availablePorts()
        for port in ports:
            portList.append(port.portName())
        self.cb_list_ports.clear()
        self.cb_list_ports.addItems(portList)

    def serial_connect(self):
        self.serial.waitForReadyRead(10)
        self.port=self.cb_list_ports.currentText()
        self.baud=self.baudrate
        self.serial.setBaudRate(self.baud)
        self.serial.setPortName(self.port)
        self.serial.open(QIODevice.ReadWrite)





    def read_data(self):

        if not self.serial.canReadLine(): return
        
        rx = self.serial.readAll()
        x = str(rx, "utf-8").split('\r')[-2]
        data_dict = split_data(x)
        print(data_dict)
        self.lat1=data_dict['La1']
        self.lat2=data_dict['La2']
        self.lon1=data_dict['Lo1']
        self.lon2=data_dict['Lo2']
        self.lat_val_1.setText(self.lat1)
        self.lat_val_2.setText(self.lat2)
        self.lon_val_1.setText(self.lon1)
        self.lon_val_2.setText(self.lon2)




        try:
            self.temp1=self.temp1[1:]
            self.temp1.append(float(data_dict['T1']))
            self.temp2=self.temp2[1:]
            self.temp2.append(float(data_dict['T2']))
            self.plt_temp.clear()
            self.plt_temp.plot(self.x,self.temp1,pen=pg.mkPen(color='#9EDF9C',width=2),name="Temperature 1")
            self.plt_temp.plot(self.x,self.temp2,pen=pg.mkPen(color='#000000',width=2),name="Temperature 2")


            self.hum1=self.hum1[1:]
            self.hum1.append(float(data_dict['H1']))
            self.hum2=self.hum2[1:]
            self.hum2.append(float(data_dict['H2']))
            self.plt_hum.clear()
            self.plt_hum.plot(self.x,self.hum1,pen=pg.mkPen(color='#9EDF9C',width=2),name="Humidity 1")
            self.plt_hum.plot(self.x,self.hum2,pen=pg.mkPen(color='#000000',width=2),name="Humidity 2")




        except Exception:
            self.x=list(np.linspace(0,100,100))
            self.temp1=list(np.linspace(0,0,100))
            self.temp2=list(np.linspace(0,0,100))
            self.hum1=list(np.linspace(0,0,100))
            self.hum2=list(np.linspace(0,0,100))

            print(Exception)
        #* Guardar datos en nueva fila del archivo csv
        with open(csv_file, mode='a') as file:
            writer = csv.writer(file)
            writer.writerow([data_dict['T1'], data_dict['H1'], data_dict['T2'], data_dict['H2'], data_dict['La1'], data_dict['Lo1'], data_dict['La2'], data_dict['Lo2']])
        

            
def split_data(date):
#%El formato de los datos es "AX:0.00,AY:0.00,AZ:0.00,GX:0.00,GY:0.00,GZ:0.00,T:0.00,P:0.00,A:0.00\r\n"
#*Se debe de agregar las opciones para GPS de carga primaria y secundaria.

    splited= {}
    for data in date.split(","):
        try:
            splited[data.split(":")[0]]=data.split(":")[1]
            
            
            

        except IndexError:
            pass
    return splited
        
        

    
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = myApp()
    window.show()
    sys.exit(app.exec_())