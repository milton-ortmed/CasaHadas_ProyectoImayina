/**
 * @file PruebaTiraLED.ino
 * @brief Prueba independiente de la tira WS2812B/NeoPixel del proyecto.
 *
 * Hardware esperado:
 * - ESP32-S3 Super Mini
 * - DIN de la tira conectado a GPIO 4
 * - GND de la tira conectado a GND del ESP32
 * - Fuente de 5 V adecuada para la tira
 */

#include <Arduino.h>
#include <Botones.hpp>
#include "../../src/FastLEDController.h"
#include "../../src/Config.h"

enum TestMode {
    MODE_IDLE,
    MODE_SHOW
};

FastLEDController lighting;
Controlador Ctrl;
TestMode currentMode = MODE_IDLE;
uint32_t modeStartedAt = 0;
int showChoosen = 0;
bool teclado = true;

const uint32_t SHOW_DURATION_MS = SHOW_RUNNING_DURATION_MS;

void startShow();
void readKeyboard();

void setup() {
    Serial.begin(115200);
    delay(500);

    lighting.begin();
    lighting.setBrightness(BRIGHTNESS_IDLE);
    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, startShow);
    Ctrl.InicializarCtrl();

    Serial.println();
    Serial.println("========================================");
    Serial.println("   PRUEBA DE TIRA LED RGB NEOPIXEL");
    Serial.println("========================================");
    Serial.print("Pin de datos: GPIO ");
    Serial.println(PIN_LED_DATA);
    Serial.print("Pin del boton: GPIO ");
    Serial.println(PIN_BUTTON);
    Serial.print("Cantidad de LEDs: ");
    Serial.println(NUM_LEDS);
    Serial.println("Modo IDLE: brillo tenue y color calido");
    Serial.println("Presiona el boton para iniciar SHOW durante 7 segundos");
}

void loop() {
    Ctrl.ActualizarCtrl(DEBOUNCE_DELAY_MS);
    readKeyboard();

    if (currentMode == MODE_SHOW && millis() - modeStartedAt >= SHOW_DURATION_MS) {
        currentMode = MODE_IDLE;
        lighting.setBrightness(BRIGHTNESS_IDLE);
        Serial.println("SHOW terminado: regresando a IDLE");
    }

    if (currentMode == MODE_IDLE) {
        //lighting.updateIdleEffect(); // Secuencia de respiración y titileo cálido
        FastLED.clear(); // Limpiar los LEDs
        FastLED.show();
    } else {
        if (showChoosen == 1) {
            lighting.updateShowEffect(); // Secuencia de destellos mágicos y pulso brillante
        } else {
            lighting.updateAmberSequenceEffect2(); // Secuencia de colores del efecto SHOW
        }
    }
}

void startShow() {
    if (currentMode != MODE_IDLE) {
        return;
    }

    currentMode = MODE_SHOW;
    showChoosen = (showChoosen + 1) % 2;
    modeStartedAt = millis();
    lighting.setBrightness(BRIGHTNESS_SHOW);
    Serial.println("Boton presionado: iniciando SHOW durante 7 segundos");
}

void readKeyboard() {
    if (!teclado) {
        return;
    }

    while (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'r' || command == 'R') {
            Serial.println("Tecla R recibida");
            startShow();
        }
    }
}
