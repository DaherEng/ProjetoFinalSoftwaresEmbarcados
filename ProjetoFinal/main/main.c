#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "freertos/semaphore.h"
#include <LiquidCrystal.h>
#include <dht11.h>
#include <Wire.h>
#include <math.h>
#include <TinyGPS++.h>

#define GPIO_DHT
#define MPU 0x68 // Endereço I2C do sensor

MessageBufferHandle_t bufferTemperatura, bufferUmidade, bufferLatitude, bufferLongitude, bufferVelocidade, bufferCombustivel;
SemaphoreHandle_t mutexRFID;

void TaskAtualizaDisplay();
void TaskReadGPS();
void TaskReadDHT();
void TaskReadMPU();
void TaskReadCombustivel();
void TaskLeituraRFID();
void TaskFormataDados();

void app_main(void)
{
    bufferTemperatura = xMessageBufferCreate(128);
    bufferUmidade = xMessageBufferCreate(128);
    bufferLatitude = xMessageBufferCreate(128);
    bufferLongitude = xMessageBufferCreate(128);
    bufferVelocidade = xMessageBufferCreate(128);
    bufferCombustivel = xMessageBufferCreate(128);

    mutexRFID = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(TaskFormataDados, "Task Formata", 4098, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(TaskLeituraRFID, "Task RFID", 4098, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(TaskAtualizaDisplay, "Task Display", 4098, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(TaskReadGPS, "Task GPS", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadDHT, "Task DHT", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadMPU, "Task MPU", 4098, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskReadCombustivel, "Task Combustivel", 4098, NULL, 1, NULL, 1);
}

void TaskLeituraRFID()
{
    int usuarioID;
    int usuarioCredenciado;

    while (1)
    {
        xSemaphoreTake(mutexRFID, portMAX_DELAY);
        if (deucerto pegar o semaforo)
        {
            // Lê o RFID
            if (usuarioID==usuarioCredenciado)
            {
                // Libera as outras tasks
                vTaskDelete(NULL);
            }
        }
        
    }
    
}

void TaskReadDHT()
{
    DHT11_init(GPIO_DHT);

    struct dht11_reading
    {
        int status;
        int temperature;
        int humidity;
    };


    while(1)
    {
        DHT11_read();

        xMessageBufferSend(bufferTemperatura, &dht11_reading.temperature, sizeof(int), portMAX_DELAY);
        xMessageBufferSend(bufferUmidade, &dht11_reading.humidity, sizeof(int), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskReadGPS()
{
    //gps init

    while (1)
    {
        // gps read
        xMessageBufferSend(bufferLatitude, &latitude, sizeof(int), portMAX_DELAY);
        xMessageBufferSend(bufferLongitude, &longitude, sizeof(int), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

void TaskReadCombustivel()
{
    // sensor init
    while (1)
    {
        // read combustivel
        // calculo combustivel
        xMessageBufferSend(bufferCombustivel, &combustivel, sizeof(int), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

void TaskReadMPU()
{
    // Variáveis para armazenar os dados coletados
    float AccX;
    float AccY;
    float AccZ;
    float VelX_inicial = 0;
    float VelY_inicial = 0;
    float VelZ_inicial = 0;
    // Velocidade Real
    float velocidade;
    // Inicialização do MPU-6050
    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
    // Configuração do fundo de escala do acelerômetro (2G)
    Wire.beginTransmission(MPU);
    Wire.write(0x1C);
    Wire.write(0x00);
    Wire.endTransmission(true);


    while (1)
    {
        Wire.beginTransmission(MPU);
        Wire.write(0x3B); // Endereço de inicio da solicitação de dados
        Wire.endTransmission(false);
        Wire.requestFrom(MPU, 14, true); // Solicita 14 bytes de dados a partir do primeiro endereço

        // Armazena os dados
        AccX = Wire.read() << 8 | Wire.read();
        AccY = Wire.read() << 8 | Wire.read();
        AccZ = Wire.read() << 8 | Wire.read();

        // Integração de Newton
        VelX = VelX_inicial + (AccX / (16384 * 0.1));
        VelY = VelY_inicial + (AccY / (16384 * 0.1));
        VelZ = VelZ_inicial + (AccZ / (16384 * 0.1));

        velocidade = sqrt((VelX * VelX) + (VelY * VelY) + (VelZ * VelZ));
        
        VelX_inicial = VelX;
        VelY_inicial = VelY;
        VelZ_inicial = VelZ;

        
        xMessageBufferSend(bufferVelocidade, &velocidade, sizeof(float), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

void TaskAtualizaDisplay()
{
    int buffer1[16], buffer2[16];
    int temperatura, umidade, latitude, longitude, velocidade, combustivel;

    lcd.begin(16, 2);
    lcd.clear();
    
    while (1)
    {
        xMessageBufferReceive(bufferTemperatura, &temperatura, sizeof(int), portMAX_DELAY);
        xMessageBufferReceive(bufferUmidade, &umidade, sizeof(int), portMAX_DELAY);
        xMessageBufferReceive(bufferLatitude, &latitude, sizeof(int), portMAX_DELAY);
        xMessageBufferReceive(bufferLongitude, &longitude, sizeof(int), portMAX_DELAY);
        xMessageBufferReceive(bufferVelocidade, &velocidade, sizeof(float), portMAX_DELAY);
        xMessageBufferReceive(bufferCombustivel, &combustivel, sizeof(int), portMAX_DELAY);

        sprintf(buffer1, "T:%d H:%d V:%d C:%d", temperatura, umidade, velocidade, combustivel);
        sprintf(buffer2, "Lat:%d Lon:%d", latitude, longitude);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(buffer1);
        lcd.setCursor(0, 1);
        lcd.print(buffer2);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}