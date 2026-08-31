#include "../../src/Botones.hpp"
#include "../../src/ServoManager.h"
#include "../../src/Config.h"

const unsigned long guillotineOpenDuration = GUILLOTINE_OPEN_TIME_MS;

ServoManager servoManager;
Controlador Ctrl;
unsigned long guillotineOpenedAt = 0;
bool teclado = true;

void activateGuillotine() {
  if (servoManager.isGuillotineOpen()) {
    return;
  }

  Serial.println("Botón presionado: abriendo guillotina");
  if (servoManager.openGuillotine()) {
    guillotineOpenedAt = millis();
  }
}

void readKeyboard() {
  if (!teclado) {
    return;
  }

  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'r' || command == 'R') {
      Serial.println("Tecla R recibida");
      activateGuillotine();
    }
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Posición inicial");
  servoManager.begin();

  Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, activateGuillotine);
  Ctrl.InicializarCtrl();

  Serial.println("--- Prueba de guillotina lista ---");
  Serial.println("Presiona el botón para abrir la guillotina durante 500 ms.");
}

void loop() {
  Ctrl.ActualizarCtrl(DEBOUNCE_DELAY_MS);
  readKeyboard();

  if (servoManager.isGuillotineOpen() && millis() - guillotineOpenedAt >= guillotineOpenDuration) {
    Serial.println("500 ms cumplidos: cerrando guillotina");
    servoManager.closeGuillotine();
  }
}