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

public:
    FastLEDController() : lastUpdateMs(0) {}

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

    /**
     * @brief Efecto IDLE: Respiración tenue con sutil titileo cálido ("Hada viviendo dentro").
     * Totalmente asíncrono y guiado por millis().
     */
    void updateIdleEffect() override {
        uint32_t now = millis();
        if (now - lastUpdateMs < 20) return; // Limitar actualización a ~50 FPS
        lastUpdateMs = now;

        // Onda senoidal para el efecto de respiración (período ~3.5 segundos)
        uint8_t breath = beatsin8(17, 40, BRIGHTNESS_IDLE);
        
        for (int i = 0; i < NUM_LEDS; i++) {
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
     */
    void updateShowEffect() override {
        uint32_t now = millis();
        if (now - lastUpdateMs < 15) return; // ~66 FPS para máxima fluidez
        lastUpdateMs = now;

        // Decaimiento paulatino de los LEDs existentes
        fadeToBlackBy(leds, NUM_LEDS, 30);

        // Ocasionalmente genera un destello estelar (sparkle) brillante en un LED aleatorio
        if (random8() < 90) {
            int pos = random16(NUM_LEDS);
            // Colores variados mágicos (Violeta, Cían, Dorado)
            uint8_t hue = random8(120, 220);
            leds[pos] += CHSV(hue, 180, 255);
        }

        FastLED.show();
    }
};

#endif // FASTLED_CONTROLLER_H
