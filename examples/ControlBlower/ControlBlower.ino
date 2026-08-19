#include <Botones.hpp>
#include "../../src/BlowerControl.h"
#include "../../src/Config.h"

// Definición de pines según la tabla del ESP32-S3 Super Mini
const int BUTTON_PIN = PIN_BUTTON; // Botón (un pin a GPIO 7, el otro a GND)
const int TIEMPO_ESPERA = 100;
const unsigned long BLOWER_ACTIVATION_TIME_MS =
    BLOWER_PRE_TIME_MS + GUILLOTINE_OPEN_TIME_MS + BLOWER_POST_CLEAN_TIME_MS;

BlowerControl blower;
Controlador Ctrl;
unsigned long blowerActivatedAt = 0;

void activateBlower() {
  if (blower.isOn()) {
    return;
  }

  blower.turnOn();
  blowerActivatedAt = millis();
  Serial.println("Estado: Botón PRESIONADO -> Blower ENCENDIDO");
}

void setup() {
  Serial.begin(115200);

  blower.begin();

  Ctrl.RegistrarAccion(BUTTON_PIN, EventoBoton::Pulsar, activateBlower);
  Ctrl.InicializarCtrl();

  Serial.println("--- Prueba de Blower con Botón Lista ---");
  Serial.print("Tiempo de activación: ");
  Serial.print(BLOWER_ACTIVATION_TIME_MS);
  Serial.println(" ms");
}

void loop() {
  Ctrl.ActualizarCtrl(TIEMPO_ESPERA);

  if (blower.isOn() && millis() - blowerActivatedAt >= BLOWER_ACTIVATION_TIME_MS) {
    blower.turnOff();
    Serial.println("Tiempo cumplido -> Blower APAGADO");
  }
}