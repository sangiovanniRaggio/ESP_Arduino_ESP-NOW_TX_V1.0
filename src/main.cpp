#include <Arduino.h>

#if defined(ESP8266)
	#pragma message "ESP8266"
	#include <ESP8266WiFi.h>
	#include <espnow.h>
#elif defined(ESP32)
	#pragma message "ESP32"
	#include "WiFi.h"
	#include <esp_now.h>
#else
	#error "Plataforma incorrecta"
#endif

// MAC ADDRESS DANTE ESP32:         {0x3C, 0x71, 0xBF, 0xFE, 0x54, 0xF4}
// MAC ADDRESS DANTE ESP8266:       {0xBC, 0xDD, 0xC2, 0xFE, 0x2E, 0xC0}
uint8_t broadcastAddress[] = {0x3C, 0x71, 0xBF, 0xFE, 0x54, 0xF4};

//Esctructura de datos
typedef struct struct_message 
{
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

//Variable de tipo estructura 
struct_message myData;

// callback when data is sent
#if defined(ESP8266)
void OnDataSent(uint8_t *mac_addr, uint8_t status) 
{
    Serial.print("\r\nEstado del ultimo paquete enviado:\t");
    Serial.println(status == 0 ? "Entregado OK" : "Fallo al entregar");
}
#elif defined(ESP32)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    Serial.print("\r\nEstado del ultimo paquete enviado:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Entregado satisfactoriamente" : "Fallo en la entrega");
}
#endif

void setup()
{
    Serial.begin(115200);
    Serial.println("\n\n\nInicializando..");

    WiFi.mode(WIFI_STA);
    delay(2000);

    Serial.println("MAC ADDRESS: "+String(WiFi.macAddress()));

    #if defined(ESP8266)
        if (esp_now_init() != 0) {
            Serial.println("Error initializing ESP-NOW");
            return;
        }
        esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
        esp_now_register_send_cb(OnDataSent);

        esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

    #elif defined(ESP32)
        if (esp_now_init() != ESP_OK) {
            Serial.println("Error initializing ESP-NOW");
            return;
        }

        esp_now_register_send_cb(OnDataSent);

        esp_now_peer_info_t peerInfo;
        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
     
        if (esp_now_add_peer(&peerInfo) != ESP_OK){
            Serial.println("Failed to add peer");
            return;
        }
    #endif

    delay(1000);
	Serial.println("Comenzando el envio..");
}

void loop() 
{
    strcpy(myData.a, "Un poco de texto");
    myData.b = random(1,20);
    myData.c = 1.2;
    myData.d = !myData.d;

    #if defined(ESP8266)
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    #elif defined(ESP32)
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        if (result == ESP_OK) {
            Serial.println("Enviado con exito");
        }
        else {
            Serial.println("Error enviando datos");
        }
    #endif

    Serial.println();
	Serial.print("Bytes enviados: ");
	Serial.println(sizeof(myData));
	Serial.print("Char: ");
	Serial.println(myData.a);
	Serial.print("Int: ");
	Serial.println(myData.b);
	Serial.print("Float: ");
	Serial.println(myData.c);
	Serial.print("Bool: ");
	Serial.println(myData.d);

    delay(2000);
}