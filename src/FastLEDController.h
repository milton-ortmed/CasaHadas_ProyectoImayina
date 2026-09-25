/**
 * @file FastLEDController.h
 * @brief Implementación concreta de ILightingController utilizando la librería FastLED.
 */

#ifndef FASTLED_CONTROLLER_H
#define FASTLED_CONTROLLER_H

#include "ILightingController.h"
#include "Config.h"
#include <FastLED.h>

class FastLEDController : public ILightingController {
private:
    CRGB leds[NUM_LEDS];
    uint32_t lastUpdateMs;
    uint16_t amberSequenceIndex;

public:
    FastLEDController()
        : lastUpdateMs(0), amberSequenceIndex(0) {}

    void begin() override {
        // FastLED inicialización para WS2812B en PIN_LED_DATA (GPIO 4)
        FastLED.addLeds<WS2812B, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
        FastLED.setBrightness(BRIGHTNESS_IDLE);
        FastLED.clear();
        FastLED.show();
    }

    void setBrightness(uint8_t brightness) override {
        FastLED.setBrightness(brightness);
    }

    void clearLED(uint16_t startIndex, uint16_t endIndex){
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        for (uint16_t i = startIndex; i <= endIndex; i++) {            
            leds[i] = CRGB::Black;
        }
        
        FastLED.show();
    }

    /**
     * @brief Efecto de color sólido ambar.
     * 
     * @param startIndex Índice del primer LED del segmento a iluminar.
     * @param endIndex Índice del último LED del segmento a iluminar.
     */
    void updateSolidEffect(uint16_t startIndex, uint16_t endIndex){
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        for (uint16_t i = startIndex; i <= endIndex; i++) {            
            // Tono cálido dorado/hadas (Hue ~32 es dorado/cálido)
            leds[i] = CHSV(32, 220, 255);
        }
        
        FastLED.show();
    }

    /**
     * @brief Efecto IDLE: Respiración tenue con sutil titileo cálido ("Hada viviendo dentro").
     * Totalmente asíncrono y guiado por millis().
     * @param startIndex Índice del primer LED del segmento a iluminar.
     * @param endIndex Índice del último LED del segmento a iluminar.
     */
    void updateIdleEffect(uint16_t startIndex, uint16_t endIndex) override {
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        uint32_t now = millis();
        if (now - lastUpdateMs < 20) return; // Limitar actualización a ~50 FPS
        lastUpdateMs = now;

        // Onda senoidal para el efecto de respiración (período ~3.5 segundos)
        uint8_t breath = beatsin8(17, 40, BRIGHTNESS_IDLE);
        
        for (uint16_t i = startIndex; i <= endIndex; i++) {
            // Sutil variación por LED para simular luces de luciérnaga/hada
            uint8_t flicker = random8(15);
            uint8_t val = (breath > flicker) ? (breath - flicker) : breath;
            
            // Tono cálido dorado/hadas (Hue ~32 es dorado/cálido)
            leds[i] = CHSV(32, 220, val);
        }
        
        FastLED.show();
    }

    /**
     * @brief Efecto SHOW: Destellos mágicos estelares y pulso de luz brillante.
     * @param startIndex Índice del primer LED del segmento a iluminar.
     * @param endIndex Índice del último LED del segmento a iluminar.
     */
    void updateShowEffect(uint16_t startIndex, uint16_t endIndex) override {
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        uint32_t now = millis();
        if (now - lastUpdateMs < 15) return; // ~66 FPS para máxima fluidez
        lastUpdateMs = now;

        uint16_t count = endIndex - startIndex + 1;

        // Decaimiento paulatino de los LEDs existentes en el rango
        fadeToBlackBy(&(leds[startIndex]), count, 30);

        // Ocasionalmente genera un destello estelar (sparkle) brillante en un LED aleatorio dentro del rango
        if (random8() < 90) {
            uint16_t pos = startIndex + random16(count);
            // Colores variados mágicos (Violeta, Cían, Dorado)
            uint8_t hue = random8(120, 220);
            leds[pos] += CHSV(hue, 180, 255);
        }

        FastLED.show();
    }

    /**
     * @brief Recorre la tira encendiendo un LED ámbar cálido cada vez dentro del rango especificado.
     * @param startIndex Índice del primer LED del segmento a recorrer.
     * @param endIndex Índice del último LED del segmento a recorrer.
     */
    void updateAmberSequenceEffect(uint16_t startIndex, uint16_t endIndex) override {
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        uint32_t now = millis();
        if (now - lastUpdateMs < AMBER_SEQUENCE_INTERVAL_MS) return;
        lastUpdateMs = now;

        FastLED.setBrightness(BRIGHTNESS_SHOW);

        uint16_t count = endIndex - startIndex + 1;
        fill_solid(&(leds[startIndex]), count, CRGB::Black);

        if (amberSequenceIndex < startIndex || amberSequenceIndex > endIndex) {
            amberSequenceIndex = startIndex;
        }

        leds[amberSequenceIndex] = CHSV(32, 220, 255);
        FastLED.show();

        amberSequenceIndex++;
        if (amberSequenceIndex > endIndex) {
            amberSequenceIndex = startIndex;
        }
    }

    /**
     * @brief Recorre la tira encendiendo un LED con un color del rango del efecto SHOW dentro del rango especificado.
     * @param startIndex Índice del primer LED del segmento a recorrer.
     * @param endIndex Índice del último LED del segmento a recorrer.
     */
    void updateAmberSequenceEffect2(uint16_t startIndex, uint16_t endIndex) override {
        if (startIndex >= NUM_LEDS) return;
        if (endIndex >= NUM_LEDS) endIndex = NUM_LEDS - 1;
        if (startIndex > endIndex) {
            uint16_t temp = startIndex;
            startIndex = endIndex;
            endIndex = temp;
        }

        uint32_t now = millis();
        if (now - lastUpdateMs < AMBER_SEQUENCE_INTERVAL_MS) return;
        lastUpdateMs = now;

        FastLED.setBrightness(BRIGHTNESS_SHOW);

        uint16_t count = endIndex - startIndex + 1;
        fill_solid(&(leds[startIndex]), count, CRGB::Black);

        if (amberSequenceIndex < startIndex || amberSequenceIndex > endIndex) {
            amberSequenceIndex = startIndex;
        }

        uint8_t hue = random8(120, 220);
        leds[amberSequenceIndex] = CHSV(hue, 180, 255);
        FastLED.show();

        amberSequenceIndex++;
        if (amberSequenceIndex > endIndex) {
            amberSequenceIndex = startIndex;
        }
    }
};

#endif // FASTLED_CONTROLLER_H
