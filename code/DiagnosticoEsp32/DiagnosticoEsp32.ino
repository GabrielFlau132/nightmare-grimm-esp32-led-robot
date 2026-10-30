// Diagnóstico do ESP32 do Nightmare Grimm.
//
// Mostra no Monitor Serial (115200), a cada 5 s: chip, memória, motivo do
// último reset, rádio Wi-Fi e o teste dos 10 pinos dos LEDs. No teste de
// pino, cada saída vai a LOW e a HIGH e o nível é lido de volta: se não
// acompanhar, há curto (fio encostando no GND ou no 3V3) ou o pino está ruim.
//
// No fim os LEDs ficam acesos. Depois do teste, grave de novo o sketch
// NightmareGrimm.

#include <WiFi.h>
#include <esp_system.h>

const uint8_t PINOS[] = { 25, 13, 26, 27, 32, 33, 23, 19, 18, 4 };
const char* const FIOS[] = { "marrom", "vermelho", "laranja", "amarelo", "verde",
                             "azul", "roxo", "cinza", "branco", "vinho" };

int redesWifi = -1;

const char* motivoReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:   return "ligou na energia";
    case ESP_RST_EXT:       return "botao EN / pino de reset";
    case ESP_RST_SW:        return "reset por software (ex.: depois de gravar)";
    case ESP_RST_PANIC:     return "TRAVOU (panic) - problema no codigo";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:       return "WATCHDOG - travou";
    case ESP_RST_BROWNOUT:  return "BROWNOUT - tensao caiu (fonte/cabo fraco)";
    case ESP_RST_DEEPSLEEP: return "acordou do deep sleep";
    default:                return "outro";
  }
}

// true = o pino acompanha LOW e HIGH (leitura do próprio pino de saída)
bool testarPino(uint8_t pino, bool& leuLow, bool& leuHigh) {
  digitalWrite(pino, LOW);
  delayMicroseconds(200);
  leuLow = digitalRead(pino);
  digitalWrite(pino, HIGH);
  delayMicroseconds(200);
  leuHigh = digitalRead(pino);
  return !leuLow && leuHigh;
}

void relatorio() {
  Serial.println();
  Serial.println("========== Diagnostico do ESP32 ==========");
  Serial.printf("Chip: %s rev %d, %d nucleos, %lu MHz\n", ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores(), (unsigned long)ESP.getCpuFreqMHz());
  Serial.printf("Flash: %lu MB a %lu MHz | sketch: %lu KB de %lu KB livres\n",
                (unsigned long)(ESP.getFlashChipSize() / (1024 * 1024)),
                (unsigned long)(ESP.getFlashChipSpeed() / 1000000), (unsigned long)(ESP.getSketchSize() / 1024),
                (unsigned long)((ESP.getSketchSize() + ESP.getFreeSketchSpace()) / 1024));
  Serial.printf("RAM: %lu KB livres de %lu KB (menor livre ate agora: %lu KB)\n",
                (unsigned long)(ESP.getFreeHeap() / 1024), (unsigned long)(ESP.getHeapSize() / 1024),
                (unsigned long)(ESP.getMinFreeHeap() / 1024));
  Serial.printf("PSRAM: %s\n", ESP.getPsramSize() ? "sim" : "nao tem (normal no DevKit)");
  Serial.printf("Ultimo reset: %s\n", motivoReset(esp_reset_reason()));
  Serial.printf("Ligado ha: %lu s\n", (unsigned long)(millis() / 1000));
  if (redesWifi >= 0) {
    Serial.printf("Wi-Fi: radio OK, %d redes encontradas\n", redesWifi);
  } else {
    Serial.println("Wi-Fi: FALHOU a busca de redes");
  }

  Serial.println("Pinos dos LEDs (LOW / HIGH lidos de volta):");
  int ruins = 0;
  for (uint8_t i = 0; i < sizeof(PINOS); i++) {
    bool l, h;
    const bool ok = testarPino(PINOS[i], l, h);
    const char* diag = ok ? "OK" : (!h && !l) ? "PRESO EM LOW: curto com o GND?" :
                       (l && h) ? "PRESO EM HIGH: curto com 3V3/5V?" : "estranho";
    Serial.printf("  GPIO %2u (%-8s)  %d / %d  %s\n", PINOS[i], FIOS[i], l, h, diag);
    if (!ok) ruins++;
  }
  Serial.printf("Resultado: %s\n", ruins == 0 ? "TUDO OK" : "ATENCAO: veja os pinos acima");
  Serial.println("(LEDs ficam acesos)");
}

void setup() {
  Serial.begin(115200);
  for (uint8_t i = 0; i < sizeof(PINOS); i++) {
    pinMode(PINOS[i], OUTPUT);
    digitalWrite(PINOS[i], HIGH);
  }
  WiFi.mode(WIFI_STA);
  redesWifi = WiFi.scanNetworks();
  WiFi.mode(WIFI_OFF);
}

void loop() {
  relatorio();
  delay(5000);
}
