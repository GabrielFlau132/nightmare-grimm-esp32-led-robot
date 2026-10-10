// Grupos de LEDs: cada grupo acende e apaga junto. Os pinos de cada grupo
// ficam em config.h (os grupos grandes usam 2 pinos).
//
// Quantidades e posições finais: docs/gerar_layout.py (mapa em docs/mapa-leds.svg).
#pragma once
#include <Arduino.h>

enum class GrupoLed : uint8_t {
  OLHOS_ANEL,            // 16 amarelos: contorno dos olhos, menos as laterais do meio
  OLHOS_MEIO_LATERAL,    //  4 amarelos: laterais do meio, acesos com olho aberto e fechado
  OLHOS_MEIO_CENTRO,     //  4 amarelos: centro da barra do meio, só na piscada
  SOBRANCELHAS_BRAVAS,   //  8 vermelhos: descem em direção ao centro
  SOBRANCELHAS_TRISTES,  //  6 azuis:     sobem em direção ao centro
  BOCA_RETA,             //  4 vermelhos: linha central da boca
  BOCA_SORRISO,          //  6 vermelhos: cantos que sobem no sorriso
  BOCA_TRISTE,           // 10 azuis:     linha central + cantos que descem
  QUANTIDADE             // não é um grupo: marca o total
};

// Conjunto de grupos acesos: um bit por grupo.
using MascaraLeds = uint16_t;

constexpr MascaraLeds mascara(GrupoLed grupo) {
  return MascaraLeds(1u << uint8_t(grupo));
}

// Configura os pinos e deixa tudo apagado.
void gruposIniciar();

// Deixa acesos exatamente os grupos da máscara. Só escreve nos pinos que
// mudaram, então o resto do rosto não é tocado.
void gruposAplicar(MascaraLeds acesos);

const char* grupoNome(GrupoLed grupo);
