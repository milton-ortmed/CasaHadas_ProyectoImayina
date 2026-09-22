#include <Botones.hpp>
#include "../../src/ActuadorLinealControl.h"
#include "../../src/Config.h"

const int BUTTON_PIN = PIN_BUTTON;
const int TIEMPO_ESPERA = 100;
const unsigned long LINEAR_ACTUATOR_ACTIVATION_TIME_MS = LINEAR_ACTUATOR_DURATION_MS;

ActuadorLinealControl actuadorLineal;
Controlador Ctrl;
bool teclado = true;

void activateActuadorLineal() {
  actuadorLineal.forward();
  Serial.println("Estado: Botón PRESIONADO -> Actuador Lineal ENCENDIDO");
}

void readKeyboard() {
  if (!teclado) {
    return;
  }

  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'r' || command == 'R') {
      Serial.println("Tecla R recibida");
      activateActuadorLineal();
    }
  }
}

void setup() {
  Serial.begin(115200);

  actuadorLineal.begin();

  Ctrl.RegistrarAccion(BUTTON_PIN, EventoBoton::Pulsar, activateActuadorLineal);
  Ctrl.InicializarCtrl();

  Serial.println("--- Prueba de Actuador Lineal con Botón Lista ---");
  Serial.print("Tiempo de activación: ");
  Serial.print(LINEAR_ACTUATOR_ACTIVATION_TIME_MS);
  Serial.println(" ms");
}

void loop() {
  Ctrl.ActualizarCtrl(TIEMPO_ESPERA);
  readKeyboard();

  if (actuadorLineal.isOn() && millis() - actuadorLinealActivatedAt >= LINEAR_ACTUATOR_ACTIVATION_TIME_MS) {
    actuadorLineal.turnOff();
    Serial.println("Tiempo cumplido -> Actuador Lineal APAGADO");
  }
}