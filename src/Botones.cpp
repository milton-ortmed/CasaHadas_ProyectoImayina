/*
 * Proyecto: Controlador de Hardware Modular
 * Empresa: Ortmed S.A. de C.V.
 * Autor: Milton Elias Pech Uc (Ing. Floppa)
 * Descripción: Manejador de eventos de entrada digital con debounce.
 */

#include <functional>
#include <vector>
#include <map>

#include "Botones.hpp"

// Botones

Boton::Boton(int Pin) : Pin(Pin) {}

void Boton::Inicializar(){
    pinMode(Pin, INPUT_PULLUP);
    Tiempo_Anterior = millis();
    Estado = 1;
}

void Boton::DefinirEstado(int Tiempo_Espera){

    unsigned long Tiempo_Actual = millis();
    if (Tiempo_Actual - Tiempo_Anterior > Tiempo_Espera){
        if(Lectura == 1){
            Estado = 1; //Liberado
            //Serial.printf("Botón en estado %d, liberado", Estado);
        } else {
            Estado = 0; //Apretado
            //Serial.printf("Botón en estado %d, apretado", Estado);
        }
        //Serial.println();

        Tiempo_Anterior = Tiempo_Actual;
    }
}

bool Boton::Sensado(int Tiempo_Espera){
    Lectura = digitalRead(Pin);
    DefinirEstado(Tiempo_Espera);
    return Lectura;
}

bool Boton::GetEstado(){
    return Estado;
}

// Funciones

/*
// Función auxiliar para traducir el enum a texto (con ayuda de IA)
const char* AccionToString(AccionCtrl control) { 
    switch (control) {
        case AccionCtrl::Motor:      return "Motor";
        case AccionCtrl::Led:        return "Led";
        case AccionCtrl::Calentador: return "Calentador";
        default:                     return "Desconocido";
    }
}

void AccionPulsar(int Pin, AccionCtrl control){
    Serial.print(AccionToString(control));
    Serial.print(" activado en el pin ");
    Serial.println(Pin);
}

void AccionSoltar(int Pin, AccionCtrl control){
    Serial.print(AccionToString(control));
    Serial.print(" desactivado en el pin ");
    Serial.println(Pin);
}

void AccionEvento(int Pin, std::function<void(int)> Accion){
    Accion(Pin);
}
*/

// Controlador

Controlador::Controlador() {}

void Controlador::RegistrarAccion(int Pin, EventoBoton evento, std::function<void()> Accion){
    if (MapaBotones.find(Pin) == MapaBotones.end()){
        MapaBotones.emplace(Pin, ContextoBoton(Pin));
    }
    
    if (evento == EventoBoton::Pulsar){
        MapaBotones.at(Pin).accionPulsar = Accion;
    } else if (evento == EventoBoton::Soltar) {
        MapaBotones.at(Pin).accionSoltar = Accion;
    }
}

void Controlador::InicializarCtrl(){
    /*
    if(Pines.size()==0) return;

    Botones.clear();
    EstadoPines.clear();

    for(int i=0; i<Pines.size(); i++){
        Botones.emplace_back(Pines[i]);
        Botones[i].Inicializar();
        EstadoPines.push_back(Botones[i].GetEstado());
    }
    */

    if (MapaBotones.size() == 0) return;

    for(auto it=MapaBotones.begin(); it!=MapaBotones.end(); it++){
        it->second.Btn.Inicializar();
    }
}

void Controlador::ActualizarCtrl(int Tiempo_Espera){
    /*
    if(Pines.size()==0) return;

    std::vector<bool> EstadoPinesAnterior = EstadoPines;
    for(int i=0; i<Pines.size(); i++){
        Botones[i].Sensado(Tiempo_Espera);
        EstadoPines[i] = Botones[i].GetEstado();
        if(EstadoPines[i] == 0 && EstadoPinesAnterior[i] == 1){
            AccionEvento(Pines[i], [this](int Pin){
                AccionPulsar(Pin, this->control);
            });
        } else if(EstadoPines[i] == 1 && EstadoPinesAnterior[i] == 0) {
            AccionEvento(Pines[i], [this](int Pin){
                AccionSoltar(Pin, this->control);
            });
        }
    }
    */

    if (MapaBotones.size() == 0) return;

    for(auto it=MapaBotones.begin(); it!=MapaBotones.end(); it++){
        bool EstadoAnterior = it->second.Btn.GetEstado();
        it->second.Btn.Sensado(Tiempo_Espera);
        bool EstadoActual = it->second.Btn.GetEstado();

        // El botón en reposo da lectura 1
        // Al presionar el botón da lectura 0

        if(EstadoActual == 0 && EstadoAnterior == 1){
            if (it->second.accionPulsar) {
                it->second.accionPulsar();
            }
        } else if(EstadoActual == 1 && EstadoAnterior == 0) {
            if (it->second.accionSoltar) {
                it->second.accionSoltar();
            }
        }
    }
}