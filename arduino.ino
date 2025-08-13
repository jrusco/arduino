#include <Arduino.h>


// Asignación de pines para motores y sensores
const int ACTIVATE_MOTOR_RIGHT = 12;
const int MOTOR_RIGHT_FORWARD = 9;
const int MOTOR_RIGHT_BACKWARD = 8;
const int ACTIVATE_MOTOR_LEFT = 13;
const int MOTOR_LEFT_FORWARD = 10;
const int MOTOR_LEFT_BACKWARD = 11;

const int TRIG_FRONT = 5;
const int ECHO_FRONT = 4;
const int TRIG_LEFT = 3;
const int ECHO_LEFT = 2;
const int TRIG_RIGHT = 7;
const int ECHO_RIGHT = 6;
const int DIST_CM = 6;
const int DELAY_MS = 200;

// Variables para comportamiento de curiosidad
int pasosAvanzados = 0;           // Contador de pasos avanzados
const int CURIOSIDAD = 5;         // Revisar lados cada 5 pasos
const int APERTURA_MINIMA = 15;   // Tamaño mínimo de apertura para explorar

// Variables para guardar las distancias (en centímetros)
int dist_front = 0;
int dist_left = 0;
int dist_right = 0;

// Lee la distancia de un sensor ultrasónico (en centímetros)
long readUltrasonicDistance(int triggerPin, int echoPin)
{
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    long duration = pulseIn(echoPin, HIGH);
    // Convierte el tiempo en distancia (cm): velocidad del sonido = 343 m/s
    return 0.01723 * duration;
}

// Funciones para controlar los motores
void avanzar()
{
    // Hace que el robot avance
    digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
    digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
}

void parar()
{
    // Detiene todos los motores
    digitalWrite(ACTIVATE_MOTOR_RIGHT, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
    digitalWrite(ACTIVATE_MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
}

void girarDerecha()
{
    // Gira a la derecha: la rueda izquierda va para adelante y la derecha para atras
    digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, HIGH);
    digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void girarIzquierda()
{
    // Gira a la derecha: la rueda derecha va para atrás y la izquierda para adelante
    digitalWrite(ACTIVATE_MOTOR_LEFT, HIGH);
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
    digitalWrite(ACTIVATE_MOTOR_RIGHT, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, HIGH);
}

String decidirDireccion(float dist_left, float dist_right) {

    bool izquierdaLibre = dist_left > DIST_CM && dist_left < DIST_CM * 150;
    bool derechaLibre  = dist_right > DIST_CM && dist_right < DIST_CM * 150;

    if (!izquierdaLibre && !derechaLibre) {
        return "BLOQUEADO";
    }

    return (dist_left > dist_right) ? "IZQUIERDA" : "DERECHA";
}

// Nueva función: explorar lados buscando aperturas
String explorarLados(float dist_left, float dist_right) {
    // Busca aberturas lo suficientemente grandes para explorar
    bool aperturaIzquierda = dist_left > APERTURA_MINIMA && dist_left < DIST_CM * 150;
    bool aperturaDerecha = dist_right > APERTURA_MINIMA && dist_right < DIST_CM * 150;
    
    if (!aperturaIzquierda && !aperturaDerecha) {
        return "CONTINUAR";  // No hay aperturas interesantes
    }
    
    // Si hay aperturas, elegir la más grande
    if (aperturaIzquierda && aperturaDerecha) {
        return (dist_left > dist_right) ? "IZQUIERDA" : "DERECHA";
    } else if (aperturaIzquierda) {
        return "IZQUIERDA";
    } else {
        return "DERECHA";
    }
}

void setup()
{
    // Configura los pines de los motores como salida
    pinMode(ACTIVATE_MOTOR_RIGHT, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);
    pinMode(ACTIVATE_MOTOR_LEFT, OUTPUT);
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);

    Serial.begin(9600);
}

void loop()
{
    // Mide las distancias con los sensores ultrasónicos
    dist_front = readUltrasonicDistance(TRIG_FRONT, ECHO_FRONT);
    dist_left = readUltrasonicDistance(TRIG_LEFT, ECHO_LEFT);
    dist_right = readUltrasonicDistance(TRIG_RIGHT, ECHO_RIGHT);

    // Muestra las distancias por el monitor serie al usuario
    Serial.print("Frente: ");
    Serial.print(dist_front);
    Serial.print(" cm, Izquierda: ");
    Serial.print(dist_left);
    Serial.print(" cm, Derecha: ");
    Serial.println(dist_right);

    // Lógica mejorada: evitar obstáculos + comportamiento de curiosidad
    if (dist_front > DIST_CM * 2 && dist_front < DIST_CM * 150 )
    {
        // El camino adelante está libre
        pasosAvanzados++; // Incrementar contador de pasos
        
        // Revisar si es hora de ser curioso
        if (pasosAvanzados >= CURIOSIDAD) {
            Serial.println("🤔 ¡Momento de curiosidad! Revisando lados...");
            parar();
            delay(DELAY_MS); // Pausa para medir bien
            
            String exploracion = explorarLados(dist_left, dist_right);
            
            if (exploracion != "CONTINUAR") {
                Serial.print("¡Apertura encontrada hacia: ");
                Serial.println(exploracion);
                
                // Girar hacia la apertura encontrada
                if (exploracion == "DERECHA") {
                    girarDerecha();
                    delay(DELAY_MS * 2);
                    parar();
                } else if (exploracion == "IZQUIERDA") {
                    girarIzquierda();
                    delay(DELAY_MS * 2);
                    parar();
                }
                pasosAvanzados = 0; // Reiniciar contador
            } else {
                Serial.println("No hay aperturas interesantes, siguiendo adelante");
                pasosAvanzados = 0; // Reiniciar contador de curiosidad
                avanzar();
            }
        } else {
            // Avanzar normalmente
            avanzar();
        }
    }
    else
    {
        // Si hay un obstáculo adelante, usar lógica original
        parar();
        delay(DELAY_MS);
        pasosAvanzados = 0; // Reiniciar contador cuando hay obstáculo

        String direccion = decidirDireccion(dist_left, dist_right);

        if (direccion == "DERECHA")
        {
            Serial.println("Obstáculo detectado - girando a la derecha");
            girarDerecha();
            delay(DELAY_MS * 2);
            parar();
        }
        else if (direccion == "IZQUIERDA")
        {
            Serial.println("Obstáculo detectado - girando a la izquierda");
            girarIzquierda();
            delay(DELAY_MS * 2);
            parar();
        }
        else
        {
            Serial.println("Todos los caminos bloqueados, girando hasta encontrar salida...");
            while (true)
            {
                girarDerecha();
                delay(DELAY_MS * 4);
                parar();
                delay(DELAY_MS / 5);

                dist_front = readUltrasonicDistance(TRIG_FRONT, ECHO_FRONT);

                if (dist_front > DIST_CM * 2)
                {
                    Serial.println("¡Camino libre encontrado!");
                    break;
                }
            }
        }
    }

    delay(DELAY_MS / 2); // Pequeña pausa antes de repetir el ciclo
}
