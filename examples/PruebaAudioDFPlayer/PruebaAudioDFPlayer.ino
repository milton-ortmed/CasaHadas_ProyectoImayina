#include <Arduino.h>
#include "../../src/Config.h"
#include "../../src/TarjetaAudio.cpp"
#include "../../src/SPControladorDFPlayerMini.hpp"
#include "../../src/SPControladorDFPlayerMini.cpp"
#include <Botones.hpp>

const int TIEMPO_ESPERA = 100;
#define MIN_TRACK 1

ControladorDFRobotDFPlayerMini reproductor(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX, PIN_DFPLAYER_BUSY);
Controlador Ctrl;
bool teclado = true;

void initDFPlayer() {
    Serial.print("Inicializando DFPlayer Mini...");
    reproductor.Inicializar();
    delay(200);
    Serial.println(" OK.");
    reproductor.EstablecerVolumen(5);
    delay(200);
}

void playTrack() {
    Serial.print("Reproduciendo pista: ");
    Serial.println(MIN_TRACK);
    reproductor.ReproducirPista(MIN_TRACK);
    delay(200);
}

void readKeyboard() {
    if (!teclado) {
        return;
    }

    while (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'r' || command == 'R') {
            Serial.println("Tecla R recibida");
            playTrack();
        }
    }
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

    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, playTrack);
    Ctrl.InicializarCtrl();

    Serial.println("Sistema listo.");
    Serial.println("Presiona el botón para reproducir la pista 1");
}

void loop() {
    Ctrl.ActualizarCtrl(TIEMPO_ESPERA);
    readKeyboard();
}