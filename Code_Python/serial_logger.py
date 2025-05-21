#!/usr/bin/env python3

# BRONNEN
# ... (rest van je script blijft hetzelfde) ...
import os # Goed om te importeren, hoewel niet direct gebruikt in de logpaden als WorkingDirectory is ingesteld

import time
from datetime import datetime, timezone # timezone toegevoegd voor UTC optie

import serial
import serial.tools.list_ports

DATETIME_FORMAT = "%d-%m-%Y %H:%M"  # Formaat: DD-MM-YYYY HH:MM (lokale systeemtijd)
DEVICE_TYPE = "USB" # Deel van de poortbeschrijving om naar te zoeken

# Definiëer de map waar de logbestanden moeten komen.
# Als WorkingDirectory in systemd is ingesteld, is dit niet strikt nodig,
# maar het kan helpen als je het script ook handmatig wilt draaien vanuit een andere locatie.
# SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
# LOG_DIR = SCRIPT_DIR # Schrijf logs in dezelfde map als het script

def get_current_time(use_utc=False): # Parameter toegevoegd voor UTC
  """
  Haalt de huidige tijd op.
  :param use_utc: Indien True, retourneert UTC tijd. Anders lokale tijd.
  :return: Geformatteerde tijdstring.
  """
  try:
    if use_utc:
      current_time = datetime.now(timezone.utc)
    else:
      current_time = datetime.now()
    return current_time.strftime(DATETIME_FORMAT)
  except Exception as e:
    print(f"KRITIEKE FOUT bij ophalen/formatteren van tijd: {e}") # Gaat naar journalctl
    return "TIJD_FOUT"

def store_data(file_prefix: str, log_message: str):
  log_time = get_current_time(use_utc=False)
  formatted_log_entry = f"{log_time} [{str(log_message)}]\n"

  # Gebruik os.path.join als je LOG_DIR hierboven hebt gedefinieerd:
  # log_file_path = os.path.join(LOG_DIR, f"{file_prefix}.txt")
  # Anders, als WorkingDirectory in systemd is ingesteld, is dit prima:
  log_file_path = f"{file_prefix}.txt"

  try:
    with open(log_file_path, "a", encoding='utf-8') as fo:
      fo.write(formatted_log_entry)
  except IOError as e:
    # Deze print gaat naar journalctl als de service draait
    print(f"FOUT: Kon niet schrijven naar logbestand {log_file_path}: {e}")

def connect_port():
  ports = serial.tools.list_ports.comports()
  attempted_details = []

  for port_info in ports:
    if port_info.description and DEVICE_TYPE.lower() in port_info.description.lower():
      try:
        serial_connection = serial.Serial(port_info.device, 9600, timeout=1)
        # Deze store_data schrijft naar het logbestand in WorkingDirectory
        store_data("connection_logs", f"Succesvol verbonden met {port_info.device}: {port_info.description}")
        # Deze print gaat naar journalctl
        print(f"Succesvol verbonden met {port_info.device}: {port_info.description}")
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
  
  full_log_message = "".join(log_parts)
  store_data("connection_logs", full_log_message)
  print(full_log_message) # Naar journalctl
  return None

def collect_data():
  ser = None
  connection_timer = 0.5 # Start met kortere interval voor snelle eerste poging

  while True:
    while ser is None:
      print("Poging tot verbinden met seriële poort...") # Naar journalctl
      ser = connect_port()
      if ser is None:
        print(f"Verbinding mislukt, volgende poging over {connection_timer}s...") # Naar journalctl
        time.sleep(connection_timer)
        # Optioneel: connection_timer = min(connection_timer * 2, 60) # Exponentiële backoff
        connection_timer = min(connection_timer + 5, 60) # Lineaire backoff, max 60s
      else:
        print("Verbonden, data collectie start.") # Naar journalctl
        connection_timer = 0.5 # Reset timer na succesvolle verbinding

    if ser.is_open:
      try:
        raw_serial_data = ser.readline()
      except serial.SerialException as e_ser:
        port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
        error_msg = f"Serial error tijdens lezen van {port_name}: {e_ser}"
        store_data("error_logs", error_msg)
        print(error_msg) # Naar journalctl
        if ser and ser.is_open:
            try: ser.close()
            except: pass
        ser = None
        connection_timer = 10.0 # Wacht langer na een leesfout
        continue
      except Exception as e_exc:
        port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
        error_msg = f"Onverwachte error bij lezen van {port_name}: {e_exc}"
        store_data("error_logs", error_msg)
        print(error_msg) # Naar journalctl
        if ser and ser.is_open:
            try: ser.close()
            except: pass
        ser = None
        connection_timer = 10.0
        continue

      if not raw_serial_data:
        time.sleep(0.1) # Wacht even als er geen data is, voorkomt CPU-spinning
        continue

      serial_data = raw_serial_data.decode(errors='ignore').strip()

      if not serial_data:
        continue

      # print(f"Ontvangen ruwe data: {serial_data}") # Debug, naar journalctl

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
        if serial_data: # Alleen loggen als er daadwerkelijk onbekende data is
            store_data("unknown_data_logs", f"Onbekend dataformaat: {serial_data}")
            # print(f"Onbekend dataformaat ontvangen: {serial_data}") # Debug, naar journalctl
        continue # Ga door naar de volgende readline

      if parsed_log_entries:
        final_log_data = " | ".join(parsed_log_entries)
        store_data("data", final_log_data)
        # print(f"Data gelogd: {final_log_data}") # Optioneel, kan veel output genereren in journalctl
    
    else: # ser is niet None, maar .is_open is False
      port_name = ser.name if ser and hasattr(ser, 'name') else 'onbekende poort'
      error_msg = f"Poort {port_name} is onverwacht gesloten."
      store_data("error_logs", error_msg)
      print(error_msg) # Naar journalctl
      ser = None # Trigger herverbindingspoging
      connection_timer = 5.0 # Wacht even voor de volgende poging

if __name__ == "__main__":
  # Deze print statements zijn nuttig om in journalctl te zien wanneer het script (her)start
  print("--- Script Start ---")
  current_py_time_local = datetime.now().strftime(DATETIME_FORMAT + " (Lokaal)")
  current_py_time_utc = datetime.now(timezone.utc).strftime(DATETIME_FORMAT + " (UTC)")
  
  print(f"Python script gestart. Huidige systeemtijd (Lokaal): {current_py_time_local}")
  print(f"Python script gestart. Huidige systeemtijd (UTC):   {current_py_time_utc}")
  print(f"Logs gebruiken: {'UTC tijd' if False else 'Lokale tijd'}") # Pas 'False' aan als use_utc in store_data verandert
  print("--- Starten datacollectie ---")

  try:
    collect_data()
  except KeyboardInterrupt:
    print("\nProgramma gestopt door KeyboardInterrupt (zal wss niet gebeuren als service).")
  except Exception as e:
    error_message = f"Onbehandelde KRITIEKE FOUT in hoofdlus: {e}"
    print(error_message) # Naar journalctl
    try:
      # Zorg ervoor dat dit pad correct is of relatief aan WorkingDirectory
      store_data("critical_error_logs", error_message)
    except Exception as log_e:
      print(f"Kon kritieke fout niet loggen naar bestand: {log_e}") # Naar journalctl
