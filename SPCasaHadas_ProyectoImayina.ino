/**
 * @file CasaHadas_ProyectoImayina.ino
 * @brief Firmware Principal - Proyecto Casas de Hadas (Mundo Imayina)
 * @microcontroller ESP32-S3 Super Mini
 * 
 * Descripción:
 * Este programa controla la iluminación, expulsión asíncrona de purpurina
 * y reproducción de audio temática para las Casas de Hadas mecánicas.
 */

#include <Arduino.h>
#include <esp_task_wdt.h>
#include <Botones.hpp>

#include "src/Config.h"
#include "src/ILightingController.h"
#include "src/FastLEDController.h"
#include "src/ServoManager.h"
#include "src/BlowerControl.h"
#include "src/SPControladorDFPlayerMini.hpp"

// ==========================================
// ESTADOS DE LA MÁQUINA DE ESTADOS FINITA (FSM)
// ==========================================
enum SystemState {
    STATE_IDLE,          // Reposó: Iluminación tenue ("hada dentro"), espera de botón
    STATE_ACTIVATED,     // Activación: Inicia audio y prepara purpurina
    STATE_SHOW_RUNNING,  // Espectáculo: Mantiene show, ejecuta secuencia de purpurina y monitorea pin BUSY
    STATE_CLOSING        // Cierre: Limpia estados y regresa a IDLE
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
ControladorDFRobotDFPlayerMini audioPlayer(PIN_DFPLAYER_RX, PIN_DFPLAYER_TX, PIN_DFPLAYER_BUSY);
Controlador Ctrl;

uint32_t purpurinaStageStartTime = 0;
uint32_t showRunningStartTime = 0;
bool purpurinaCompleted = false;
bool teclado = true;

// ==========================================
// PROTOTIPOS DE FUNCIONES
// ==========================================
void setupWatchdog();
void activateShow();
void updateFSM();
void processPurpurinaSequence();
void readKeyboard();

// ==========================================
// SETUP PRINCIPAL
// ==========================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("  INICIALIZANDO CASAS DE HADAS - MUNDO IMAYINA");
    Serial.println("=============================================");

    // Inicializar componentes
    lighting->begin();
    servos.begin();
    blower.begin();
    audioPlayer.Inicializar();
    audioPlayer.EstablecerVolumen(5);

    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, activateShow);
    Ctrl.InicializarCtrl();

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

    // 1. Actualización del controlador de botones con antirrebote
    Ctrl.ActualizarCtrl(DEBOUNCE_DELAY_MS);

    // 2. Lectura opcional del teclado por el monitor serial
    readKeyboard();

    // 3. Despachador de la Máquina de Estados Finita (FSM)
    updateFSM();

    // Cede el procesador para que las tareas idle del ESP32 se ejecuten.
    delay(1);
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
    esp_task_wdt_add(NULL);
#else
    esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL);
#endif
}

// ==========================================
// EVENTO DE BOTÓN
// ==========================================
void activateShow() {
    if (currentState != STATE_IDLE) {
        return;
    }

    Serial.println("[EVENTO] Activación recibida -> Activando Show");
    currentState = STATE_ACTIVATED;
}

void readKeyboard() {
    if (!teclado) {
        return;
    }

    while (Serial.available() > 0) {
        char command = Serial.read();
        if (command == 'r' || command == 'R') {
            Serial.println("[EVENTO] Tecla R recibida");
            activateShow();
        }
    }
}

// ==========================================
// LÓGICA DE LA MÁQUINA DE ESTADOS (FSM)
// ==========================================
void updateFSM() {
    switch (currentState) {

        case STATE_IDLE:
            // Iluminación detenida
            FastLED.clear(); // Limpiar los LEDs
            FastLED.show();
            break;

        case STATE_ACTIVATED:
            Serial.println("[FSM] Estado: ACTIVATED -> Iniciando Audio y Mecanismo");
            
            // 1. Iniciar reproducción de audio temático en DFPlayer Mini
            audioPlayer.ReproducirPista(1);

            // 2. Cambiar brillo de luces e iniciar efecto mágico
            lighting->setBrightness(BRIGHTNESS_SHOW);

            // 3. Iniciar la secuencia de purpurina
            purpurinaStage = PURPURINA_PRE_BLOWER;
            purpurinaStageStartTime = millis();
            showRunningStartTime = millis();
            purpurinaCompleted = false;
            blower.turnOn(); // Paso 1: Pre-soplado del blower

            currentState = STATE_SHOW_RUNNING;
            break;

        case STATE_SHOW_RUNNING:
            // 1. Actualizar la secuencia ámbar mientras el audio esté activo
            lighting->updateAmberSequenceEffect();

            // 2. Procesar secuencia asíncrona de purpurina
            if (!purpurinaCompleted) {
                processPurpurinaSequence();
            }

            // 3. Finalizar el show después del tiempo configurado
            if (millis() - showRunningStartTime >= SHOW_RUNNING_DURATION_MS) {
                Serial.println("[FSM] Duración de SHOW_RUNNING completada");
                currentState = STATE_CLOSING;
            }
            break;

        case STATE_CLOSING:
            Serial.println("[FSM] Estado: CLOSING -> Limpiando");

            // Asegurar que la guillotina esté cerrada y el blower apagado
            servos.closeGuillotine();
            blower.turnOff();

            // Restablecer brillo de luces para reposo
            lighting->setBrightness(BRIGHTNESS_IDLE);

            // Regresar a reposo
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
