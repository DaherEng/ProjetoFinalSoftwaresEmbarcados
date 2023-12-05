#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"

#define GPIO_DHT

MessageBufferHandle_t bufferDisplay;

struct leituraDHT {
    int status;
    int temperature;
    int humidity;
};

void TaskAtualizaDisplay();
void TaskReadGPS();
void TaskReadDHT();
void TaskReadMPU();
void TaskReadCombustivel();
void TaskLeituraRFID();
void TaskFormataDados();

void app_main(void)
{
    bufferDisplay = xMessageBufferCreate(128);

    xTaskCreatePinnedToCore(TaskAtualizaDisplay, "Task Display", 4098, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(TaskReadGPS, "Task GPS", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadDHT, "Task DHT", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadMPU, "Task MPU", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadCombustivel, "Task Combustivel", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskFormataDados, "Task Formata", 4098, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(TaskLeituraRFID, "Task RFID", 4098, NULL, 3, NULL, 1);
}

void TaskReadDHT()
{
    DHT11_init(GPIO_DHT);
    
    while(1)
    {
        leituraDHT = DHT11_read();
    }
}

