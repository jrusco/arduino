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

    // Lógica para evitar obstáculos
    if (dist_front > DIST_CM * 2 && dist_front < DIST_CM * 150 )
    {
        // Si el camino de adelante está libre, avanza
        avanzar();
    }
    else
    {
        // Si hay un obstáculo adelante, revisa los costados
        parar();
        delay(DELAY_MS); // Hace una pausa corta antes de girar

        String direccion = decidirDireccion(dist_left, dist_right);

        if (direccion == "DERECHA")
        {
            // Si la izquierda está libre, gira a la izquierda
            girarDerecha();
            delay(DELAY_MS * 2); // Ajustá este valor para cambiar el ángulo de giro
            parar();
        }
        else if (direccion == "IZQUIERDA")
        {
            // Si la derecha está libre, gira a la derecha
            girarIzquierda();
            delay(DELAY_MS * 2); // Ajustá este valor para cambiar el ángulo de giro
            parar();
        }
        else
        {
            // Si todos los lados están bloqueados, gira sobre sí mismo hasta encontrar salida
            Serial.println("Todos los caminos bloqueados, girando hasta encontrar salida...");
            while (true)
            {
                girarDerecha(); // Gira sobre sí mismo hacia la izquierda
                delay(DELAY_MS * 4); // Pequeña pausa para girar un poco
                parar();
                delay(DELAY_MS / 5); // Pausa para medir de nuevo

                // Vuelve a medir la distancia al frente
                dist_front = readUltrasonicDistance(TRIG_FRONT, ECHO_FRONT);

                // Si encuentra camino libre al frente, sale del bucle y avanza
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
