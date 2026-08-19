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
    const uint8_t maxPulse;

public:
    BlowerControl() : running(false), maxPulse(60) {}

    /**
     * @brief Configura el pin del MOSFET como salida y asegura que inicie apagado.
     */
    void begin() {
        pinMode(PIN_BLOWER_MOSFET, OUTPUT);
        turnOff();
    }

    /**
      * @brief Enciende el Mini Blower con un ciclo de trabajo PWM de 60/255.
     */
    void turnOn() {
          analogWrite(PIN_BLOWER_MOSFET, maxPulse);
        running = true;
    }

    /**
     * @brief Apaga completamente el Mini Blower.
     */
    void turnOff() {
        analogWrite(PIN_BLOWER_MOSFET, 0);
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
