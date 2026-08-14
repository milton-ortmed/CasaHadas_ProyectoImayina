/**
 * @file ServoManager.h
 * @brief Gestión y control de los microservos MG90S (Puerta y Guillotina) para ESP32-S3.
 */

#ifndef SERVO_MANAGER_H
#define SERVO_MANAGER_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "Config.h"

class ServoManager {
private:
    Servo doorServo;
    Servo guillotineServo;

    bool isDoorOpenState;
    bool isGuillotineOpenState;

public:
    ServoManager() : isDoorOpenState(false), isGuillotineOpenState(false) {}

    /**
     * @brief Inicializa los timers de PWM y vincula los servos a sus respectivos pines.
     */
    void begin() {
        // Asignar temporizadores de PWM requeridos por ESP32Servo
        ESP32PWM::allocateTimer(0);
        ESP32PWM::allocateTimer(1);

        doorServo.setPeriodHertz(50); // Frecuencia estándar 50 Hz para MG90S
        guillotineServo.setPeriodHertz(50);

        // Vinculación a pines GPIO con rango de pulsos de 500us a 2400us
        doorServo.attach(PIN_SERVO_DOOR, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
        guillotineServo.attach(PIN_SERVO_GUILLOTINE, SERVO_MIN_PULSE, SERVO_MAX_PULSE);

        // Establecer posiciones iniciales de reposo (cerradas)
        closeDoor();
        closeGuillotine();
    }

    /**
     * @brief Abre la puerta principal de la casa de hadas.
     */
    void openDoor() {
        doorServo.write(DOOR_OPEN_ANGLE);
        isDoorOpenState = true;
    }

    /**
     * @brief Cierra la puerta principal de la casa de hadas.
     */
    void closeDoor() {
        doorServo.write(DOOR_CLOSED_ANGLE);
        isDoorOpenState = false;
    }

    /**
     * @brief Abre la guillotina del dispensador de purpurina.
     */
    void openGuillotine() {
        guillotineServo.write(GUILLOTINE_OPEN_ANGLE);
        isGuillotineOpenState = true;
    }

    /**
     * @brief Cierra y sella la guillotina del dispensador de purpurina.
     */
    void closeGuillotine() {
        guillotineServo.write(GUILLOTINE_CLOSED_ANGLE);
        isGuillotineOpenState = false;
    }

    bool isDoorOpen() const { return isDoorOpenState; }
    bool isGuillotineOpen() const { return isGuillotineOpenState; }
};

#endif // SERVO_MANAGER_H
