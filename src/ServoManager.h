/**
 * @file ServoManager.h
 * @brief Gestión y control del microservo MG90S de la guillotina para ESP32-S3.
 */

#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "Config.h"

class ServoManager {
private:
    Servo guillotineServo;

    bool isGuillotineOpenState;
    uint32_t lastMovementCommandAt;
    bool hasMovementCommand;

    bool canSendMovementCommand() const {
        return !hasMovementCommand || millis() - lastMovementCommandAt >= 300;
    }

public:
    ServoManager() : isGuillotineOpenState(false), lastMovementCommandAt(0), hasMovementCommand(false) {}

    /**
     * @brief Inicializa los timers de PWM y vincula los servos a sus respectivos pines.
     */
    void begin() {
        // Asignar temporizadores de PWM requeridos por ESP32Servo
        ESP32PWM::allocateTimer(0);

        guillotineServo.setPeriodHertz(50); // Frecuencia estándar 50 Hz para MG90S

        // Vinculación a pines GPIO con rango de pulsos de 500us a 2400us
        guillotineServo.attach(PIN_SERVO_GUILLOTINE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);

        // Establecer la posición inicial de reposo (cerrada)
        closeGuillotine();
    }

    /**
     * @brief Abre la guillotina del dispensador de purpurina.
     */
    bool openGuillotine() {
        if (!canSendMovementCommand()) {
            return false;
        }

        guillotineServo.write(GUILLOTINE_OPEN_ANGLE);
        isGuillotineOpenState = true;
        lastMovementCommandAt = millis();
        hasMovementCommand = true;
        return true;
    }

    /**
     * @brief Cierra y sella la guillotina del dispensador de purpurina.
     */
    bool closeGuillotine() {
        if (!canSendMovementCommand()) {
            return false;
        }

        guillotineServo.write(GUILLOTINE_CLOSED_ANGLE);
        isGuillotineOpenState = false;
        lastMovementCommandAt = millis();
        hasMovementCommand = true;
        return true;
    }

    bool isGuillotineOpen() const { return isGuillotineOpenState; }
};

#endif // SERVO_MANAGER_H
