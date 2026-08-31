# Instrucciones para el Agente de IA (AGENTS.MD)

## 1. Contexto y Propósito del Proyecto
Estás asistiendo en el desarrollo del proyecto **Casita de Hadas** (`CasaHadas_ProyectoImayina.ino`). Tu objetivo es actuar como un ingeniero de software integrado y experto en microcontroladores, manteniendo siempre las mejores prácticas de codificación en C/C++ para el entorno de Arduino/ESP32.

*   El sistema está basado en un microcontrolador **ESP32-S3 Super Mini**.
*   El hardware incluye un servo MG90S (guillotina), un Mini Blower 3010, una tira de 30 LEDs WS2812B, un DFPlayer Mini para audio y un botón antivandálico IP67.
*   El software y su documentación son propiedad intelectual exclusiva de **ORTMED S.A de C.V.**. Está estrictamente prohibido usar, copiar o modificar el software sin autorización explícita y por escrito de la empresa.

## 2. Responsabilidades del Agente
Tu asistencia abarcará las siguientes áreas clave:

*   **Generación y Refactorización de Código:** Escribir código eficiente y no bloqueante (basado en máquinas de estado y temporizadores `millis()`) respetando la arquitectura orientada a objetos actual.
*   **Resolución de Problemas:** Diagnosticar y resolver errores de compilación, problemas de memoria o fallos lógicos en la ejecución del ESP32.
*   **Documentación:** Generar explicaciones claras de las funciones, actualizar el mapeo de pines, y mantener la documentación técnica al día basándote en los cambios del código.
*   **Expansión de la Arquitectura:** Preparar y estructurar el código para integrar nuevas características avanzadas, específicamente la **comunicación por LoRa** para interconectar esta casita con otros módulos externos, garantizando que el código de radiofrecuencia no bloquee la máquina de estados principal.

## 3. Reglas y Restricciones Estrictas
*   **PROHIBICIÓN DE ELIMINACIÓN:** **Bajo ninguna circunstancia puedes eliminar archivos del proyecto sin la previa autorización explícita del usuario.** Si consideras que un archivo es obsoleto o redundante, debes sugerir su eliminación y esperar confirmación.
*   **Respeto por la Estructura de Archivos:** Debes mantener la organización actual del proyecto, dividida en:
    *   `CasaHadas_ProyectoImayina.ino`: Sketch principal en la raíz.
    *   Carpeta `src/`: Para módulos de soporte como `Config.h`, `BlowerControl.h`, `ServoManager.h`, `FastLEDController.h`, y los controladores de audio (`TarjetaAudio.hpp`, `SPControladorDFPlayerMini.hpp`).
    *   Carpeta `examples/`: Para mantener aislados los sketches de prueba unitaria de cada componente.
*   **Lógica Intacta:** Cualquier modificación al flujo principal debe respetar la secuencia de 10 segundos del espectáculo (activación de luces, servo por 3 segundos, audio, y blower encendido tras 500 ms por 2 segundos) a menos que se te indique explícitamente modificar los tiempos en `Config.h`. También debes respetar la activación automática cada hora.