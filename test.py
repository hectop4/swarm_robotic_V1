import sys
import serial
import threading
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QWidget
from PyQt5.QtGui import QKeyEvent
from PyQt5.QtCore import Qt

# Configurar la conexión serial
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0)  # Cambia '/dev/ttyUSB0' por tu puerto serial
data_dict = {}  # Diccionario para almacenar los datos leídos
class KeyboardEventApp(QMainWindow):
    def __init__(self): 
        super().__init__()

        self.setWindowTitle("Swarm Robotics Keyboard Control")
        self.setGeometry(100, 100, 400, 200)

        self.central_widget = QWidget(self)
        self.setCentralWidget(self.central_widget)

        self.key_label = QLabel("Last Key Pressed: None", self.central_widget)
        self.key_label.setGeometry(10, 10, 300, 30)

        self.active_keys = set()  # Almacena las teclas actualmente presionadas

        # Inicia un hilo para leer desde el puerto serial
        self.serial_thread = threading.Thread(target=self.read_serial, daemon=True)
        self.serial_thread.start()

    def keyPressEvent(self, event):
        if isinstance(event, QKeyEvent):
            key_code = event.key()
            if key_code not in self.active_keys:  # Evita repetición al mantener presionada
                self.active_keys.add(key_code)
                self.handle_key_action(key_code, pressed=True)

    def keyReleaseEvent(self, event):
        if isinstance(event, QKeyEvent):
            key_code = event.key()
            if key_code in self.active_keys:
                self.active_keys.remove(key_code)
                self.handle_key_action(key_code, pressed=False)

    def handle_key_action(self, key_code, pressed):
        """
        Manejador de acciones de teclas.
        Envia los comandos seriales y actualiza la etiqueta.
        """
        command = None
        if key_code == Qt.Key_W:
            command = b'f' if pressed else b's'
            key_text = "W"
        elif key_code == Qt.Key_A:
            command = b'l' if pressed else b's'
            key_text = "A"
        elif key_code == Qt.Key_S:
            command = b'b' if pressed else b's'
            key_text = "S"
        elif key_code == Qt.Key_D:
            command = b'r' if pressed else b's'
            key_text = "D"
        elif key_code == Qt.Key_Left:
            command = b'v' if pressed else b's'
            key_text = "Left Arrow"
        elif key_code == Qt.Key_Right:
            command = b'c' if pressed else b's'
            key_text = "Right Arrow"
        elif key_code == Qt.Key_Up:
            command = b'z' if pressed else b's'
            key_text = "Up Arrow"
        elif key_code == Qt.Key_Down:
            command = b'x' if pressed else b's'
            key_text = "Down Arrow"
        elif key_code == Qt.Key_Space:
            command = b's' if pressed else b's'  # Detener con la barra espaciadora
            key_text = "Space Bar"
        else:
            key_text = "Unknown Key"

        if command:
            ser.write(command)

        state = "Pressed" if pressed else "Released"
        self.key_label.setText(f"Key {state}: {key_text}")

    def read_serial(self):
        """
        Lee datos desde el puerto serial, procesa líneas en un diccionario con valores flotantes,
        y las imprime en la consola.
        """
        buffer = ""  # Buffer temporal para acumular datos
        while True:
            if ser.in_waiting > 0:  # Verifica si hay datos disponibles
                # Leer los datos disponibles
                data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')  
                buffer += data  # Agregar los datos leídos al buffer

                # Procesar líneas completas del buffer
                while '\n' in buffer:  
                    line, buffer = buffer.split('\n', 1)  # Separar la línea completa
                    line = line.strip()  # Eliminar espacios en blanco y caracteres extra
                    if line:  # Asegurarse de que la línea no esté vacía
                        try:
                            # Dividir en elementos, convertir valores a float y construir el diccionario
                            data_dict = {
                                key.strip(): float(value.strip()) 
                                for key, value in (item.split(':') for item in line.split(','))
                            }
                            print(f"Parsed Data: {data_dict}")
                        except ValueError as e:
                            print(f"Error converting values to float: {line} - {e}")
                        except Exception as e:
                            print(f"Error parsing line: {line} - {e}")




def main():
    app = QApplication(sys.argv)
    window = KeyboardEventApp()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
