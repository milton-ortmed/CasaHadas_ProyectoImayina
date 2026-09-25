/**
 * @file ILightingController.h
 * @brief Interfaz abstracta C++ pura para el control de iluminación en el proyecto Casa de Hadas.
 * Permite cambiar la implementación subyacente (FastLED, Adafruit NeoPixel, etc.) sin modificar el resto del sistema.
 */

#ifndef I_LIGHTING_CONTROLLER_H
#define I_LIGHTING_CONTROLLER_H

#include <Arduino.h>

class ILightingController {
public:
    virtual ~ILightingController() {}

    /**
     * @brief Inicializa los pines y periféricos de la tira LED.
     */
    virtual void begin() = 0;

    /**
     * @brief Poner LEDs en blanco.
     * 
     * @param startIndex Índice del primer LED del segmento a limpiar.
     * @param endIndex Índice del último LED del segmento a limpiar.
     */
    virtual void clearLED(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Efecto de color sólido ambar.
     * 
     * @param startIndex Índice del primer LED del segmento a iluminar.
     * @param endIndex Índice del último LED del segmento a iluminar.
     */
    virtual void updateSolidEffect(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Actualiza la animación de estado en reposo (IDLE).
     * Efecto tenue parpadeante/respiración ("Fairy Glow"). No bloqueante.
     * @param startIndex Índice del primer LED del segmento.
     * @param endIndex Índice del último LED del segmento.
     */
    virtual void updateIdleEffect(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Actualiza la animación durante el espectáculo (SHOW_RUNNING).
     * Efecto mágico brillante con destellos. No bloqueante.
     * @param startIndex Índice del primer LED del segmento.
     * @param endIndex Índice del último LED del segmento.
     */
    virtual void updateShowEffect(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Actualiza la secuencia de recorrido ámbar durante el espectáculo.
     * @param startIndex Índice del primer LED del segmento.
     * @param endIndex Índice del último LED del segmento.
     */
    virtual void updateAmberSequenceEffect(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Actualiza una secuencia de recorrido con colores del efecto SHOW.
     * @param startIndex Índice del primer LED del segmento.
     * @param endIndex Índice del último LED del segmento.
     */
    virtual void updateAmberSequenceEffect2(uint16_t startIndex, uint16_t endIndex) = 0;

    /**
     * @brief Define el nivel de brillo general de los LEDs.
     * @param brightness Valor entre 0 y 255.
     */
    virtual void setBrightness(uint8_t brightness) = 0;
};

#endif // I_LIGHTING_CONTROLLER_H
