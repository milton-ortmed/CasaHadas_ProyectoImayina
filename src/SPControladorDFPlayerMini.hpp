#include "TarjetaAudio.hpp"
#include <DFRobotDFPlayerMini.h>

#pragma once

class ControladorDFRobotDFPlayerMini : public ControladorTarjetaAudio {
    protected:
        DFRobotDFPlayerMini audio;
        Stream* serial = NULL;
        bool esHardwareSerial = true;
        uint8_t pinRx = 0;
        uint8_t pinTx = 0;
        uint8_t pinBusy = 0;
        static const uint32_t BAUDAJE = 9600;
        static const int VOLUMEN_DEFAULT = 3;
    public:
        ControladorDFRobotDFPlayerMini();
        ControladorDFRobotDFPlayerMini(uint8_t rx, uint8_t tx);
        ControladorDFRobotDFPlayerMini(uint8_t rx, uint8_t tx, uint8_t busy);
        ~ControladorDFRobotDFPlayerMini();
        void Inicializar() override;
        void ReproducirPista(int pista, bool repetir = false) override;
        void Detener() override;
        bool isPlaying() const;
        // Cambia el volumen del reproductor de audio. Para esta implementación, el nivel mínimo es 0 (sin volumen) y el máximo es 5.
        void EstablecerVolumen(int nivel) override;
};

