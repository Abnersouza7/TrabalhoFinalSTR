/* Kernel includes. */
#include "Arduino_FreeRTOS.h"
#include "Arduino.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Pinos de acesso para interrupcao do arduino
 *
 */

#define PIN_T1 3
#define PIN_T2 4
#define PIN_T3 5
#define PIN_T4 6
#define PIN_INT 2

/**
 * @brief Variaveis do sistema
 *
 */
unsigned long ulTaskNumber[5];
uint16_t vCTimeInterrupt = 0;
Tasks tarefa[3];
uint8_t n = sizeof(tarefa) / sizeof(tarefa[0]);

/**
 * @brief Prototipos das funcoes de tarefas
 *
 * @param pvParameters
 */
static void vT1(void *pvParameters);
static void vT2(void *pvParameters);
static void vAT(void *pvParameters);

/**
 * @brief Funcao para interrupcao com o botao
 *
 */
void ISR_aperiodic();
/**
 * @brief Funcao que faz sorteamento das tasks de acordo com o RM
 *
 * @param n
 */
void Bubblesort(int n);

/**
 * @brief Funcao para debugging no terminal do serial
 *
 * @param i
 */
void enviaInfo(int i);

/**
 * @brief Variaveis de manipulacao de tarefas
 *
 */
SemaphoreHandle_t xSemaforo;
TaskHandle_t xT1, xT2, xAT;
QueueHandle_t xAperTaskPendent; // pooling server
uint16_t x = 2;

void enviaInfo(int i)
{
  Serial.println("Enviando informacao da tarefa: " + String(i));
}

/*********************************INICIO DAS TAREFAS*********************************/
static void vT1(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  TickType_t periodo = (TickType_t)pvParameters;
  while (1)
  {
    xSemaphoreTake(xSemaforo, pdMS_TO_TICKS(10));
    enviaInfo(1);
    for (uint8_t i = 0; i < 3 * x; i++)
    {
      digitalWrite(PIN_T1, !digitalRead(PIN_T1));
      delayMicroseconds(30000);
    }
    for (uint8_t i = 0; i < 3 * x; i++)
    {
      digitalWrite(PIN_T3, !digitalRead(PIN_T3));
      delayMicroseconds(30000);
    }
    xTaskDelayUntil(&xLastWakeTime,
                    pdMS_TO_TICKS(periodo));
    xSemaphoreGive(xSemaforo);
    // Delay para reduzir erro por troca de contexto
    vTaskDelay(10);
  }
}

static void vT2(void *pvParameters)
{
  TickType_t periodo = (TickType_t)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    xSemaphoreTake(xSemaforo, pdMS_TO_TICKS(15));
    enviaInfo(2);
    for (uint8_t i = 0; i < 3 * x; i++)
    {
      digitalWrite(PIN_T2, !digitalRead(PIN_T2));
      delayMicroseconds(25000);
    }
    for (uint8_t i = 0; i < 3 * x; i++)
    {
      digitalWrite(PIN_T1, !digitalRead(PIN_T1));
      delayMicroseconds(25000);
    }
    xTaskDelayUntil(&xLastWakeTime,
                    pdMS_TO_TICKS(periodo));
    xSemaphoreGive(xSemaforo);
    // Delay para reduzir erro por troca de contexto
    vTaskDelay(10);
  }
}

static void vAT(void *pvParameters)
{
  uint16_t CTime = 0;
  TickType_t periodo = (TickType_t)pvParameters;

  {
    BaseType_t inicia_aperiodica;
    while (1)
    {
      TickType_t xLastWakeTime = xTaskGetTickCount();
      inicia_aperiodica = xQueueReceiveFromISR(xAperTaskPendent,
                                               (uint16_t *)CTime,
                                               &inicia_aperiodica);
      xSemaphoreTake(xSemaforo, pdMS_TO_TICKS(10));
      if (inicia_aperiodica == pdTRUE)
      {
        enviaInfo(4);

        for (uint16_t j = 50; j > 0; j--)
        {
          digitalWrite(PIN_T1, LOW);
          digitalWrite(PIN_T2, LOW);
          digitalWrite(PIN_T3, LOW);
          digitalWrite(PIN_T4, !digitalRead(PIN_T4));

          delayMicroseconds(10000);
        }
        digitalWrite(PIN_T4, LOW);
        xQueueReceive(xAperTaskPendent, (uint16_t *)CTime, 1);
      }
      vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(periodo));
      xSemaphoreGive(xSemaforo);
      // Delay para reduzir erro por troca de contexto
      vTaskDelay(10);
    }
  }
}
/*********************************FIM DAS TAREFAS*********************************/
void Bubblesort(int n)
{
  Tasks tarefaux;
  for (uint8_t i = 0; i < n; i++)
  {
    for (uint8_t j = i + 1; j < n; j++)
    {
      if (tarefa[i].periodo > tarefa[j].periodo)
      {
        tarefaux = tarefa[i];
        tarefa[i] = tarefa[j];
        tarefa[j] = tarefaux;
        for (uint8_t k = 0; k < n; k++)
          Serial.println(tarefa[k].name);
      }
    }
  }
}

void ISR_aperiodic()
{
  xQueueSend(xAperTaskPendent, &vCTimeInterrupt, 2);
}

/**
 * @brief funcao de configuracao e inicializacao das tarefas
 *
 */
void setup()
{
  // Inicializando sistema
  Serial.begin(115200);
  Serial.println("Inicializing...");

  pinMode(PIN_T1, OUTPUT);
  pinMode(PIN_T2, OUTPUT);
  pinMode(PIN_T3, OUTPUT);
  pinMode(PIN_T4, OUTPUT);
  pinMode(PIN_INT, INPUT);

  digitalWrite(PIN_T1, LOW);
  digitalWrite(PIN_T2, LOW);
  digitalWrite(PIN_T3, LOW);
  digitalWrite(PIN_T4, LOW);

  // Definicao de prioridades e sorteamento de acordo com as prioridades
  tarefa[0].funcao = vT1;
  tarefa[0].periodo = 33;
  strcpy(tarefa[0].name, "T1");
  tarefa[0].handler = xT1;
  tarefa[0].priority = 0;
  tarefa[1].funcao = vT2;
  tarefa[1].periodo = 7;
  strcpy(tarefa[1].name, "T2");
  tarefa[1].handler = xT2;
  tarefa[1].priority = 0;
  tarefa[2].funcao = vAT;
  tarefa[2].periodo = 17;
  strcpy(tarefa[2].name, "AT");
  tarefa[2].handler = xAT;
  tarefa[2].priority = 0;
  Bubblesort(n);

  xSemaforo = xSemaphoreCreateMutex();
  attachInterrupt(digitalPinToInterrupt(PIN_INT),
                  ISR_aperiodic,
                  FALLING);
  delayMicroseconds(5000);

  for (uint8_t i; i < n; i++)
  {
    xTaskCreate(tarefa[i].funcao,
                tarefa[i].name,
                configMINIMAL_STACK_SIZE,
                (TickType_t *)tarefa[i].periodo,
                tarefa[i].priority,
                &tarefa[i].handler);
  }

  xAperTaskPendent = xQueueCreate(10, 2); 
  vTaskStartScheduler();
}
