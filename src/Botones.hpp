/*
 * Proyecto: Controlador de Hardware Modular
 * Empresa: Ortmed S.A. de C.V.
 * Autor: Milton Elias Pech Uc (Ing. Floppa)
 * Descripción: Manejador de eventos de entrada digital con debounce.
 */

#pragma once

#include <Arduino.h>
#include <map>

/**
 * @brief Herramienta para usar botones pulsadores.
 * Gestiona internamente el debounce.
 */
class Boton {
    private:
        int Pin;
        bool Lectura;
        bool Estado;
        unsigned long Tiempo_Anterior;
        /**
         * @brief Actualiza el estado del botón si el tiempo de espera ha transcurrido.
         * @param Tiempo_Espera Periodo mínimo de actualización de lectura (debounce).
         */
        void DefinirEstado(int Tiempo_Espera);

    public:
        Boton(int Pin);
        /**
         * @brief Configura un pin como entrada digital.
         */
        void Inicializar();
        /**
         * @brief Realiza la lectura del botón en el pin asignado.
         * @param Tiempo_Espera Periodo mínimo de actualización de lectura (debounce).
         */
        bool Sensado(int Tiempo_Espera);
        /**
         * @brief Permite obtener la última lectura sensada del botón.
         */
        bool GetEstado();
};

/*
enum class AccionCtrl {
    Motor,
    Led,
    Calentador,
};
*/

enum class EventoBoton {
    Pulsar,
    Soltar,
};

/**
 * @brief Controlador de botones con resistencias de pull-up para ESP32 con manejo de eventos por callback.
 * Permite registrar acciones (lambdas) para eventos de pulsación y liberación,
 * gestionando internamente el debounce y la lógica de estados.
 */
class Controlador {
    private:
        struct ContextoBoton{
            Boton Btn;
            std::function<void()> accionPulsar = nullptr;
            std::function<void()> accionSoltar = nullptr;

            ContextoBoton(int Pin) : Btn(Pin) {}
        };
        std::map<int, ContextoBoton> MapaBotones;

    public:
        Controlador();
        /**
         * @brief Registra una acción para un evento de botón específico.
         * @param Pin GPIO donde está conectado el botón.
         * @param evento El tipo de evento (AlPulsar o AlSoltar).
         * @param Accion Función anónima (lambda) a ejecutar.
         */
        void RegistrarAccion(int Pin, EventoBoton evento, std::function<void()> Accion);
        /**
         * @brief Configura los pines de los botones como entradas digitales.
         */
        void InicializarCtrl();
        /**
         * @brief Realiza la lectura de los botones.
         * @param Tiempo_Espera Periodo mínimo de actualización de lectura (debounce).
         */
        void ActualizarCtrl(int Tiempo_Espera);
};