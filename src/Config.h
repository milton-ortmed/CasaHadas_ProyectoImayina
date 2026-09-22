/**
 * @file Config.h
 * @brief Configuración global de pines, tiempos y constantes del proyecto "Casas de Hadas - Mundo Imayina".
 * @microcontroller ESP32 WROOM32 o ESP32-S3 Super Mini
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// SELECCIÓN DE HARDWARE
// ==========================================
// 1 = ESP32 WROOM32 con módulo LoRa
// 0 = ESP32-S3 Super Mini sin LoRa (configuración anterior)
#ifndef USE_ESP32_WROOM32
#define USE_ESP32_WROOM32 1
#endif

#if USE_ESP32_WROOM32

// ==========================================
// MAPEO DE PINES (ESP32 WROOM32 + LoRa)
// ==========================================
#define PIN_SERVO_GUILLOTINE      13 // GPIO 13: Señal PWM -> Servo de la guillotina MG90S
#define PIN_BLOWER_MOSFET         33 // GPIO 33: Salida digital -> Control MOSFET del blower (5V)
#define PIN_VENTILATOR_MOSFET     27 // GPIO 27: Salida digital -> Control MOSFET del ventilador (5V)
#define PIN_LINEAR_ACTUATOR_FWD   22 // GPIO 22: Salida digital -> Actuador lineal hacia adelante
#define PIN_LINEAR_ACTUATOR_REV   21 // GPIO 21: Salida digital -> Actuador lineal hacia atrás
#define PIN_LED_DATA               4 // GPIO 4: Datos DIN -> Tira WS2812B (FastLED)
#define PIN_HADA_LED               5 // GPIO 5: LED del hada (ver nota de bootstrapping más abajo)
#define PIN_DFPLAYER_TX           17 // GPIO 17: TX UART2 -> RX DFPlayer Mini
#define PIN_DFPLAYER_RX           16 // GPIO 16: RX UART2 <- TX DFPlayer Mini
#define PIN_BUTTON                32 // GPIO 32: Entrada digital -> Botón IP67
#define PIN_DFPLAYER_BUSY         35 // GPIO 35: Entrada solo entrada -> BUSY DFPlayer

// Nota importante: GPIO 4 y GPIO 5 son pines de arranque/strapping del ESP32.
// Son funcionales para salidas normales si el pin queda libre en el arranque,
// pero no conviene forzarlos con pull-down o conexiones externas en el encendido.

// LoRa SPI VSPI: SCK 18, MISO 19, MOSI 23.
#define PIN_LORA_SCK              18 // GPIO 18: SCK del módulo LoRa
#define PIN_LORA_MISO             19 // GPIO 19: MISO del módulo LoRa
#define PIN_LORA_MOSI             23 // GPIO 23: MOSI del módulo LoRa
#define PIN_LORA_CS               25 // GPIO 25: NSS/CS del módulo LoRa
#define PIN_LORA_RST              14 // GPIO 14: RESET del módulo LoRa
#define PIN_LORA_DIO0             26 // GPIO 26: DIO0/IRQ del módulo LoRa

#else

// ==========================================
// MAPEO DE PINES (ESP32-S3 Super Mini, anterior)
// ==========================================
#define PIN_SERVO_GUILLOTINE    5 // GPIO 5: Señal PWM -> Servo de la guillotina MG90S
#define PIN_BLOWER_MOSFET       2 // GPIO 2: Salida digital -> Control MOSFET (Blower 5V)
#define PIN_LED_DATA           10 // GPIO 10: Datos DIN -> Tira WS2812B (FastLED)
#define PIN_DFPLAYER_TX         6 // GPIO 6: TX -> RX DFPlayer Mini
#define PIN_DFPLAYER_RX         7 // GPIO 7: RX <- TX DFPlayer Mini
#define PIN_BUTTON              9 // GPIO 9: Entrada digital -> Botón IP67
#define PIN_DFPLAYER_BUSY       8 // GPIO 8: Entrada digital -> BUSY DFPlayer

// El módulo LoRa no está instalado en esta configuración.
#define PIN_LORA_SCK           -1
#define PIN_LORA_MISO          -1
#define PIN_LORA_MOSI          -1
#define PIN_LORA_CS            -1
#define PIN_LORA_RST           -1
#define PIN_LORA_DIO0          -1

#endif

// ==========================================
// PARÁMETROS DE SERVO (ÁNGULOS MG90S)
// ==========================================
#define GUILLOTINE_CLOSED_ANGLE  0   // Grados con la guillotina de purpurina sellada
#define GUILLOTINE_OPEN_ANGLE   180   // Grados para apertura de liberación de purpurina

// Pulso estándar MG90S (microsegundos)
#define SERVO_MIN_PULSE        500
#define SERVO_MAX_PULSE       2400

// ==========================================
// PARÁMETROS DE ACTUADOR LINEAL
// ==========================================
#define LINEAR_ACTUATOR_DURATION_MS  5000 // Tiempo de movimiento del actuador lineal (5.0s)

// ==========================================
// SECUENCIA ASÍNCRONA DE PURPURINA (TIEMPOS MS)
// ==========================================
#define GUILLOTINE_OPEN_TIME_MS     3000  // Tiempo de guillotina abierta (3.0s)
#define BLOWER_START_DELAY_MS       500  // Espera después de cerrar la guillotina (0.5s)
#define BLOWER_DURATION_MS         6000  // Tiempo de funcionamiento del blower (2.0s)

// ==========================================
// ILUMINACIÓN (WS2812B - FastLED)
// ==========================================
#define NUM_LEDS             30  // 30x Módulo LED WS2812B NeoPixel
#define BRIGHTNESS_IDLE      60  // Brillo tenue para efecto reposo (0-255)
#define BRIGHTNESS_SHOW     255  // Brillo máximo durante el espectáculo (0-255)
#define AMBER_SEQUENCE_INTERVAL_MS 50 // Tiempo entre LEDs de la secuencia ámbar
#define SHOW_RUNNING_DURATION_MS 10000 // Duración del estado SHOW_RUNNING
#define AUTO_SHOW_RUNNING_DURATION_MS 7000 // Duración del estado SHOW_AUTO
// ==========================================
// TIEMPOS DE DEBOUNCE Y SEGURIDAD
// ==========================================
#define DEBOUNCE_DELAY_MS        100  // Filtro de antirrebote para el botón (100ms)
#define WDT_TIMEOUT_SECONDS       5  // Watchdog Timer de seguridad (5 segundos)

#endif // CONFIG_H
