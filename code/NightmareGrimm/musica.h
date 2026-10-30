// Músicas nos 5 buzzers passivos, sem bloquear o loop.
// Cada voz da música toca no seu buzzer, com canal LEDC próprio, então as
// notas de buzzers diferentes soam juntas sem uma interromper a outra.
#pragma once
#include <Arduino.h>

// Configura os canais LEDC dos buzzers e deixa tudo em silêncio.
void musicaIniciar();

// Chamar a cada volta do loop.
void musicaAtualizar(uint32_t agora);

// Toca a música da lista (0 = primeira) do começo; para o teste dos buzzers ou
// a música que estiver tocando.
void musicaTocar(uint8_t indice, uint32_t agora);

// Para a música ou o teste e silencia os buzzers.
void musicaParar();

bool musicaTocando();

// Lista de músicas (definida em musica.cpp).
uint8_t musicaSelecionada();  // a que está tocando ou tocou por último
uint8_t musicaQuantidade();
const char* musicaNome(uint8_t indice);

// Teste de montagem: cada buzzer sozinho, depois os 5 juntos em notas diferentes.
void musicaTestarBuzzers(uint32_t agora);
