// Piscada automática, sem bloquear o loop (usa millis(), nunca delay()).
// Os tempos ficam em config.h.
#pragma once
#include <Arduino.h>

// Abre os olhos e agenda a próxima piscada.
void piscadaIniciar(uint32_t agora);

// Chamar a cada volta do loop.
void piscadaAtualizar(uint32_t agora);

// Pisca agora, se não estiver piscando.
void piscadaAgora(uint32_t agora);
