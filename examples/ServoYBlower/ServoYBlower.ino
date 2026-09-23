#include <Botones.hpp>                                             
#include <ServoManager.h>                                          
#include <BlowerControl.h>                                         
#include <Config.h>                                                
                                                                    
const unsigned long guillotineOpenDuration = GUILLOTINE_OPEN_TIME_MS;                                             
const unsigned long blowerDuration = BLOWER_DURATION_MS;           
                                                                    
ServoManager servoManager;                                         
BlowerControl blower;                                              
Controlador Ctrl;                                                  
                                                                    
unsigned long guillotineOpenedAt = 0;                              
unsigned long guillotineClosedAt = 0;                              
unsigned long blowerActivatedAt = 0;                               
bool teclado = true;                                               
uint8_t state = 0;                                                 
                                                                    
void activateShow() {                                              
    if (state != 0) return;                                        
    state = 1;                                                     
}                                                                  
                                                                    
void readKeyboard() {                                              
    if (!teclado) {                                                  
        return;                                                        
    }                                                                
                                                                    
    while (Serial.available() > 0) {                                 
        char command = Serial.read();                                  
        if (command == 'r' || command == 'R') {                        
            Serial.println("Tecla R recibida");                          
            activateShow();                                              
        }                                                              
    }                                                                
}                                                                  
                                                                    
void setup() {                                                     
    Serial.begin(115200);                                            
                                                                    
    Serial.println("Posición inicial");                              
    servoManager.begin();                                            
    blower.begin();                                                  
                                                                    
    Ctrl.RegistrarAccion(PIN_BUTTON, EventoBoton::Pulsar, activateShow);                                                       
    Ctrl.InicializarCtrl();                                          
                                                                    
    Serial.println("--- Prueba de Guillotina y Blower lista ---");   
    Serial.println("Presiona el botón o envía 'R' por Serial para iniciar la secuencia.");                                             
}                                                                  
                                                                    
void loop() {                                                      
    Ctrl.ActualizarCtrl(DEBOUNCE_DELAY_MS);                          
    readKeyboard();                                                  
    actualizarFSM();                                                 
}                                                                  
                                                                    
void actualizarFSM() {                                             
    switch (state) {                                               
        case 0:                                                    
            // En reposo (IDLE)                                    
            break;                                                 
                                                                    
        case 1:                                                    
            if (servoManager.openGuillotine()) {                   
                Serial.println("Abriendo guillotina...");          
                guillotineOpenedAt = millis();                     
                state = 2;                                         
            }                                                      
            break;                                                 
                                                                    
        case 2:                                                    
            // Mantener guillotina abierta el tiempo configurado   
            if (millis() - guillotineOpenedAt >= guillotineOpenDuration) {                                            
                if (servoManager.closeGuillotine()) {              
                    Serial.println("Cerrando guillotina...");      
                    guillotineClosedAt = millis();                 
                    state = 3;                                     
                }                                                  
            }                                                      
            break;                                                 
                                                                    
        case 3:                                                    
            // Espera antes de arrancar el blower                  
            if (millis() - guillotineClosedAt >= BLOWER_START_DELAY_MS) {                                             
                Serial.println("Encendiendo ventiladores/blower...");                                                                  
                blower.turnOn();                                   
                blowerActivatedAt = millis();                      
                state = 4;                                         
            }                                                      
            break;                                                 
                                                                    
        case 4:                                                    
            // Mantener blower encendido el tiempo configurado     
            if (millis() - blowerActivatedAt >= blowerDuration) {  
                blower.turnOff();                                  
                Serial.println("Apagando ventiladores/blower. Secuencia finalizada.");                                             
                state = 0;                                         
            }                                                      
            break;                                                 
    }                                                              
}                                                                  