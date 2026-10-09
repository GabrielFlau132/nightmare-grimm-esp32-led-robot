// Escolha da expressão: comandos pela serial, modo demonstração e teste de
// montagem. Envie "?" no Monitor Serial (115200) para ver a lista.
#pragma once
#include <Arduino.h>

void comandosIniciar(uint32_t agora);

// Chamar a cada volta do loop.
void comandosAtualizar(uint32_t agora);

// Durante o teste de montagem os grupos são acesos diretamente, então a
// piscada fica pausada.
bool comandosTesteAtivo();
