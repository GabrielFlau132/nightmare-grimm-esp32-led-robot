// Teste de montagem: acende todos os LEDs do Nightmare Grimm e deixa acesos.
//
// Mesma ligação do projeto: GPIO (pino S do shield) -> resistor -> perna longa,
// perna curta no GND. Pino em HIGH acende. Os pinos são os de
// code/NightmareGrimm/config.h.
//
// Depois do teste, grave de novo o sketch NightmareGrimm.

const uint8_t PINOS[] = { 25, 13, 26, 27, 32, 33, 23, 19, 18, 4 };
const char* const FIOS[] = { "marrom", "vermelho", "laranja", "amarelo", "verde",
                             "azul", "roxo", "cinza", "branco", "vinho" };

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < sizeof(PINOS); i++) {
    pinMode(PINOS[i], OUTPUT);
    digitalWrite(PINOS[i], HIGH);
  }
  delay(500);
  Serial.println();
  Serial.println("Teste: todos os LEDs acesos.");
  for (uint8_t i = 0; i < sizeof(PINOS); i++) {
    Serial.printf("  GPIO %2u (%s): HIGH\n", PINOS[i], FIOS[i]);
  }
}

void loop() {
}
