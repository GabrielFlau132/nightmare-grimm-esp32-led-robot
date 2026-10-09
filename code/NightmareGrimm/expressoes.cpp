#include "expressoes.h"

namespace {

// Uma linha por expressão, na mesma ordem do enum Expressao.
// Para criar uma expressão nova: acrescente o nome no enum (antes de
// QUANTIDADE) e a linha correspondente aqui.
const DefinicaoExpressao EXPRESSOES[] = {
  // nome           símbolo  sobrancelhas            boca              modelo
  { "FELIZ",        ":)",   Sobrancelhas::NENHUMA, Boca::SORRISO },  // Feliz.png
  { "FELIZ_BRAVO",  ">:)",  Sobrancelhas::BRAVAS,  Boca::SORRISO },  // bravo-Feliz.png
  { "BRAVO",        ">:|",  Sobrancelhas::BRAVAS,  Boca::RETA    },  // bravo.png
  { "NEUTRO",       ":|",   Sobrancelhas::NENHUMA, Boca::RETA    },  // Neutro.png
  { "TRISTE",       "<:(",  Sobrancelhas::TRISTES, Boca::TRISTE  },  // triste.png
};

static_assert(sizeof(EXPRESSOES) / sizeof(EXPRESSOES[0]) == size_t(Expressao::QUANTIDADE),
              "EXPRESSOES precisa de uma linha para cada valor de Expressao");

}  // namespace

const DefinicaoExpressao& expressaoDefinicao(Expressao expressao) {
  return EXPRESSOES[uint8_t(expressao)];
}
