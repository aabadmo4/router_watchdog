# 🐕 Watchdog Router (v1.3)

[![License: PolyForm Noncommercial 1.0.0](https://img.shields.io/badge/License-PolyForm--Noncommercial--1.0.0-blue.svg)](https://polyformproject.org/licenses/noncommercial/1.0.0)

Un sistema autónomo basado en **ESP8266** para monitorizar la conexión a internet y reiniciar automáticamente el router mediante un relé en caso de caídas persistentes. Incluye una pantalla LCD I2C (0x27, 20x4) para visualizar el estado de la red en tiempo real.

> 🎮 **¡Prueba el emulador LCD!**
> He preparado una simulación web de la pantalla. [Haz clic aquí para ver el emulador interactivo](https://aabadmo4.github.io/router_watchdog/)

## 🌟 Características Principales

- **Monitorización Dual:** Realiza pings a los DNS de Google (8.8.8.8) y, como respaldo, a Cloudflare (1.1.1.1).
- **Control de Energía:** Si se acumulan 5 pings fallidos consecutivos (con intervalos de 10 segundos), el ESP8266 activa un relé en el pin **D5** para cortar y restaurar la alimentación del router.
- **Prevención de Bucles (Lockout):** Tras 3 intentos de reinicio fallidos consecutivos, el sistema entra en un estado de espera de 1 hora ("Fallo persistente") para evitar reiniciar el router en caso de caída general de la operadora.
- **Protección Interna:** Utiliza el watchdog de software integrado del ESP8266 (alimentado automáticamente con cada `delay()`/`yield()`) para evitar que el propio microcontrolador se quede colgado.

## 📺 Vistas del Sistema (Simulación LCD)

A continuación, se muestran los diferentes estados que puede presentar la pantalla LCD HD44780 durante su funcionamiento.

### Estado Normal

El sistema está conectado y con salida a internet.
[![Estado OK](https://github.com/aabadmo4/router_watchdog/raw/main/assets/estado_ok.png)](/aabadmo4/router_watchdog/blob/main/assets/estado_ok.png)
*(Reemplaza la ruta de la imagen con una captura de tu HTML mostrando el estado OK)*

### Caída de Red y Reinicio

El router pierde conexión. Tras 5 intentos, procede a cortar la energía temporalmente.
[![Alerta Red Caída](https://github.com/aabadmo4/router_watchdog/raw/main/assets/alerta_caida.png)](/aabadmo4/router_watchdog/blob/main/assets/alerta_caida.png)
*(Añade aquí la captura del estado "Corte de energía")*

### Fallo Persistente (Modo Lockout)

Si la red no vuelve tras múltiples reinicios, el sistema asume un problema externo (cobertura) y pausa los reinicios durante 1 hora.
[![Fallo Persistente](https://github.com/aabadmo4/router_watchdog/raw/main/assets/fallo_persistente.png)](/aabadmo4/router_watchdog/blob/main/assets/fallo_persistente.png)
*(Añade aquí la captura del estado "Fallo persistente (1h)")*

## 🛠 Instalación y Configuración

### Librerías necesarias

Antes de nada, instala el **core de placas "esp8266"** (por ESP8266 Community) desde Herramientas > Placa > Gestor de Placas, si no lo tienes ya. Una vez instalado, estas librerías ya vienen incluidas y no requieren nada más:

- `ESP8266WiFi.h`
- `ESP8266HTTPClient.h`
- `Wire.h`

Estas otras dos **no** vienen incluidas y hay que instalarlas aparte:

| Librería | Cómo instalarla |
|---|---|
| `LiquidCrystal_I2C` | Sketch > Include Library > **Manage Libraries...** (Ctrl+Mayús+I), busca "LiquidCrystal I2C" (autor: Frank de Brabander) e instálala. |
| `ESP8266Ping` | No está en el Gestor de Librerías. Descárgala manualmente desde [github.com/dancol90/ESP8266Ping](https://github.com/dancol90/ESP8266Ping) (botón verde **Code > Download ZIP**), y luego en el IDE ve a Sketch > Include Library > **Add .ZIP Library...** y selecciona el ZIP descargado. |

> ⚠️ Al compilar puede aparecer un aviso de que `LiquidCrystal_I2C` "pretende ejecutarse sobre arquitectura(s) avr" y podría ser incompatible con ESP8266. Es solo un aviso, no un error — la librería solo usa `Wire.h` internamente y funciona sin problemas en ESP8266. Si quieres eliminarlo, edita `library.properties` dentro de la carpeta de la librería instalada y cambia `architectures=avr` por `architectures=*`.

### Pasos

1. Modifica las variables `ssid` y `password` en el archivo `router_watchdog.ino`.
2. Conecta tu relé al pin **D5** del ESP8266 (NodeMCU / Wemos D1 Mini).
3. Conecta los pines SDA y SCL de la pantalla LCD I2C a **D2 (SDA)** y **D1 (SCL)** — pines por defecto de `Wire` en placas ESP8266.
4. Compila y sube el código usando el IDE de Arduino, seleccionando tu placa ESP8266 (p. ej. "NodeMCU 1.0" o "LOLIN(WEMOS) D1 R2 & mini").

## 📄 Licencia

Este proyecto está publicado bajo **[PolyForm Noncommercial License 1.0.0](https://polyformproject.org/licenses/noncommercial/1.0.0)**.

- ✅ Uso personal, educativo, de investigación o por organizaciones sin ánimo de lucro: libre.
- ⚠️ **Uso comercial (empresas, integración en productos o servicios de pago) requiere autorización previa del autor.**
- 🖊️ Debe mantenerse la atribución de autoría en cualquier copia o distribución.

Copyright (c) 2026 Adán. Para licencias comerciales, contacta a través de [GitHub](https://github.com/aabadmo4).
