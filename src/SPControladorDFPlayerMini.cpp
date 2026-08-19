#include "SPControladorDFPlayerMini.hpp"

ControladorDFRobotDFPlayerMini::ControladorDFRobotDFPlayerMini() {
    esHardwareSerial = true;
    pinRx = 6;
    pinTx = 5;
    pinBusy = 8;
    serial = &Serial1;
}

ControladorDFRobotDFPlayerMini::ControladorDFRobotDFPlayerMini(uint8_t rx, uint8_t tx)
    : ControladorDFRobotDFPlayerMini(rx, tx, 8) {}

ControladorDFRobotDFPlayerMini::ControladorDFRobotDFPlayerMini(uint8_t rx, uint8_t tx, uint8_t busy) {
    esHardwareSerial = false;
    pinRx = rx;
    pinTx = tx;
    pinBusy = busy;
    serial = new SoftwareSerial(rx, tx);
}

ControladorDFRobotDFPlayerMini::~ControladorDFRobotDFPlayerMini() {
    if (!esHardwareSerial) {
        delete serial;
        serial = NULL;
    }
}

void ControladorDFRobotDFPlayerMini::Inicializar() {
    pinMode(pinBusy, INPUT_PULLUP);

    if (esHardwareSerial) {
        static_cast<HardwareSerial*>(serial)->begin(BAUDAJE, SERIAL_8N1, pinRx, pinTx);
    }
    else {
        static_cast<SoftwareSerial*>(serial)->begin(BAUDAJE);
    }
    // Actualmente no se espera que haya conexión entre el pin RX del microcontrolador y el pin TX del DFPlayer Mini.
    // Es decir, no se espera que hayan respuestas por parte del reproductor de audio.
    audio.begin(*serial, false, false);
    audio.reset();
    delay(1200);
}

void ControladorDFRobotDFPlayerMini::ReproducirPista(int pista, bool repetir) {
    if (!repetir) {
        audio.play(pista);
    }
    else {
        audio.loop(pista);
    }
}

void ControladorDFRobotDFPlayerMini::Detener() {
    audio.stop();
}
    
bool ControladorDFRobotDFPlayerMini::isPlaying() const {
    return digitalRead(pinBusy) == LOW;
}

void ControladorDFRobotDFPlayerMini::EstablecerVolumen(int nivel) {
    // El controlador para el DFPlayer Mini acepta niveles de volumen entre 0 y 30
    // Para esta implementación el volumen se limitará a 6 niveles (donde el nivel 0 es sin volumen y 5 es el nivel máximo)
    // que cubrirán solamente el intervalo [0..25] en los niveles aceptados por DFPlayer Mini.
    volumen = constrain(nivel, 0, 5);
    audio.volume(5 * volumen);
}
