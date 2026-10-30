// Formato das notas das músicas dos buzzers.
#pragma once
#include <Arduino.h>

// Uma nota: {início em ms, duração em ms, frequência em Hz, voz 0..4}.
// Em cada música, os eventos ficam em ordem de início e nenhuma voz tem duas
// notas ao mesmo tempo (cada buzzer toca uma frequência por vez).
struct EventoNota {
  uint32_t inicioMs;
  uint16_t duracaoMs;
  uint16_t frequencia;
  uint8_t voz;
};
