#include "rosto.h"
#include "grupos_led.h"

namespace {

Expressao expressaoAtual = Expressao::NEUTRO;
Olhos olhosAtuais = Olhos::ABERTOS;

// Tradução de cada parte do rosto para grupos de LEDs, conforme os modelos.

MascaraLeds mascaraOlhos(Olhos olhos) {
  switch (olhos) {
    case Olhos::ABERTOS:   // contorno completo (Feliz.png, Neutro.png...)
      return mascara(GrupoLed::OLHOS_ANEL) | mascara(GrupoLed::OLHOS_MEIO_LATERAL);
    case Olhos::FECHADOS:  // só a barra do meio (piscando.png)
      return mascara(GrupoLed::OLHOS_MEIO_LATERAL) | mascara(GrupoLed::OLHOS_MEIO_CENTRO);
  }
  return 0;
}

MascaraLeds mascaraSobrancelhas(Sobrancelhas sobrancelhas) {
  switch (sobrancelhas) {
    case Sobrancelhas::NENHUMA: return 0;
    case Sobrancelhas::BRAVAS:  return mascara(GrupoLed::SOBRANCELHAS_BRAVAS);
    case Sobrancelhas::TRISTES: return mascara(GrupoLed::SOBRANCELHAS_TRISTES);
  }
  return 0;
}

MascaraLeds mascaraBoca(Boca boca) {
  switch (boca) {
    case Boca::RETA:    return mascara(GrupoLed::BOCA_RETA);
    case Boca::SORRISO: return mascara(GrupoLed::BOCA_RETA) | mascara(GrupoLed::BOCA_SORRISO);
    case Boca::TRISTE:  return mascara(GrupoLed::BOCA_TRISTE);
  }
  return 0;
}

void aplicar() {
  const DefinicaoExpressao& definicao = expressaoDefinicao(expressaoAtual);
  gruposAplicar(mascaraOlhos(olhosAtuais) |
                mascaraSobrancelhas(definicao.sobrancelhas) |
                mascaraBoca(definicao.boca));
}

}  // namespace

void rostoIniciar(Expressao inicial) {
  expressaoAtual = inicial;
  olhosAtuais = Olhos::ABERTOS;
  aplicar();
}

void rostoDefinirExpressao(Expressao expressao) {
  expressaoAtual = expressao;
  aplicar();
}

Expressao rostoExpressao() {
  return expressaoAtual;
}

void rostoDefinirOlhos(Olhos olhos) {
  olhosAtuais = olhos;
  aplicar();
}

void rostoReaplicar() {
  aplicar();
}
