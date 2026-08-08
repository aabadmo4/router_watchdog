/*
 * Watchdog Router - Reinicio automático GPRS/IP
 * Copyright (c) 2026 Adán
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * Licensed under the PolyForm Noncommercial License 1.0.0.
 * Uso comercial no permitido sin autorización expresa del autor.
 * Full license: https://polyformproject.org/licenses/noncommercial/1.0.0
 *
 * Repo: https://github.com/aabadmo4/router_watchdog
 * Version: 1.3
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <esp_task_wdt.h>

// Configuración LCD I2C (Dirección habitual: 0x27 o 0x3F, 20 columnas, 4 filas)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Configuración WiFi
const char* ssid = "TU_NOMBRE_DE_RED_WIFI";
const char* password = "TU_CONTRASEÑA_WIFI";

// Pines y Parámetros
const int RELAY_PIN = 23;
const IPAddress targetIP1(8, 8, 8, 8);   // Google DNS
const IPAddress targetIP2(1, 1, 1, 1);   // Cloudflare DNS (respaldo)

int failedPings = 0;
const int MAX_FAILED_PINGS = 5;          // Fallos seguidos antes de reiniciar
const int PING_INTERVAL_MS = 10000;      // Intervalo entre pings (10s)
const int ROUTER_BOOT_TIME_MS = 300000;  // Tiempo de espera tras reiniciar (5 min)

// --- v1.3: control de reinicios en bucle ---
int rebootCount = 0;
const int MAX_REBOOTS_BEFORE_LOCKOUT = 3;      // Reinicios permitidos antes de bloquear
const unsigned long LOCKOUT_TIME_MS = 3600000; // Espera larga: 1 hora

// --- v1.3: task watchdog del propio ESP32 ---
const int TWDT_TIMEOUT_S = 30;

String publicIP = "Obteniendo...";

// Imprime una línea en la LCD, recortada o rellenada a exactamente 20 caracteres.
void printRow(int row, String text) {
  if (text.length() > 20) text = text.substring(0, 20);
  while (text.length() < 20) text += " ";
  lcd.setCursor(0, row);
  lcd.print(text);
}

void setup() {
  Serial.begin(115200);

  // Inicializar watchdog del propio ESP32
  esp_task_wdt_init(TWDT_TIMEOUT_S, true); // true = reset en caso de disparo
  esp_task_wdt_add(NULL);                  // registra la tarea actual (loop)

  lcd.init();
  lcd.backlight();
  lcd.clear();

  printRow(0, "  WATCHDOG ROUTER");
  printRow(1, "  Iniciando sistema v1.3");
  delay(2000);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relé inactivo (Alimentación OK)

  conectarWiFi();
  publicIP = obtenerIPPublica();
}

void loop() {
  esp_task_wdt_reset(); // alimentar watchdog en cada ciclo

  // 1. Verificar conexión WiFi local (asociación al router y DHCP)
  if (!wifiConectadoValido()) {
    printRow(0, estadoWiFiTexto());
    conectarWiFi();
  } else {
    printRow(0, WiFi.localIP().toString());
  }

  // 2. Realizar prueba de Ping (con respaldo a 1.1.1.1 si falla 8.8.8.8)
  bool pingSuccess = false;
  if (wifiConectadoValido()) {
    pingSuccess = Ping.ping(targetIP1, 2);
    if (!pingSuccess) {
      pingSuccess = Ping.ping(targetIP2, 2);
    }
  }

  if (pingSuccess) {
    if (failedPings > 0 || publicIP == "Obteniendo..." || publicIP == "Error IP") {
      publicIP = obtenerIPPublica();
    }

    failedPings = 0;
    rebootCount = 0; // la red se recuperó: reseteamos el contador de reinicios

    printRow(1, publicIP);
    printRow(2, "Ping: OK");
    printRow(3, "WAN: OK");

  } else {
    failedPings++;
    publicIP = "No connection";

    printRow(1, "No connection");
    printRow(2, "Ping ERR " + String(failedPings) + "/" + String(MAX_FAILED_PINGS));
    printRow(3, "WAN: Error");
  }

  // 3. Evaluar si es necesario reiniciar (o entrar en espera larga)
  if (failedPings >= MAX_FAILED_PINGS) {
    rebootCount++;

    if (rebootCount > MAX_REBOOTS_BEFORE_LOCKOUT) {
      esperaLockout();
      rebootCount = 0;
      failedPings = 0;
    } else {
      reiniciarRouter();
    }
  }

  delay(PING_INTERVAL_MS);
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 15) {
    delay(500);
    intentos++;
  }
}

// Conectado de verdad = asociado al router Y con IP válida por DHCP
bool wifiConectadoValido() {
  return WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0, 0, 0, 0);
}

// Texto corto (<=20 car.) según la causa real del fallo
String estadoWiFiTexto() {
  wl_status_t s = WiFi.status();

  if (s == WL_CONNECTED && WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    return "DHCP error";
  }

  switch (s) {
    case WL_NO_SSID_AVAIL:   return "Not found";
    case WL_CONNECT_FAILED:  return "Pwd error";
    case WL_CONNECTION_LOST: return "Conn lost";
    case WL_DISCONNECTED:    return "No response";
    case WL_IDLE_STATUS:     return "Connecting...";
    default:                 return "No connection";
  }
}

// Función para consultar la IP pública mediante petición HTTP
String obtenerIPPublica() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://api.ipify.org");
    http.setTimeout(3000);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      http.end();
      return payload;
    }
    http.end();
  }
  return "Error IP";
}

void reiniciarRouter() {
  lcd.clear();
  printRow(0, "!ALERTA RED CAIDA!");
  printRow(1, "Reiniciando router");
  printRow(2, "Intento " + String(rebootCount) + "/" + String(MAX_REBOOTS_BEFORE_LOCKOUT));
  printRow(3, "AC Error");

  digitalWrite(RELAY_PIN, LOW);
  delay(10000);
  digitalWrite(RELAY_PIN, HIGH);

  failedPings = 0;
  publicIP = "Obteniendo...";

  // Cuenta atrás en pantalla durante el arranque del router (5 min)
  for (int i = ROUTER_BOOT_TIME_MS / 1000; i > 0; i--) {
    esp_task_wdt_reset(); // alimentar watchdog durante la espera larga
    printRow(0, "Router reiniciado");
    printRow(1, "Esperando arranque");
    printRow(2, "Tiempo: " + String(i) + " seg");
    printRow(3, "Intento " + String(rebootCount) + "/" + String(MAX_REBOOTS_BEFORE_LOCKOUT));
    delay(1000);
  }
  lcd.clear();
}

// --- v1.3: espera larga tras 3 reinicios fallidos consecutivos ---
void esperaLockout() {
  lcd.clear();
  printRow(0, "!FALLO PERSISTENTE!");
  printRow(1, String(MAX_REBOOTS_BEFORE_LOCKOUT) + " reinicios fallidos");

  unsigned long totalSeconds = LOCKOUT_TIME_MS / 1000;
  for (unsigned long i = totalSeconds; i > 0; i--) {
    esp_task_wdt_reset(); // alimentar watchdog durante la hora de espera
    unsigned long minutos = i / 60;
    unsigned long segundos = i % 60;
    printRow(2, "Espera: " + String(minutos) + "m " + String(segundos) + "s");
    printRow(3, "GPRS Error");
    delay(1000);
  }
  lcd.clear();
}
