// Escolha da expressão: comandos pela serial, modo demonstração e teste de
// montagem. Envie "?" no Monitor Serial (115200) para ver a lista.
#pragma once
#include <Arduino.h>
#include "expressoes.h"

void comandosIniciar(uint32_t agora);

// Chamar a cada volta do loop.
void comandosAtualizar(uint32_t agora);

// Durante o teste de montagem os grupos são acesos diretamente, então a
// piscada fica pausada.
bool comandosTesteAtivo();

// Usados pelo controle do celular (controle_wifi.cpp). Escolher uma expressão
// desliga a demonstração, igual aos comandos da serial.
void comandosEscolherExpressao(Expressao expressao);
void comandosDefinirDemo(bool ligada);
bool comandosDemoAtiva();
