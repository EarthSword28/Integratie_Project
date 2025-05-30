# BRONNEN
  # bewaren van data in een txt bestand: https://www.tutorialspoint.com/python/python_files_io.htm (16/05/2025)
  # datatime datasheet: https://docs.python.org/3/library/datetime.html (16/05/2025)

  # het lezen van een serial port met python om de gegevens van de ESP32 binnen te halen: een vorig integratieproject waar ik aan gewerkt heb: https://github.com/ChaOsRul3z/SmartFridge (16/05/2025)
  # het openen van een serial port met python: https://pyserial.readthedocs.io/en/latest/shortintro.html (16/05/2025)

import serial
import time
from datetime import datetime

FIREBEETLTE_PORT = "/dev/ttyACM0"

DATETIME_FORMAT = "%d-%m-%Y %H:%M"  # de vorm van hoe tijdstippen worden opgeslagen

def get_current_time():
  try:
    current_time = datetime.now()  # haal de huidige datum op
    return current_time.strftime(DATETIME_FORMAT)
  except Exception as e:
    return f"ERROR: {e}"

def store_data(file:str, log):
  log_time = get_current_time()
  log_data = f"{log_time} [{log}]\n"

  fo = open(f"{file}.txt", "a")   # Open een bestand
  fo.write(log_data)              # Bewaar de log
  fo.close()                      # Sluit het geopend bestand

def collect_data():
  ser = None

  while True:
    while (ser == None):
      try:
        ser = serial.Serial(FIREBEETLTE_PORT, 9600)
      except Exception as e:
        store_data("error_logs", e)
        time.sleep(0.5)
        continue

    log = []

    if (ser.is_open):
      # legenda: START@TEMP_CORE$coreTemp&HUMIDITY_CORE$coreHumid&TEMP_WALL$wallTemp&HUMIDITY_WALL$wallHumid&TEMP_OUT$outsideTemp&HUMIDITY_OUT$outsideHumid&MASS$massa@END
      raw_serial_data = ser.readline()
      serial_data = raw_serial_data.decode().strip()

      if (serial_data.count("@") >= 2) and (serial_data.count("&") >= 1):
        split_data = serial_data.split("@")
        split_data_values = split_data[1].split("&")
      else:
        continue

      for data in split_data_values:
        try:
          variable = data.split("$")
          log_variable = f"{variable[0]}: {variable[1]}"
        except Exception as e:
          log_variable = f"ERROR: {e} {variable}"
        log.append(log_variable)

      log_data = ""
      for data in log:
        if log_data == "":
          log_data = data
        else:
          log_data = f"{log_data} | {data}"
      
      store_data("data", log_data)

collect_data()