// Configuração do Nightmare Grimm: pinos e tempos.
// É aqui que se ajusta o comportamento sem mexer na lógica.
#pragma once
#include <Arduino.h>
#include "expressoes.h"

// ---------------------------------------------------------------------------
// Pinos do ESP32 — um pino por grupo de LEDs (ver grupos_led.h)
// ---------------------------------------------------------------------------
// Escolhidos entre as saídas seguras do ESP32 DevKit: ficam de fora os pinos
// de boot (0, 2, 5, 12, 15), os da memória flash (6–11), os que só funcionam
// como entrada (34–39) e a serial USB (1, 3). 21 e 22 ficam livres para I2C.
constexpr uint8_t PINO_OLHOS_ANEL           = 25;
constexpr uint8_t PINO_OLHOS_MEIO_LATERAL   = 26;
constexpr uint8_t PINO_OLHOS_MEIO_CENTRO    = 27;
constexpr uint8_t PINO_SOBRANCELHAS_BRAVAS  = 32;
constexpr uint8_t PINO_SOBRANCELHAS_TRISTES = 33;
constexpr uint8_t PINO_BOCA_RETA            = 23;
constexpr uint8_t PINO_BOCA_SORRISO         = 19;
constexpr uint8_t PINO_BOCA_TRISTE          = 18;

// Nível do pino que ACENDE um grupo.
//   HIGH: LEDs acionados por driver no lado do GND (ULN2803A, NPN ou MOSFET N),
//         que é a ligação recomendada no README.
//   LOW:  LEDs ligados entre 3,3 V e o pino, sem driver (só para testes
//         rápidos com 1 LED por pino).
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
