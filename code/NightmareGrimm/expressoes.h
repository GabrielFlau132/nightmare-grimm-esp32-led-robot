// Expressões faciais do Nightmare Grimm.
//
// Cada expressão é descrita só pelo que muda entre elas: sobrancelhas e boca.
// Os olhos são iguais em todos os modelos (Models/*.png) e só mudam na piscada,
// por isso ficam fora desta tabela (ver rosto.h e piscada.h).
#pragma once
#include <Arduino.h>

enum class Expressao : uint8_t {
  FELIZ,        //  :)
  FELIZ_BRAVO,  // >:)
  BRAVO,        // >:|
  NEUTRO,       //  :|
  TRISTE,       // <:(
  QUANTIDADE    // não é uma expressão: marca o total
};

enum class Sobrancelhas : uint8_t { NENHUMA, BRAVAS, TRISTES };
enum class Boca : uint8_t { RETA, SORRISO, TRISTE };

struct DefinicaoExpressao {
  const char* nome;
  const char* simbolo;
  Sobrancelhas sobrancelhas;
  Boca boca;
};

const DefinicaoExpressao& expressaoDefinicao(Expressao expressao);
