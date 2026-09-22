/**
 * @file ActuadorLinealControl.h
 * @brief Control del actuador lineal para mover la figura del Hada.
 */

#ifndef ACTUADOR_LINEAL_CONTROL_H
#define ACTUADOR_LINEAL_CONTROL_H

#include <Arduino.h>
#include "Config.h"

class ActuadorLinealControl {
private:
    bool running;
    const uint8_t maxPulse;
    uint32_t lastMovementCommandAt;

    bool canSendMovementCommand() const {
        return !running || millis() - lastMovementCommandAt >= 500;
    }

public:
    ActuadorLinealControl() : running(false), maxPulse(255), lastMovementCommandAt(0) {}

    /**
     * @brief Configura los pines del actuador lineal como salida y asegura que inicie apagado.
     */
    void begin() {
        pinMode(PIN_LINEAR_ACTUATOR_FWD, OUTPUT);
        pinMode(PIN_LINEAR_ACTUATOR_REV, OUTPUT);
        turnOff();
    }

    /**
      * @brief Mueve el actuador lineal hacia adelante.
     */
    void forward() {
        if (!canSendMovementCommand()) {
            return;
        }

        analogWrite(PIN_LINEAR_ACTUATOR_FWD, maxPulse);
        analogWrite(PIN_LINEAR_ACTUATOR_REV, 0);
        running = true;
        lastMovementCommandAt = millis();
    }

    /**
      * @brief Mueve el actuador lineal hacia atrás.
     */
    void reverse() {
        if (!canSendMovementCommand()) {
            return;
        }

        analogWrite(PIN_LINEAR_ACTUATOR_FWD, 0);
        analogWrite(PIN_LINEAR_ACTUATOR_REV, maxPulse);
        running = true;
        lastMovementCommandAt = millis();
    }

    /**
     * @brief Apaga completamente el actuador lineal.
     */
    void turnOff() {
        analogWrite(PIN_LINEAR_ACTUATOR_FWD, 0);
        analogWrite(PIN_LINEAR_ACTUATOR_REV, 0);
        running = false;
        lastMovementCommandAt = 0;
    }

    /**
     * @brief Retorna el estado actual del actuador lineal.
     */
    bool isOn() const {
        return running;
    }
};

#endif // ACTUADOR_LINEAL_CONTROL_H