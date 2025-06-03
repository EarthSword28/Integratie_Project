#ifndef CONFIG_H
#define CONFIG_H

// --- Wi-Fi Instellingen ---
const char* ssid = "WifiBijenkast";
const char* password = "G5bijenkast!";

// const char* ssid = "Wifi_Nitro_5";
// const char* password = "fa107679";

// --- API Endpoint Instellingen ---
const char* api_host = "192.168.10.204";
const int api_port = 443;
const char* api_path_sensor_data = "/api/sensor-data";
const char* api_key = "apiSleutelGroep1";

//Eigen ESP32: 192.168.10.206

// --- NTP Instellingen ---
const char* ntpServer = "pool.ntp.org";
// gmtOffset_sec is de totale offset in seconden t.o.v. UTC die je wilt.
// Voorbeeld: UTC+2 (zomertijd West-Europa) = 7200. UTC+1 (wintertijd) = 3600.
const long gmtOffset_sec = 7200;
// ntp_update_interval_ms is hoe vaak de NTPClient intern probeert te updaten (minder relevant bij deepsleep).
const unsigned long ntp_update_interval_ms = 60000UL; // 60 seconden

// --- Root CA Certificaat voor HTTPS (momenteel niet gebruikt omdat setInsecure() actief is) ---
const char* rootCACertificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEVzCCAz+gAwIBAgIUKiP9bdXSXJ0hjbXK8ENrP8brRfswDQYJKoZIhvcNAQEL\n"
    "BQAwgboxCzAJBgNVBAYTAkJFMRIwEAYDVQQIDAlBbnR3ZXJwZW4xEDAOBgNVBAcM\n"
    "B0hvYm9rZW4xDDAKBgNVBAoMA0tkRzEjMCEGA1UECwwaQmlqZW5rYXN0X0NlbnRy\n"
    "YWFsX1N5c3RlZW0xITAfBgNVBAMMGGJpamVua2FzdC1jZW50cmFhbC5sb2NhbDEv\n"
    "MC0GCSqGSIb3DQEJARYga2Vuem8uaGFlbWVybGluY2tAc3R1ZGVudC5rZGcuYmUw\n"
    "HhcNMjUwNTA5MTAxNDA0WhcNMjYwNTA5MTAxNDA0WjCBujELMAkGA1UEBhMCQkUx\n"
    "EjAQBgNVBAgMCUFudHdlcnBlbjEQMA4GA1UEBwwHSG9ib2tlbjEMMAoGA1UECgwD\n"
    "S2RHMSMwIQYDVQQLDBpCaWplbmthc3RfQ2VudHJhYWxfU3lzdGVlbTEhMB8GA1UE\n"
    "AwwYYmlqZW5rYXN0LWNlbnRyYWFsLmxvY2FsMS8wLQYJKoZIhvcNAQkBFiBrZW56\n"
    "by5oYWVtZXJsaW5ja0BzdHVkZW50LmtkZy5iZTCCASIwDQYJKoZIhvcNAQEBBQAD\n"
    "ggEPADCCAQoCggEBAK9UTMXTXUbSMsEXV/PfXulbwm9if7D10h6swsT9VmX81nB8\n"
    "2Sd7S1nZ6V0gRiA5ywrqCpfhov7sn8Ix07oMxACxKVAQYB4tAWK1i1fENG92D+M9\n"
    "y6hJpFQofPeSdF6iluULfIDM4FbXsVlS8TwLaT8xC7ShYo0hfxKJ7Zzw9pCA5iSK\n"
    "EDIkX+5lcgQAdN4/waCGlzTaHP422E2d/KPtY030SK6aiH9E4v7iTazDt8Tci3BH\n"
    "ElvB9la0Hk4JNUXOPmlwJbrim989SXlp8Dr5cEl5YZ08hJq6OnQ7ZKZVSO42D0KO\n"
    "EoI0UpitPLa5sU/40G90Okamz6x+PuaRDM2048MCAwEAAaNTMFEwHQYDVR0OBBYE\n"
    "FLUyzJoIu2/i3+q5gIfyoVwcDL7BMB8GA1UdIwQYMBaAFLUyzJoIu2/i3+q5gIfy\n"
    "oVwcDL7BMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBABtAoshB\n"
    "2VaF5gdfooe8FvUrw9Gka3oB649nEPiptKZ7B4m6EZdtCDh9YaDISsSi/s6dpqlLz\n"
    "fCIIY/fouPcAE16xqCz2Fl5wnwiHVNkecGlbQZr0gUaNc4IAL9gMX80zOeDG8SVh\n"
    "2R1TvR0k7TNfhtyS+NoWYtUtPTl1Rjf5rGBn0l7R0S9uyw+qS2t+y17TAswHYEdZ\n"
    "ofsP2mnJR0Zu+UEvFgAQFj9pnDpm18l2Vpowe1bHIgZLaL7qCad7Hd+v67xlMTSW\n"
    "9mI/y686LnsYbVRRIVPbLTRFU+r9Yvs9nkx+uBLe98ADcMbouCu5H/g4z+Zzblib\n"
    "V3NeXCv0lacbjVI=\n"
    "-----END CERTIFICATE-----\n";

// --- Deepsleep Instellingen ---
#define uS_TO_S_FACTOR 1000000ULL       // Conversie factor microseconden naar seconden
#define TIME_TO_SLEEP_SECONDS 60        // Aantal seconden slapen (1 minuut)

#endif // CONFIG_H


