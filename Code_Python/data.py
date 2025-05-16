# BRONNEN
  # bewaren van data in een txt bestand: https://www.tutorialspoint.com/python/python_files_io.htm (16/05/2025)

  # het lezen van een serial port met python om de gegevens van de ESP32 binnen te halen: een vorig integratieproject waar ik aan gewerkt heb: https://github.com/ChaOsRul3z/SmartFridge (16/05/2025)
  # het openen van een serial port met python: https://pyserial.readthedocs.io/en/latest/shortintro.html (16/05/2025)

import serial
import time

FIREBEETLTE_PORT = "COM1"
ser = None

def store_data():
  while True:
    while ser == None:
      ser = serial.Serial(FIREBEETLTE_PORT, 9600)
      time.sleep(0.5)
      continue

    if (ser.is_open):
      raw_serial_data = ser.readline()
      serial_data = raw_serial_data.decode().strip()

      # Open a file  
      fo = open("data.txt", "a")
      fo.write(serial_data)

      # Close opened file
      fo.close()