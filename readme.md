# 🐕 Watchdog Router (v1.3)

Un sistema autónomo basado en ESP32 para monitorizar la conexión a internet y reiniciar automáticamente el router mediante un relé en caso de caídas persistentes. Incluye una pantalla LCD I2C (0x27, 20x4) para visualizar el estado de la red en tiempo real.

> 🎮 **¡Prueba el emulador LCD!** > He preparado una simulación web de la pantalla. [Haz clic aquí para ver el emulador interactivo](https://aabadmo4.github.io/router_watchdog/) 

## 🌟 Características Principales

* **Monitorización Dual:** Realiza pings a los DNS de Google (8.8.8.8) y, como respaldo, a Cloudflare (1.1.1.1).
* **Control de Energía:** Si se acumulan 5 pings fallidos consecutivos (con intervalos de 10 segundos), el ESP32 activa un relé en el pin 23 para cortar y restaurar la alimentación del router.
* **Prevención de Bucles (Lockout):** Tras 3 intentos de reinicio fallidos consecutivos, el sistema entra en un estado de espera de 1 hora ("Fallo persistente") para evitar reiniciar el router en caso de caída general de la operadora.
* **Protección Interna:** Utiliza el Task Watchdog Timer (TWDT) nativo del ESP32 (timeout de 30s) para evitar que el propio microcontrolador se quede colgado.

## 📺 Vistas del Sistema (Simulación LCD)

A continuación, se muestran los diferentes estados que puede presentar la pantalla LCD HD44780 durante su funcionamiento.

### Estado Normal
El sistema está conectado y con salida a internet.
![Estado OK](assets/estado_ok.png)
*(Reemplaza la ruta de la imagen con una captura de tu HTML mostrando el estado OK)*

### Caída de Red y Reinicio
El router pierde conexión. Tras 5 intentos, procede a cortar la energía temporalmente.
![Alerta Red Caída](assets/alerta_caida.png)
*(Añade aquí la captura del estado "Corte de energía")*

### Fallo Persistente (Modo Lockout)
Si la red no vuelve tras múltiples reinicios, el sistema asume un problema externo (cobertura) y pausa los reinicios durante 1 hora.
![Fallo Persistente](assets/fallo_persistente.png)
*(Añade aquí la captura del estado "Fallo persistente (1h)")*

## 🛠 Instalación y Configuración

1. Modifica las variables `ssid` y `password` en el archivo `router_watchdog.ino`.
2. Conecta tu relé al **PIN 23** del ESP32.
3. Conecta los pines SDA y SCL de la pantalla LCD I2C.
4. Compila y sube el código usando el IDE de Arduino.
   
