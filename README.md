# Casas de Hadas - Mundo Imayina

Firmware para una casa de hadas con ESP32, iluminación WS2812B, mecanismos de movimiento, expulsión de purpurina y reproducción de audio mediante un receptor LoRa conectado a una SoundBoard.

## Requisitos

- ESP32 WROOM32 con módulo LoRa en la configuración actual.
- PlatformIO o Arduino IDE con el núcleo ESP32.
- Librerías `FastLED`, `ESP32Servo`, `Botones` y `LoRa`.
- Módulo LoRa de 3.3 V configurado a 915 MHz.

La configuración activa se selecciona con `USE_ESP32_WROOM32` en `src/Config.h`. El valor predeterminado es `1`.

## Licencia

Copyright © 2026 ORTMED. Todos los derechos reservados.

Este software y su documentación son propiedad de ORTMED S.A de C.V. Está estrictamente prohibido usar, copiar o modificar el software sin autorización explícita y por escrito de ORTMED S.A de C.V. Consulta el archivo [LICENSE](LICENSE) para ver el aviso completo.

## Mapeo de pines

El mapa activo se selecciona en `src/Config.h` mediante `USE_ESP32_WROOM32`.

### ESP32 WROOM32 con LoRa (configuración predeterminada)

| Pin | Sensor o actuador | Conexión / función |
| --- | --- | --- |
| GPIO 13 | Servo MG90S de la guillotina | Señal PWM del servo |
| GPIO 33 | MOSFET del Mini Blower 3010 (5 V) | Salida para activar el soplador |
| GPIO 27 | MOSFET del ventilador (5 V) | Salida para activar el ventilador |
| GPIO 22 | Actuador lineal (5 V) | Salida para mover hacia adelante |
| GPIO 21 | Actuador lineal (5 V) | Salida para mover hacia atrás |
| GPIO 4 | Tira LED WS2812B | Línea de datos DIN |
| GPIO 32 | Botón antivandálico IP67 | Entrada digital con `INPUT_PULLUP` |
| GPIO 18 | Módulo LoRa | SPI SCK |
| GPIO 19 | Módulo LoRa | SPI MISO |
| GPIO 23 | Módulo LoRa | SPI MOSI |
| GPIO 25 | Módulo LoRa | NSS/CS |
| GPIO 14 | Módulo LoRa | RESET |
| GPIO 26 | Módulo LoRa | DIO0/IRQ |
| GPIO 16 | Módulo LoRa | DIO1, actualmente sin usar |

En esta configuración el DFPlayer Mini no se conecta directamente al ESP32: el sketch transmite por LoRa el número de pista a otro módulo que controla la SoundBoard. El módulo LoRa debe trabajar a 3.3 V y compartir GND con el ESP32.

GPIO 4 y GPIO 5 son pines de arranque (strapping) del ESP32. Son funcionales como salidas, pero no conviene forzarlos durante el encendido.

### ESP32-S3 Super Mini sin LoRa (configuración anterior)

| Pin | Sensor o actuador | Conexión / función |
| --- | --- | --- |
| GPIO 5 | Servo MG90S de la guillotina | Señal PWM del servo |
| GPIO 2 | MOSFET del Mini Blower 3010 (5 V) | Salida para activar el soplador |
| GPIO 10 | Tira LED WS2812B | Línea de datos DIN |
| GPIO 6 | DFPlayer Mini | TX del ESP32 hacia RX del DFPlayer |
| GPIO 7 | DFPlayer Mini | RX del ESP32 desde TX del DFPlayer |
| GPIO 9 | Botón antivandálico IP67 | Entrada digital con `INPUT_PULLUP` |
| GPIO 8 | DFPlayer Mini | Entrada `BUSY` con `INPUT_PULLUP` |

Esta configuración conserva el cableado anterior. No incluye ventilador, actuador lineal ni módulo LoRa, y no es compatible con el flujo actual del sketch principal sin adaptar el código.

## Lógica de funcionamiento

- Al iniciar, la guillotina, el blower, el ventilador y el actuador lineal quedan apagados; la tira LED queda limpia con brillo de reposo.
- Al pulsar el botón conectado al GPIO 32, se activa el espectáculo si el sistema está en `STATE_IDLE`. También puede activarse con `R` desde el monitor serial.
- Las pistas de audio se transmiten tres veces por LoRa al receptor con `id_destino = 5`. El receptor reproduce la pista solicitada.
- La secuencia principal utiliza estas pistas: `10` para la presentación, `11` cuando sale el hada, `12` durante la expulsión de purpurina y `13` durante el regreso.
- La tira tiene 40 LEDs. La presentación usa una secuencia ámbar; las zonas de la puerta usan luz ámbar sólida y la expulsión usa el efecto de destellos.
- El actuador lineal mueve el hada hacia afuera durante 9 segundos. Después se inicia la secuencia de purpurina: la guillotina se abre a 180 grados durante 3 segundos, se cierra, espera 1 segundo y enciende el blower junto con el ventilador durante 5.5 segundos.
- El blower se controla con PWM y el ventilador con una salida digital. Ambos se apagan al terminar la expulsión.
- `STATE_SHOW_RUNNING` finaliza después de 10 segundos; luego la guillotina se asegura cerrada y el actuador lineal retorna durante 9 segundos antes de volver a `STATE_IDLE`.
- Cada hora desde el encendido se intenta iniciar `STATE_SHOW_AUTO`, siempre que el sistema esté en reposo. Este modo dura 7 segundos, enciende la tira a brillo máximo y transmite una pista aleatoria del rango configurado.
- La lógica principal usa una máquina de estados y temporizadores basados en `millis()`. El botón usa un antirrebote de 100 ms y el watchdog tiene un tiempo de espera de 5 segundos.

## Comunicación LoRa

El sketch transmite una estructura binaria empaquetada de dos bytes:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `id_destino` | `uint8_t` | Identificador del receptor; actualmente `5` |
| `comando` | `uint8_t` | Número de pista que debe reproducirse |

Parámetros actuales: frecuencia de 915 MHz, ancho de banda de 125 kHz, factor de dispersión SF9, codificación 4/5 y palabra de sincronización `0x12`. El receptor debe usar exactamente la misma estructura y configuración de radio.

## Organización del proyecto

### Sketch final

- [CasaHadas_ProyectoImayina.ino](CasaHadas_ProyectoImayina.ino): sketch principal ubicado en la raíz de la biblioteca. Integra el botón, la iluminación, el audio por LoRa, el blower, el ventilador, el actuador lineal y la guillotina mediante una máquina de estados.

### Archivos de prueba (`examples`)

- [ControlBlower.ino](examples/ControlBlower/ControlBlower.ino): prueba el Mini Blower. Lo enciende y lo apaga después de completar la secuencia de tiempos configurada.
- [ControlServomotores.ino](examples/ControlServomotores/ControlServomotores.ino): prueba el servo de la guillotina. Al pulsar el botón, abre la guillotina y la cierra después del tiempo configurado.
- [PruebaAudioDFPlayer.ino](examples/PruebaAudioDFPlayer/PruebaAudioDFPlayer.ino): prueba una conexión directa con el DFPlayer Mini e inicia la reproducción al pulsar el botón.
- [PruebaTiraLED.ino](examples/PruebaTiraLED/PruebaTiraLED.ino): prueba la tira WS2812B con una secuencia ámbar al pulsar el botón.
- [LoRaEmisor.ino](examples/LoRaEmisor/LoRaEmisor.ino): prueba el envío de comandos LoRa.
- [LoRaReceptor.ino](examples/LoRaReceptor/LoRaReceptor.ino): prueba la recepción de comandos LoRa.
- [ReceptorLoRaASoundBoard.ino](examples/ReceptorLoRaASoundBoard/ReceptorLoRaASoundBoard.ino): recibe por LoRa el número de pista para la SoundBoard.
- [ControlActuadorLineal.ino](examples/ControlActuadorLineal/ControlActuadorLineal.ino): prueba el movimiento del actuador lineal.
- [ServoYBlower.ino](examples/ServoYBlower/ServoYBlower.ino): prueba conjunta del servo y el blower.

### Archivos de soporte (`src`)

- [Config.h](src/Config.h): concentra el mapeo de pines, los ángulos del servo, los tiempos de la secuencia, los parámetros de iluminación y los tiempos de seguridad.
- [BlowerControl.h](src/BlowerControl.h): encapsula el control PWM del blower a través del MOSFET, incluyendo su inicialización, encendido, apagado y consulta de estado.
- [ILightingController.h](src/ILightingController.h): define la interfaz abstracta que deben implementar los controladores de iluminación.
- [FastLEDController.h](src/FastLEDController.h): implementa la interfaz de iluminación con FastLED y controla la tira WS2812B, su brillo y sus efectos visuales.
- [ServoManager.h](src/ServoManager.h): administra el servo MG90S de la guillotina, sus posiciones abierta y cerrada y el intervalo mínimo entre movimientos.
- [TarjetaAudio.hpp](src/TarjetaAudio.hpp): declara las interfaces y clases base comunes para controlar tarjetas de audio de uno o varios canales.
- [TarjetaAudio.cpp](src/TarjetaAudio.cpp): implementa la funcionalidad común de las clases base de audio, como silencio, volumen y selección de comunicación serie.
- [SPControladorDFPlayerMini.hpp](src/SPControladorDFPlayerMini.hpp): declara el controlador específico del DFPlayer Mini para configuraciones que lo conecten directamente.
- [SPControladorDFPlayerMini.cpp](src/SPControladorDFPlayerMini.cpp): implementa la comunicación directa con el DFPlayer Mini, sus pines serie, el pin `BUSY` y la reproducción de pistas.

## Parámetros principales

Los pines y tiempos se centralizan en [Config.h](src/Config.h). Entre los valores vigentes están:

- 40 LEDs WS2812B y brillo de reposo `60`.
- Brillo de espectáculo `255`.
- Ángulo cerrado de la guillotina: `0`; ángulo abierto: `180`.
- Actuador lineal: `9000 ms` por movimiento.
- Guillotina abierta: `3000 ms`.
- Espera antes del blower: `1000 ms`.
- Blower encendido: `5500 ms`.
- Espectáculo principal: `10000 ms`.
- Espectáculo automático: `7000 ms`.
