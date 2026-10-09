// Estado atual do rosto: expressão ativa + olhos abertos/fechados.
//
// Expressão e olhos são independentes: a piscada mexe só nos olhos e a troca
// de expressão mexe só em sobrancelhas e boca. Assim uma piscada nunca altera
// a expressão, e trocar de expressão no meio de uma piscada não a interrompe.
#pragma once
#include "expressoes.h"

enum class Olhos : uint8_t { ABERTOS, FECHADOS };

void rostoIniciar(Expressao inicial);

void rostoDefinirExpressao(Expressao expressao);
Expressao rostoExpressao();

void rostoDefinirOlhos(Olhos olhos);

// Reescreve os LEDs a partir do estado guardado (usado depois do teste de
// montagem, que acende os grupos diretamente).
void rostoReaplicar();
