// Configuração do Nightmare Grimm: pinos e tempos.
// É aqui que se ajusta o comportamento sem mexer na lógica.
#pragma once
#include <Arduino.h>
#include "expressoes.h"

// ---------------------------------------------------------------------------
// Pinos do ESP32 (ver grupos_led.h)
// ---------------------------------------------------------------------------
// Os LEDs são ligados direto nos pinos, sem driver: pino -> resistor -> anodo,
// catodo no GND (comum a todos). Cada pino fica com no máximo 8 LEDs (~16 mA),
// por isso os grupos grandes usam 2 pinos: o primeiro liga a metade esquerda
// do rosto (vista de frente) e o segundo, a metade direita.
//
// Escolhidos entre as saídas seguras do ESP32 DevKit: ficam de fora os pinos
// de boot (0, 2, 5, 12, 15), os da memória flash (6–11), os que só funcionam
// como entrada (34–39), a serial USB (1, 3) e o 14 (oscila no boot).
// 21 e 22 ficam livres para I2C.
constexpr uint8_t SEM_PINO = 0xFF;
constexpr uint8_t MAX_PINOS_POR_GRUPO = 2;

constexpr uint8_t PINOS_OLHOS_ANEL[MAX_PINOS_POR_GRUPO]           = { 25, 13 };
constexpr uint8_t PINOS_OLHOS_MEIO_LATERAL[MAX_PINOS_POR_GRUPO]   = { 26, SEM_PINO };
constexpr uint8_t PINOS_OLHOS_MEIO_CENTRO[MAX_PINOS_POR_GRUPO]    = { 27, SEM_PINO };
constexpr uint8_t PINOS_SOBRANCELHAS_BRAVAS[MAX_PINOS_POR_GRUPO]  = { 32, SEM_PINO };
constexpr uint8_t PINOS_SOBRANCELHAS_TRISTES[MAX_PINOS_POR_GRUPO] = { 33, SEM_PINO };
constexpr uint8_t PINOS_BOCA_RETA[MAX_PINOS_POR_GRUPO]            = { 23, SEM_PINO };
constexpr uint8_t PINOS_BOCA_SORRISO[MAX_PINOS_POR_GRUPO]         = { 19, SEM_PINO };
constexpr uint8_t PINOS_BOCA_TRISTE[MAX_PINOS_POR_GRUPO]          = { 18, 4 };

// Nível do pino que ACENDE os LEDs.
//   HIGH: LED entre o pino e o GND (o pino fornece a corrente). É a ligação
//         do README.
//   LOW:  LED entre o 3V3 e o pino (o pino absorve a corrente).
constexpr uint8_t NIVEL_LED_ACESO = HIGH;

// ---------------------------------------------------------------------------
// Expressões
// ---------------------------------------------------------------------------
constexpr Expressao EXPRESSAO_INICIAL = Expressao::NEUTRO;

// Modo demonstração: troca de expressão sozinho, em ordem, até receber um
// comando de expressão pela serial.
constexpr bool     DEMO_AO_LIGAR     = true;
constexpr uint32_t DEMO_INTERVALO_MS = 5000;

// ---------------------------------------------------------------------------
// Piscada
// ---------------------------------------------------------------------------
// O tempo até a próxima piscada é sorteado entre o mínimo e o máximo.
constexpr uint32_t PISCADA_INTERVALO_MIN_MS = 2500;
constexpr uint32_t PISCADA_INTERVALO_MAX_MS = 6000;
constexpr uint32_t PISCADA_FECHADO_MS       = 150;  // olhos fechados em cada piscada

// Piscada dupla: chance (0–100 %) de piscar de novo logo em seguida.
constexpr uint8_t  PISCADA_DUPLA_CHANCE     = 20;
constexpr uint32_t PISCADA_DUPLA_PAUSA_MS   = 120;  // olhos abertos entre as duas

static_assert(PISCADA_INTERVALO_MIN_MS <= PISCADA_INTERVALO_MAX_MS,
              "PISCADA_INTERVALO_MIN_MS deve ser <= PISCADA_INTERVALO_MAX_MS");
static_assert(PISCADA_DUPLA_CHANCE <= 100, "PISCADA_DUPLA_CHANCE é uma porcentagem");

// ---------------------------------------------------------------------------
// Serial e teste de montagem
// ---------------------------------------------------------------------------
constexpr uint32_t SERIAL_VELOCIDADE = 115200;
constexpr uint32_t TESTE_GRUPO_MS    = 1500;  // tempo aceso de cada grupo no teste
