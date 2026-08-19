/**
 * @file Config.h
 * @brief Configuración global de pines, tiempos y constantes del proyecto "Casas de Hadas - Mundo Imayina".
 * @microcontroller ESP32-S3 Super Mini
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// MAPEO DE PINES (GPIO - ESP32-S3 Super Mini)
// ==========================================
#define PIN_SERVO_GUILLOTINE   2  // GPIO 2: Señal PWM -> Servo de la guillotina MG90S
#define PIN_BLOWER_MOSFET     3  // GPIO 3: Salida Digital -> Control MOSFET (Blower 5V)
#define PIN_LED_DATA          4  // GPIO 4: Datos DIN -> Tira WS2812B (FastLED)
#define PIN_DFPLAYER_TX       5  // GPIO 5: ESP32 TX -> RX DFPlayer Mini (vía res 1kΩ)
#define PIN_DFPLAYER_RX       6  // GPIO 6: ESP32 RX -> TX DFPlayer Mini
#define PIN_BUTTON            7  // GPIO 7: Entrada Digital -> Botón IP67 (INPUT_PULLUP)
#define PIN_DFPLAYER_BUSY     8  // GPIO 8: Entrada Digital -> Pin BUSY DFPlayer (LOW = reproduciendo)

// ==========================================
// PARÁMETROS DE SERVO (ÁNGULOS MG90S)
// ==========================================
#define GUILLOTINE_CLOSED_ANGLE  0   // Grados con la guillotina de purpurina sellada
#define GUILLOTINE_OPEN_ANGLE   90   // Grados para apertura de liberación de purpurina

// Pulso estándar MG90S (microsegundos)
#define SERVO_MIN_PULSE        500
#define SERVO_MAX_PULSE       2400

// ==========================================
// SECUENCIA ASÍNCRONA DE PURPURINA (TIEMPOS MS)
// ==========================================
#define BLOWER_PRE_TIME_MS          200  // Pre-soplado antes de abrir guillotina (0.2s)
#define GUILLOTINE_OPEN_TIME_MS     500  // Tiempo de guillotina abierta (0.5s)
#define BLOWER_POST_CLEAN_TIME_MS  1000  // Soplado posterior para limpiar cañón (1.0s)

// ==========================================
// ILUMINACIÓN (WS2812B - FastLED)
// ==========================================
#define NUM_LEDS             8  // 8x Módulo LED WS2812B NeoPixel
#define BRIGHTNESS_IDLE      60  // Brillo tenue para efecto reposo (0-255)
#define BRIGHTNESS_SHOW     255  // Brillo máximo durante el espectáculo (0-255)
#define AMBER_SEQUENCE_INTERVAL_MS 50 // Tiempo entre LEDs de la secuencia ámbar
#define SHOW_RUNNING_DURATION_MS 7000 // Duración del estado SHOW_RUNNING

// ==========================================
// TIEMPOS DE DEBOUNCE Y SEGURIDAD
// ==========================================
#define DEBOUNCE_DELAY_MS        100  // Filtro de antirrebote para el botón (100ms)
#define WDT_TIMEOUT_SECONDS       5  // Watchdog Timer de seguridad (5 segundos)

#endif // CONFIG_H
