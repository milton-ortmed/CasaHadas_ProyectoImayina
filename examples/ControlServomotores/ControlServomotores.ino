#include <Botones.hpp>
#include "../../src/ServoManager.h"

#define Tiempo_Espera 100

#define BUTTON_PIN 7 // Botón conectado entre GPIO 7 y GND

const unsigned long guillotineOpenDuration = 500;

ServoManager servoManager;
Controlador Ctrl;
unsigned long guillotineOpenedAt = 0;

void activateGuillotine() {
  if (servoManager.isGuillotineOpen()) {
    return;
  }

  Serial.println("Botón presionado: abriendo guillotina");
  if (servoManager.openGuillotine()) {
    guillotineOpenedAt = millis();
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Posición inicial");
  servoManager.begin();

  Ctrl.RegistrarAccion(BUTTON_PIN, EventoBoton::Pulsar, activateGuillotine);
  Ctrl.InicializarCtrl();

  Serial.println("--- Prueba de guillotina lista ---");
  Serial.println("Presiona el botón para abrir la guillotina durante 500 ms.");
}

void loop() {
  Ctrl.ActualizarCtrl(Tiempo_Espera);

  if (servoManager.isGuillotineOpen() && millis() - guillotineOpenedAt >= guillotineOpenDuration) {
    Serial.println("500 ms cumplidos: cerrando guillotina");
    servoManager.closeGuillotine();
  }
}