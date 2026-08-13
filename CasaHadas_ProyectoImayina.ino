/**
 * @file CasaHadas_ProyectoImayina.ino
 * @brief Firmware Principal - Proyecto Casas de Hadas (Mundo Imayina)
 * @microcontroller ESP32-S3 Super Mini
 * 
 * Descripción:
 * Este programa controla la iluminación, apertura de puerta, expulsión asíncrona de purpurina
 * y reproducción de audio temática para las Casas de Hadas mecánicas.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>

#include "Config.h"
#include "ILightingController.h"
#include "FastLEDController.h"
#include "ServoManager.h"
#include "BlowerControl.h"
#include "AudioPlayer.h"

// ==========================================
// ESTADOS DE LA MÁQUINA DE ESTADOS FINITA (FSM)
// ==========================================
enum SystemState {
    STATE_IDLE,          // Reposó: Iluminación tenue ("hada dentro"), espera de botón
    STATE_ACTIVATED,     // Activación: Inicia audio, abre puerta y prepara purpurina
    STATE_SHOW_RUNNING,  // Espectáculo: Mantiene show, ejecuta secuencia de purpurina y monitorea pin BUSY
    STATE_CLOSING        // Cierre: Cierra puerta, limpia estados y regresa a IDLE
};

// Sub-estados de la secuencia asíncrona de purpurina (previene atascos)
enum PurpurinaStage {
    PURPURINA_IDLE,
    PURPURINA_PRE_BLOWER,       // Pre-soplado con Blower al 100%
    PURPURINA_OPEN_GUILLOTINE,  // Apertura de Guillotina para liberación de mica
    PURPURINA_POST_CLEAN,       // Soplado de limpieza de cañón post-cierre
    PURPURINA_DONE              // Secuencia de purpurina finalizada
};

// ==========================================
// INSTANCIAS DE COMPONENTES DE SOFTWARE
// ==========================================
SystemState currentState = STATE_IDLE;
PurpurinaStage purpurinaStage = PURPURINA_IDLE;

// Abstracción del controlador de iluminación (FastLED)
ILightingController* lighting = new FastLEDController();

ServoManager servos;
BlowerControl blower;
AudioPlayer audioPlayer;

// Variables de control de tiempos (millis)
uint32_t lastDebounceTime = 0;
int lastButtonState = HIGH;
int buttonState = HIGH;

uint32_t purpurinaStageStartTime = 0;
uint32_t showStartTime = 0;
bool purpurinaCompleted = false;

// ==========================================
// PROTOTIPOS DE FUNCIONES
// ==========================================
void setupWatchdog();
void readButton();
void updateFSM();
void processPurpurinaSequence();

// ==========================================
// SETUP PRINCIPAL
// ==========================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("  INICIALIZANDO CASAS DE HADAS - MUNDO IMAYINA");
    Serial.println("=============================================");

    // Inicializar configuración del botón
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    // Inicializar componentes
    lighting->begin();
    servos.begin();
    blower.begin();
    audioPlayer.begin();

    // Configurar Watchdog Timer de seguridad
    setupWatchdog();

    Serial.println("Sistema iniciado correctamente. Estado: IDLE");
}

// ==========================================
// BUCLE PRINCIPAL (LOOP)
// ==========================================
void loop() {
    // Alimenta el Watchdog Timer para evitar reinicios por falso colgado
    esp_task_wdt_reset();

    // 1. Lectura del botón con filtrado antirrebote no bloqueante
    readButton();

    // 2. Despachador de la Máquina de Estados Finita (FSM)
    updateFSM();
}

// ==========================================
// CONFIGURACIÓN DEL WATCHDOG TIMER (WDT)
// ==========================================
void setupWatchdog() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT_SECONDS * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    esp_task_wdt_reconfigure(&wdt_config);
#else
    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL);
#endif
}

// ==========================================
// LECTURA DE BOTÓN CON ANTIRREBOTE (DEBOUNCE)
// ==========================================
void readButton() {
    int reading = digitalRead(PIN_BUTTON);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (reading != buttonState) {
            buttonState = reading;

            // Disparo de botón al presionar (Transición HIGH a LOW)
            if (buttonState == LOW && currentState == STATE_IDLE) {
                Serial.println("[EVENTO] Botón Antivandálico Presionado -> Activando Show");
                currentState = STATE_ACTIVATED;
            }
        }
    }

    lastButtonState = reading;
}

// ==========================================
// LÓGICA DE LA MÁQUINA DE ESTADOS (FSM)
// ==========================================
void updateFSM() {
    switch (currentState) {

        case STATE_IDLE:
            // Iluminación tenue parpadeante ("Hada viviendo dentro")
            lighting->updateIdleEffect();
            break;

        case STATE_ACTIVATED:
            Serial.println("[FSM] Estado: ACTIVATED -> Iniciando Audio y Mecanismo");
            
            // 1. Iniciar reproducción de audio temático en DFPlayer Mini
            audioPlayer.playTrack(1);

            // 2. Cambiar brillo de luces e iniciar efecto mágico
            lighting->setBrightness(BRIGHTNESS_SHOW);

            // 3. Abrir la puerta principal
            servos.openDoor();

            // 4. Iniciar la secuencia de purpurina
            purpurinaStage = PURPURINA_PRE_BLOWER;
            purpurinaStageStartTime = millis();
            showStartTime = millis();
            purpurinaCompleted = false;
            blower.turnOn(); // Paso 1: Pre-soplado del blower

            currentState = STATE_SHOW_RUNNING;
            break;

        case STATE_SHOW_RUNNING:
            // 1. Actualizar la animación brillante de LEDs
            lighting->updateShowEffect();

            // 2. Procesar secuencia asíncrona de purpurina
            if (!purpurinaCompleted) {
                processPurpurinaSequence();
            }

            // 3. Monitorear finalización del show mediante el pin BUSY del DFPlayer Mini
            // NOTA: pin BUSY = LOW mientras suena audio, HIGH al terminar.
            {
                bool isAudioPlaying = audioPlayer.isPlaying();
                
                // Timeout de seguridad por si el audio falla (ej. 60 segundos máx)
                bool audioTimeout = (millis() - showStartTime > 60000);

                if (purpurinaCompleted && (!isAudioPlaying || audioTimeout)) {
                    if (audioTimeout) {
                        Serial.println("[ALERTA] Timeout de seguridad de audio alcanzado");
                    } else {
                        Serial.println("[FSM] Audio finalizado detectado en pin BUSY");
                    }
                    currentState = STATE_CLOSING;
                }
            }
            break;

        case STATE_CLOSING:
            Serial.println("[FSM] Estado: CLOSING -> Cerrando Puerta y Limpiando");

            // 1. Regresar la puerta a la posición cerrada
            servos.closeDoor();

            // 2. Asegurar que la guillotina esté cerrada y el blower apagado
            servos.closeGuillotine();
            blower.turnOff();

            // 3. Restablecer brillo de luces para reposo
            lighting->setBrightness(BRIGHTNESS_IDLE);

            // 4. Regresar a reposo
            currentState = STATE_IDLE;
            Serial.println("[FSM] Retorno a STATE_IDLE completado");
            break;
    }
}

// ==========================================
// SECUENCIA ASÍNCRONA ANTI-ATASCOS DE PURPURINA
// ==========================================
void processPurpurinaSequence() {
    uint32_t elapsedTime = millis() - purpurinaStageStartTime;

    switch (purpurinaStage) {

        case PURPURINA_PRE_BLOWER:
            // Paso 1: Blower encendido pre-soplado por 0.2s
            if (elapsedTime >= BLOWER_PRE_TIME_MS) {
                Serial.println("[PURPURINA] Abriendo Guillotina de Mica");
                servos.openGuillotine();
                purpurinaStage = PURPURINA_OPEN_GUILLOTINE;
                purpurinaStageStartTime = millis();
            }
            break;

        case PURPURINA_OPEN_GUILLOTINE:
            // Paso 2: Guillotina abierta durante 0.5s para liberar mica
            if (elapsedTime >= GUILLOTINE_OPEN_TIME_MS) {
                Serial.println("[PURPURINA] Cerrando Guillotina");
                servos.closeGuillotine();
                purpurinaStage = PURPURINA_POST_CLEAN;
                purpurinaStageStartTime = millis();
            }
            break;

        case PURPURINA_POST_CLEAN:
            // Paso 3: Mantener Blower encendido 1.0s adicional para limpiar cañón
            if (elapsedTime >= BLOWER_POST_CLEAN_TIME_MS) {
                Serial.println("[PURPURINA] Ciclo de Limpieza de Cañón Completado -> Apagando Blower");
                blower.turnOff();
                purpurinaStage = PURPURINA_DONE;
                purpurinaCompleted = true;
            }
            break;

        case PURPURINA_DONE:
        case PURPURINA_IDLE:
            break;
    }
}
