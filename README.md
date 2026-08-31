# Documentación de hardware

## Licencia

Copyright © 2026 ORTMED. Todos los derechos reservados.

Este software y su documentación son propiedad de ORTMED S.A de C.V. Está estrictamente prohibido usar, copiar o modificar el software sin autorización explícita y por escrito de ORTMED S.A de C.V. Consulta el archivo [LICENSE](LICENSE) para ver el aviso completo.

## Mapeo de pines

| Pin del ESP32-S3 Super Mini | Sensor o actuador | Conexión / función |
| --- | --- | --- |
| GPIO 5 | Servo MG90S de la guillotina | Señal PWM del servo |
| GPIO 2 | MOSFET del Mini Blower 3010 (5 V) | Salida PWM para activar el soplador |
| GPIO 10 | Tira LED WS2812B, 8 LEDs | Línea de datos DIN |
| GPIO 6 | DFPlayer Mini | TX del ESP32 hacia RX del DFPlayer, con resistencia de 1 kOhm |
| GPIO 7 | DFPlayer Mini | RX del ESP32 desde TX del DFPlayer, con resistencia de 1 kOhm |
| GPIO 9 | Botón antivandálico IP67 | Entrada digital con `INPUT_PULLUP`; el botón conecta el pin a GND al pulsarse |
| GPIO 8 | DFPlayer Mini | Entrada `BUSY` con `INPUT_PULLUP`; nivel LOW indica reproducción activa, actualmente en desuso |

## Lógica de funcionamiento

- Al iniciar, la guillotina queda cerrada, el blower apagado, el DFPlayer mini en silencio y la tira LED apagada.
- Al pulsar el botón conectado al GPIO 7, se activa el espectáculo si el sistema estaba en reposo.
- La tira WS2812B pasa a brillo máximo y ejecuta una secuencia ámbar durante el espectáculo.
- El servo abre la guillotina a 180 grados durante 3 segundos para liberar la purpurina.
- La guillotina vuelve a 0 grados y el sistema reproduce la pista 1 del DFPlayer Mini.
- Después de 500 ms, el blower se enciende.
- El blower continúa encendido durante 2 segundos para expulsar la purpurina.
- El blower se apaga y el espectáculo finaliza a los 10 segundos de su inicio.
- Al cerrar el espectáculo, la guillotina se asegura cerrada, el blower queda apagado y la iluminación vuelve a apagarse. La duración del audio debe ser menor o igual a 7 segundos para asegurar que termine al mismo tiempo que el espectáculo o antes.
- El pin `BUSY` del DFPlayer indica LOW mientras hay audio, pero la duración actual del espectáculo se controla con el temporizador de 10 segundos.
- Los audios a reproducir y la secuencia de iluminación de la tira WS2812B cambian cada vez que se presiona el botón.
- Después de cada hora desde que el sistema es energizado, la tira WS2812B se enciende durante 7 segundos y se reproduce una pista de audio del DFPlayer Mini. Los audios a reproducir y la secuencia de iluminación de la tira WS2812B son aleatorios, dentro de las opciones disponibles.

## Organización del proyecto

### Sketch final

- [CasaHadas_ProyectoImayina.ino](CasaHadas_ProyectoImayina.ino): sketch principal ubicado en la raíz de la biblioteca. Contiene el código final que integra el botón, la iluminación, el audio, el blower y la guillotina para controlar la casa de hadas mediante una máquina de estados.

### Archivos de prueba (`examples`)

- [ControlBlower.ino](examples/ControlBlower/ControlBlower.ino): prueba el Mini Blower. Lo enciende y lo apaga después de completar la secuencia de tiempos configurada.
- [ControlServomotores.ino](examples/ControlServomotores/ControlServomotores.ino): prueba el servo de la guillotina. Al pulsar el botón, abre la guillotina y la cierra después del tiempo configurado.
- [PruebaAudioDFPlayer.ino](examples/PruebaAudioDFPlayer/PruebaAudioDFPlayer.ino): prueba el DFPlayer Mini. Inicializa el reproductor y reproduce la pista 1 al pulsar el botón.
- [PruebaTiraLED.ino](examples/PruebaTiraLED/PruebaTiraLED.ino): prueba la tira WS2812B de 8 LEDs. En reposo mantiene los LEDs apagados y, al pulsar el botón, ejecuta la secuencia ámbar durante el tiempo configurado.

### Archivos de soporte (`src`)

- [Config.h](src/Config.h): concentra el mapeo de pines, los ángulos del servo, los tiempos de la secuencia, los parámetros de iluminación y los tiempos de seguridad.
- [BlowerControl.h](src/BlowerControl.h): encapsula el control PWM del blower a través del MOSFET, incluyendo su inicialización, encendido, apagado y consulta de estado.
- [ILightingController.h](src/ILightingController.h): define la interfaz abstracta que deben implementar los controladores de iluminación.
- [FastLEDController.h](src/FastLEDController.h): implementa la interfaz de iluminación con FastLED y controla la tira WS2812B, su brillo y sus efectos visuales.
- [ServoManager.h](src/ServoManager.h): administra el servo MG90S de la guillotina, sus posiciones abierta y cerrada y el intervalo mínimo entre movimientos.
- [TarjetaAudio.hpp](src/TarjetaAudio.hpp): declara las interfaces y clases base comunes para controlar tarjetas de audio de uno o varios canales.
- [TarjetaAudio.cpp](src/TarjetaAudio.cpp): implementa la funcionalidad común de las clases base de audio, como silencio, volumen y selección de comunicación serie.
- [SPControladorDFPlayerMini.hpp](src/SPControladorDFPlayerMini.hpp): declara el controlador específico del DFPlayer Mini y sus operaciones de inicialización, reproducción, detención y volumen.
- [SPControladorDFPlayerMini.cpp](src/SPControladorDFPlayerMini.cpp): implementa la comunicación con el DFPlayer Mini, sus pines serie, el pin `BUSY` y la reproducción de pistas.
