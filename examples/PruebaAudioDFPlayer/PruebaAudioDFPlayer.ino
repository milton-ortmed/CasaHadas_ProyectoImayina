#include <Arduino.h>
#include "../../src/SPControladorDFPlayerMini.hpp"
#include "../../src/SPControladorDFPlayerMini.cpp"
#include <Botones.hpp>
#include "../../src/Config.h"

const int TIEMPO_ESPERA = 100;
#define MIN_TRACK 1

ControladorDFRobotDFPlayerMini reproductor; // Para ESP32-S3 Super Mini: RX 6, TX 5
Controlador Ctrl;

void initDFPlayer() {
    Serial.print("Inicializando DFPlayer Mini...");
    reproductor.Inicializar();
    Serial.println(" OK.");
    reproductor.EstablecerVolumen(5);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("  PRUEBA DFPlayer Mini - Track 1");
    Serial.println("========================================");
    Serial.println("Conecta la tarjeta SD con el audio de la pista 1");
    Serial.println("y presiona el botón conectado al GPIO 7.");

    initDFPlayer();

    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, []() {
        Serial.print("Reproduciendo pista: ");
        Serial.println(MIN_TRACK);
        reproductor.ReproducirPista(MIN_TRACK);
    });
    Ctrl.InicializarCtrl();

    Serial.println("Sistema listo.");
    Serial.println("Presiona el botón para reproducir la pista 1");
}

void loop() {
    Ctrl.ActualizarCtrl(TIEMPO_ESPERA);
}