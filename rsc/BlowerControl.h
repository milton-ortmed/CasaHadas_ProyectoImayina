/**
 * @file BlowerControl.h
 * @brief Control del MOSFET del Mini Blower 3010 a 5V para el sistema de expulsión.
 */

#ifndef BLOWER_CONTROL_H
#define BLOWER_CONTROL_H

#include <Arduino.h>
#include "Config.h"

class BlowerControl {
private:
    bool running;

public:
    BlowerControl() : running(false) {}

    /**
     * @brief Configura el pin del MOSFET como salida y asegura que inicie apagado.
     */
    void begin() {
        pinMode(PIN_BLOWER_MOSFET, OUTPUT);
        turnOff();
    }

    /**
     * @brief Enciende el Mini Blower al 100% de potencia.
     */
    void turnOn() {
        digitalWrite(PIN_BLOWER_MOSFET, HIGH);
        running = true;
    }

    /**
     * @brief Apaga completamente el Mini Blower.
     */
    void turnOff() {
        digitalWrite(PIN_BLOWER_MOSFET, LOW);
        running = false;
    }

    /**
     * @brief Retorna el estado actual del soplador.
     */
    bool isOn() const {
        return running;
    }
};

#endif // BLOWER_CONTROL_H
