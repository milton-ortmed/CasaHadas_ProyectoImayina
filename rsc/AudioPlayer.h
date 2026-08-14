/**
 * @file AudioPlayer.h
 * @brief Controlador independiente vía UART (HardwareSerial) para el módulo DFPlayer Mini.
 * Incluye lectura directa del pin BUSY para saber cuándo termina la pista.
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>
#include "Config.h"

class AudioPlayer {
private:
    HardwareSerial dfSerial;

    /**
     * @brief Envía un paquete de comandos de 10 bytes al DFPlayer Mini.
     */
    void sendCommand(uint8_t command, uint8_t highByte, uint8_t lowByte) {
        uint16_t checksum = -(0xFF + 0x06 + command + 0x00 + highByte + lowByte);
        uint8_t packet[10] = {
            0x7E,               // Byte de Inicio
            0xFF,               // Versión
            0x06,               // Longitud
            command,            // Comando
            0x00,               // Feedback (0x00 = No, 0x01 = Sí)
            highByte,           // Parámetro Alto
            lowByte,            // Parámetro Bajo
            (uint8_t)(checksum >> 8),   // Checksum Alto
            (uint8_t)(checksum & 0xFF), // Checksum Bajo
            0xEF                // Byte de Fin
        };

        dfSerial.write(packet, 10);
    }

public:
    AudioPlayer() : dfSerial(1) {}

    /**
     * @brief Configura el puerto HardwareSerial a 9600 baudios y el pin BUSY.
     */
    void begin() {
        // Inicializa UART1 en ESP32-S3 (TX en GPIO 5, RX en GPIO 6)
        dfSerial.begin(9600, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);

        // Configuración del pin BUSY para monitorear reproducción
        pinMode(PIN_DFPLAYER_BUSY, INPUT_PULLUP);

        delay(500); // Pequeña pausa para estabilizar el módulo al encender
        setVolume(25); // Volumen inicial sugerido (0 - 30)
    }

    /**
     * @brief Establece el volumen de salida.
     * @param volume Nivel de 0 a 30.
     */
    void setVolume(uint8_t volume) {
        if (volume > 30) volume = 30;
        sendCommand(0x06, 0x00, volume);
    }

    /**
     * @brief Reproduce un archivo de audio específico en la carpeta SD.
     * @param track Nivel de pista (1 a 255).
     */
    void playTrack(uint8_t track) {
        sendCommand(0x03, 0x00, track);
    }

    /**
     * @brief Detiene la reproducción actual.
     */
    void stop() {
        sendCommand(0x16, 0x00, 0x00);
    }

    /**
     * @brief Comprueba si el DFPlayer Mini está reproduciendo audio.
     * @return true si está reproduciendo (pin BUSY = LOW), false en silencio (pin BUSY = HIGH).
     */
    bool isPlaying() const {
        return (digitalRead(PIN_DFPLAYER_BUSY) == LOW);
    }
};

#endif // AUDIO_PLAYER_H
