# BRONNEN
  # bewaren van data in een txt bestand: https://www.tutorialspoint.com/python/python_files_io.htm (16/05/2025)
  # datatime datasheet: https://docs.python.org/3/library/datetime.html (16/05/2025)

  # het lezen van een serial port met python om de gegevens van de ESP32 binnen te halen: een vorig integratieproject waar ik aan gewerkt heb: https://github.com/ChaOsRul3z/SmartFridge (16/05/2025)
  # het openen van een serial port met python: https://pyserial.readthedocs.io/en/latest/shortintro.html (16/05/2025)
  # het automatisch vinden en connecteren met een serial port: ChatGPT: https://chatgpt.com/share/68272b62-f2cc-800c-b31b-2c800d6e2f0b (16/05/2025)

import time
from datetime import datetime

import serial
import serial.tools.list_ports

DATETIME_FORMAT = "%d-%m-%Y %H:%M"  # de vorm van hoe tijdstippen worden opgeslagen

DEVICE_TYPE = "USB"

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

def connect_port():
  ports = serial.tools.list_ports.comports()

  attempted_logs = []

  for port in ports:
    if DEVICE_TYPE.lower() in port.description.lower():
      try:
        serial_connection = serial.Serial(port.device, 9600)
        return serial_connection
      except serial.SerialException as s:
        attempted_logs.append({"connection_logs": s})
      except Exception as e:
        attempted_logs.append({"connection_logs": e})
  
  store_data("connection_logs", f"Geen geschikte COM-poort gevonden: [{attempted_logs}]")
  return None

def collect_data():
  ser = None
  connection_timer = 0.5

  while True:
    while (ser == None):
      ser = connect_port()
      time.sleep(connection_timer)
      continue

    log = []

    if (ser.is_open):
      # legenda: START@TEMP_CORE$coreTemp&HUMIDITY_CORE$coreHumid&TEMP_WALL$wallTemp&HUMIDITY_WALL$wallHumid&TEMP_OUT$outsideTemp&HUMIDITY_OUT$outsideHumid&MASS$massa@END
      try:
        raw_serial_data = ser.readline()
      except Exception as e:
        store_data("error_logs", e)
        connection_timer = 60.0
        ser = None
        continue

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