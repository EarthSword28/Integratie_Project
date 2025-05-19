# BRONNEN
  # bewaren van data in een txt bestand: https://www.tutorialspoint.com/python/python_files_io.htm (16/05/2025)
  # datatime datasheet: https://docs.python.org/3/library/datetime.html (16/05/2025)

  # het lezen van een serial port met python om de gegevens van de ESP32 binnen te halen: een vorig integratieproject waar ik aan gewerkt heb: https://github.com/ChaOsRul3z/SmartFridge (16/05/2025)
  # het openen van een serial port met python: https://pyserial.readthedocs.io/en/latest/shortintro.html (16/05/2025)
  # het automatisch vinden en connecteren met een serial port: ChatGPT: https://chatgpt.com/share/68272b62-f2cc-800c-b31b-2c800d6e2f0b (16/05/2025)

import time
from datetime import datetime, timezone # timezone toegevoegd voor UTC optie

import serial
import serial.tools.list_ports

DATETIME_FORMAT = "%d-%m-%Y %H:%M"  # Formaat: DD-MM-YYYY HH:MM (lokale systeemtijd)
DEVICE_TYPE = "USB" # Deel van de poortbeschrijving om naar te zoeken

def get_current_time(use_utc=False): # Parameter toegevoegd voor UTC
  """
  Haalt de huidige tijd op.
  :param use_utc: Indien True, retourneert UTC tijd. Anders lokale tijd.
  :return: Geformatteerde tijdstring.
  """
  try:
    if use_utc:
      # Gebruik datetime.now(timezone.utc) voor een 'aware' datetime object in UTC
      current_time = datetime.now(timezone.utc)
      # Als je een suffix wilt toevoegen om UTC aan te duiden in het log:
      # return current_time.strftime(DATETIME_FORMAT) + " UTC"
    else:
      current_time = datetime.now()  # haal de huidige lokale datum en tijd op
    return current_time.strftime(DATETIME_FORMAT)
  except Exception as e:
    print(f"KRITIEKE FOUT bij ophalen/formatteren van tijd: {e}")
    return "TIJD_FOUT"

def store_data(file_prefix: str, log_message: str):
  # Hier kun je kiezen of je UTC of lokale tijd wilt loggen:
  # log_time = get_current_time(use_utc=True) # Voor UTC
  log_time = get_current_time(use_utc=False) # Voor lokale tijd (standaard)

  formatted_log_entry = f"{log_time} [{str(log_message)}]\n"

  try:
    with open(f"{file_prefix}.txt", "a", encoding='utf-8') as fo: # encoding='utf-8' toegevoegd
      fo.write(formatted_log_entry)
  except IOError as e:
    print(f"FOUT: Kon niet schrijven naar logbestand {file_prefix}.txt: {e}")

def connect_port():
  ports = serial.tools.list_ports.comports()
  attempted_details = []

  for port_info in ports:
    # Kleine aanpassing: controleer of port_info.description niet None is
    if port_info.description and DEVICE_TYPE.lower() in port_info.description.lower():
      try:
        serial_connection = serial.Serial(port_info.device, 9600, timeout=1)
        store_data("connection_logs", f"Succesvol verbonden met {port_info.device}: {port_info.description}")
        return serial_connection
      except serial.SerialException as s_exc:
        attempted_details.append(f"Poort {port_info.device} ({port_info.description}): SerialException - {s_exc}")
      except Exception as e_exc:
        attempted_details.append(f"Poort {port_info.device} ({port_info.description}): Onverwachte fout - {e_exc}")
  
  log_parts = ["Kon niet verbinden met een geschikte COM-poort."]
  if attempted_details:
    log_parts.append(" Details van pogingen: " + "; ".join(attempted_details))
  else:
    log_parts.append(f" Geen poorten gevonden die '{DEVICE_TYPE}' in hun beschrijving hebben of beschrijving is niet beschikbaar.")
  
  store_data("connection_logs", "".join(log_parts))
  return None

def collect_data():
  ser = None
  connection_timer = 0.5

  while True:
    while ser is None:
      ser = connect_port()
      if ser is None:
        time.sleep(connection_timer)
        # Optioneel: connection_timer = min(connection_timer * 2, 60)

    if ser.is_open:
      try:
        raw_serial_data = ser.readline()
      except serial.SerialException as e_ser:
        port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
        store_data("error_logs", f"Serial error tijdens lezen van {port_name}: {e_ser}")
        if ser and ser.is_open:
            try: ser.close()
            except: pass # Beste poging
        ser = None
        connection_timer = 60.0
        continue
      except Exception as e_exc:
        port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
        store_data("error_logs", f"Onverwachte error bij lezen van {port_name}: {e_exc}")
        if ser and ser.is_open:
            try: ser.close()
            except: pass
        ser = None
        connection_timer = 60.0
        continue

      if not raw_serial_data:
        time.sleep(0.1)
        continue

      serial_data = raw_serial_data.decode(errors='ignore').strip()

      if not serial_data:
        continue

      parsed_log_entries = []
      if serial_data.startswith("START@") and serial_data.endswith("@END") and "&" in serial_data:
        content = serial_data[len("START@"):-len("@END")]
        split_data_values = content.split("&")

        for data_pair_str in split_data_values:
          try:
            parts = data_pair_str.split("$", 1)
            if len(parts) == 2:
              key = parts[0].strip()
              value = parts[1].strip()
              if key and value:
                 parsed_log_entries.append(f"{key}: {value}")
              else:
                 parsed_log_entries.append(f"ERROR: Lege key of value in '{data_pair_str}'")
            else:
              parsed_log_entries.append(f"ERROR: Ongeldig formaat '{data_pair_str}'")
          except Exception as e_parse:
            parsed_log_entries.append(f"ERROR: {e_parse} bij '{data_pair_str}'")
      else:
        if serial_data:
            store_data("unknown_data_logs", f"Onbekend dataformaat: {serial_data}")
        continue

      if parsed_log_entries:
        final_log_data = " | ".join(parsed_log_entries)
        store_data("data", final_log_data)
    
    else:
      port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
      store_data("error_logs", f"Poort {port_name} is onverwacht gesloten.")
      ser = None
      connection_timer = 5.0

if __name__ == "__main__":
  # Diagnostische print voor de tijd bij het starten
  current_py_time_local = datetime.now().strftime(DATETIME_FORMAT + " (Lokaal)")
  current_py_time_utc = datetime.now(timezone.utc).strftime(DATETIME_FORMAT + " (UTC)")
  
  print("--- Script Start Informatie ---")
  print(f"Huidige tijd volgens Python (Lokaal): {current_py_time_local}")
  print(f"Huidige tijd volgens Python (UTC):   {current_py_time_utc}")
  print(f"Controleer of de 'Lokale' tijd overeenkomt met de klok van uw systeem.")
  print(f"Indien incorrect, pas de datum/tijd/tijdzone instellingen van uw systeem aan.")
  print(f"De logs gebruiken momenteel: {'UTC tijd' if False else 'Lokale tijd'}") # Pas 'False' aan als je use_utc verandert in store_data
  print("--- Starten datacollectie ---")

  try:
    collect_data()
  except KeyboardInterrupt:
    print("\nProgramma gestopt door gebruiker (Ctrl+C).")
  except Exception as e:
    # Log kritieke fouten ook naar een bestand voor persistentie
    error_message = f"Onbehandelde KRITIEKE FOUT in hoofdlus: {e}"
    print(error_message)
    try:
      store_data("critical_error_logs", error_message)
    except Exception as log_e:
      print(f"Kon kritieke fout niet loggen: {log_e}")
