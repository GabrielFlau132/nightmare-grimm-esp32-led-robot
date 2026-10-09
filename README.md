# nightmare-grimm-esp32-led-robot

Nightmare Grimm é um robô de MDF com um rosto de LEDs controlado por um ESP32.
São cinco expressões, piscada automática e uma frente de 15,2 × 7,2 cm, onde
só os LEDs ficam visíveis.

![As cinco expressões e a piscada](docs/expressoes.svg)

## Estrutura do repositório

```
nightmare-grimm-esp32-led-robot/
├── README.md
├── code/
│   └── NightmareGrimm/          sketch do Arduino IDE (abrir NightmareGrimm.ino)
│       ├── NightmareGrimm.ino   setup() e loop()
│       ├── config.h             pinos e tempos: o que normalmente se ajusta
│       ├── grupos_led.h/.cpp    grupos de LEDs e escrita nos pinos
│       ├── expressoes.h/.cpp    tabela das cinco expressões
│       ├── rosto.h/.cpp         expressão ativa + olhos, traduzidos em grupos
│       ├── piscada.h/.cpp       piscada automática não bloqueante
│       └── comandos.h/.cpp      serial, modo demonstração e teste de montagem
├── docs/
│   ├── gerar_layout.py          define e confere a posição de cada LED (gera os arquivos abaixo)
│   ├── leds.csv                 coordenadas de cada LED em mm
│   ├── mapa-leds.svg            posição e grupo de cada LED, escala 1:1
│   ├── corte-frente.svg         arquivo de corte a laser da placa da frente
│   └── expressoes.svg           prévia das expressões com o layout final
└── Models/                      esboços das expressões (formato e grupos, não posições)
```

## Expressões

Os olhos são iguais em todos os modelos. O que muda entre as expressões são as
sobrancelhas e a boca.

| Modo          | Símbolo   | Sobrancelhas | Boca    | Grupos acesos | LEDs | Esboço            |
|---------------|-----------|--------------|---------|---------------|------|-------------------|
| `FELIZ`       | `:)`      | nenhuma      | sorriso | 1 2 · 6 7     | 30   | `Feliz.png`       |
| `FELIZ_BRAVO` | `>:)`     | bravas       | sorriso | 1 2 · 4 · 6 7 | 38   | `bravo-Feliz.png` |
| `BRAVO`       | `>:\|`    | bravas       | reta    | 1 2 · 4 · 6   | 32   | `bravo.png`       |
| `NEUTRO`      | `:\|`     | nenhuma      | reta    | 1 2 · 6       | 24   | `Neutro.png`      |
| `TRISTE`      | `<:(`     | tristes      | triste  | 1 2 · 5 · 8   | 36   | `triste.png`      |

- **Feliz → Feliz-Bravo:** só acende o grupo 4 (sobrancelhas bravas).
- **Feliz-Bravo → Bravo:** só apaga o grupo 7 (cantos do sorriso).
- **Piscada** (`piscando.png`): os olhos trocam o contorno (grupos 1 + 2) pela
  barra do meio (grupos 2 + 3) e depois voltam. As sobrancelhas e a boca não são
  tocadas.

Os números são os grupos da tabela abaixo. Cada máscara do firmware foi
conferida contra os esboços da pasta `Models/`. A única diferença em relação
aos esboços é a linha central da boca, que tem 4 LEDs em vez de 5.

## Grupos de LEDs

Cada grupo acende e apaga junto e usa um único pino do ESP32. As quantidades
vêm dos esboços, menos a linha central da boca (4 LEDs em vez de 5). O mapa em [`docs/mapa-leds.svg`](docs/mapa-leds.svg) mostra a
posição de cada LED com o número do grupo (ver [Layout dos LEDs](#layout-dos-leds)).

| # | Grupo                  | Cor no desenho | LEDs | GPIO | ULN2803A (entrada → saída) |
|---|------------------------|----------------|------|------|----------------------------|
| 1 | `OLHOS_ANEL`           | amarelo        | 16   | 25   | 1 → 18                     |
| 2 | `OLHOS_MEIO_LATERAL`   | amarelo        | 4    | 26   | 2 → 17                     |
| 3 | `OLHOS_MEIO_CENTRO`    | amarelo        | 4    | 27   | 3 → 16                     |
| 4 | `SOBRANCELHAS_BRAVAS`  | vermelho       | 8    | 32   | 4 → 15                     |
| 5 | `SOBRANCELHAS_TRISTES` | azul           | 6    | 33   | 5 → 14                     |
| 6 | `BOCA_RETA`            | vermelho       | 4    | 23   | 6 → 13                     |
| 7 | `BOCA_SORRISO`         | vermelho       | 6    | 19   | 7 → 12                     |
| 8 | `BOCA_TRISTE`          | azul           | 10   | 18   | 8 → 11                     |
|   | **Total**              |                | **58** |    |                            |

Por que os olhos têm três grupos: cada olho tem 12 LEDs. Com o olho aberto,
acendem os 10 do contorno. Na piscada acendem só os 4 da barra do meio, e os
2 das pontas dessa barra também fazem parte do contorno. Esses 2 ficam acesos
nos dois estados (grupo 2). Os outros 8 do contorno são o grupo 1, e os 2 do
centro da barra, que só acendem na piscada, são o grupo 3.

Os pinos evitam os de boot (0, 2, 5, 12, 15), os da flash (6–11), os que são
só de entrada (34–39) e a serial USB (1, 3). Os pinos 21 e 22 ficam livres para
I2C. Para mudar a ligação, basta alterar `config.h`.

## Layout dos LEDs

Os desenhos de `Models/` são esboços. O layout final fica em
[`docs/gerar_layout.py`](docs/gerar_layout.py), que posiciona cada LED, confere
as distâncias e gera os arquivos de `docs/`.

![Mapa dos LEDs](docs/mapa-leds.svg)

Regras do layout:

- **LEDs de 4,8 mm.** Entre as bordas de dois LEDs quaisquer há pelo menos
  **2 mm**, ou seja, 6,8 mm entre centros. Nenhum LED se sobrepõe a outro. No
  layout atual, o par mais próximo fica a 2,05 mm entre bordas, e o sorriso fica a
  pelo menos 12,9 mm (entre centros) dos LEDs dos olhos.
- **Distância até a borda da frente:** pelo menos 2 mm a partir da borda do LED.
- **Espaçamento uniforme:** em cada linha os LEDs ficam igualmente espaçados,
  com passos entre 6,85 e 8,5 mm em todo o rosto.
- **Olhos:** 4 colunas a cada 7,3 mm. As laterais têm 3 LEDs a cada 8,3 mm.
  As colunas do meio têm topo, barra e base.
- **Sobrancelhas:** ficam nas mesmas colunas dos olhos.
  - **Brava:** desce 2,7 mm a cada LED em direção ao centro do rosto.
  - **Triste:** começa uma coluna para fora do olho e sobe 2,7 mm a cada LED.
- **Boca:** colunas a cada 7 mm, centradas na frente, com 4 LEDs em cada
  linha central.
  - **Linha vermelha** (boca reta) e **linha azul** (boca triste): 7 mm de
    distância entre si.
  - **Cantos do sorriso:** sobem a partir da linha vermelha.
  - **Cantos da tristeza:** descem a partir da linha azul.
- **Simetria:** o lado direito é o espelho exato do esquerdo.

O script se recusa a gerar os arquivos se alguma regra for violada. Para
mudar o tamanho dos LEDs, a folga mínima ou a posição de alguma parte, edite as
constantes no topo do script e rode:

```bash
python docs/gerar_layout.py
```

Arquivos gerados:

| Arquivo | Para quê |
|---------|----------|
| `leds.csv` | coordenadas (mm) de cada LED, com grupo, cor e pino. A origem é o canto superior esquerdo da frente |
| `mapa-leds.svg` | gabarito em escala 1:1 com o número do grupo em cada LED. Imprima em 100% e confira a régua de 10 mm |
| `corte-frente.svg` | arquivo de corte da placa (152 × 72 mm): contorno e 58 furos de 4,8 mm, tudo em vermelho |
| `expressoes.svg` | como cada expressão fica acesa |

## Ligação elétrica (proposta)

Os desenhos não mostram a fiação. O que segue é uma proposta, e os valores
precisam ser conferidos com o datasheet dos LEDs que você comprar.

### Por que usar um driver

Um pino do ESP32 fornece 3,3 V e deve ficar abaixo de ~20 mA. O grupo 1 sozinho
tem 16 LEDs, ou ~80 mA. Por isso os LEDs são alimentados por 5 V (USB) e cada
grupo é ligado ao GND por um canal do **ULN2803A**. São 8 canais, um para cada
grupo, e as entradas aceitam os 3,3 V do ESP32. Pino do ESP32 em `HIGH` acende
o grupo.

```
+5 V ──┬──►|──[R]──┐
       ├──►|──[R]──┼── saída do canal N do ULN2803A
       └──►|──[R]──┘        (um fio de retorno por grupo)

GPIO do grupo N ──── entrada do canal N do ULN2803A
GND do ESP32 ─────── pino 9 do ULN2803A ─── GND da fonte
Pino 10 (COM) do ULN2803A: pode ficar desligado (só é usado com cargas indutivas)
```

- **Um resistor por LED, LEDs em paralelo.** Cada LED fica com o brilho
  próprio, e um LED queimado não apaga os outros.
- Atrás da frente ficam só **9 fios**: +5 V comum a todos os anodos e um
  retorno por grupo. Com um conector de 9 vias entre a frente e a placa do
  ESP32, dá para separar as duas partes na manutenção.

### Resistores

`R = (5 V − Vf do LED − ~0,9 V do ULN2803A) / corrente`. Com ~5 mA por LED,
suficiente para LEDs de alto brilho atrás de MDF:

| Cor      | Vf típico (confirmar) | R calculado | Valor comercial |
|----------|-----------------------|-------------|-----------------|
| Vermelho | ~2,0 V                | 420 Ω       | 470 Ω           |
| Amarelo  | ~2,1 V                | 400 Ω       | 390 Ω           |
| Azul     | ~3,0 V                | 220 Ω       | 220 Ω           |

Se as cores reais forem outras, refaça a conta. O brilho entre cores se
equilibra trocando o resistor.

### LEDs em série

Com 5 V, colocar LEDs em série **não é recomendado**:

- **Com o ULN2803A:** dois vermelhos em série (≈ 4,0 V) mais a queda do driver
  (≈ 0,9 V) já somam ~4,9 V. Não sobra margem para o resistor.
- **Com MOSFET canal N de nível lógico** (ex.: AO3400) no lugar do ULN2803A: a
  queda é quase zero, então dá para ligar **pares** de vermelhos ou amarelos em
  série com ~180 Ω, o que reduz o número de resistores. Em compensação, a
  corrente fica mais sensível à variação de Vf. Azuis nunca cabem em série em
  5 V.
- **Séries mais longas** exigiriam outra fonte (9–12 V). Além disso, grupos de
  6 e 10 LEDs não se dividem igualmente em séries iguais.

### Consumo

O pior caso é `FELIZ_BRAVO`: 38 LEDs × 5 mA ≈ 0,2 A, mais o ESP32
(~0,05–0,1 A sem Wi-Fi). Fica abaixo dos 0,5 A de uma porta USB. O ULN2803A
dissipa ~0,2 W nesse caso, sem problema.

## Montagem em MDF

- **Frente (15,2 × 7,2 cm):** corte `docs/corte-frente.svg`. Ele tem o
  contorno da placa e os furos de 4,8 mm, em escala 1:1. Teste num retalho
  antes, porque a largura do corte do laser varia. Se precisar, mude
  `DIAMETRO_FURO` no script e gere de novo. Os LEDs entram por trás e a aba os
  segura. Com 2 mm de folga, as abas (~5,8 mm) de LEDs vizinhos não se
  encostam.
- **Atrás da frente:** os resistores ficam soldados na perna de cada LED, mais
  o fio de +5 V e os retornos de cada grupo. Cola quente ou uma segunda camada
  de MDF prende os LEDs.
- **Eletrônica:** ESP32 em barra de pinos fêmea (sai sem dessoldar) e ULN2803A
  em soquete, numa placa perfurada. Deixe uma abertura para o cabo USB.
- **Manutenção:** tampa traseira removível (parafusos ou encaixe sem cola) e o
  conector de 9 vias entre a frente e a placa.

### Pontos que os desenhos não definem

1. **Cores reais dos LEDs.** Este README considera as cores do esboço:
   amarelo nos olhos, vermelho em bravo/sorriso e azul em triste.
2. **Faixas cinzas do esboço** (contorno dos olhos, traços da boca). Não está
   definido se devem ser gravadas no MDF ou se são só guias.
3. **Como escolher a expressão no robô pronto.** Não há botão nem sensor nos
   desenhos. Por enquanto a escolha é pela serial ou pelo modo demonstração
   (ver abaixo).
4. **Profundidade da caixa e fonte de alimentação.** Os desenhos não mostram.
   Este README considera alimentação pela USB.

## Código

### Gravar no ESP32

1. Arduino IDE 2 com um núcleo ESP32 instalado. O código usa só
   `pinMode`, `digitalWrite`, `millis` e `Serial`, sem APIs que mudaram entre
   versões. Foi compilado com o esp32-bluepad32 4.1.0, que é baseado no
   arduino-esp32 3.x, e deve funcionar também com o núcleo oficial
   "esp32 by Espressif".
2. Abra `code/NightmareGrimm/NightmareGrimm.ino`.
3. Placa **ESP32 Dev Module** e porta COM. Clique em Carregar.
4. Abra o Monitor Serial em **115200**.

### Comandos pelo Monitor Serial

| Tecla | Ação |
|-------|------|
| `1`–`5` | `FELIZ`, `FELIZ_BRAVO`, `BRAVO`, `NEUTRO`, `TRISTE` (e desliga a demonstração) |
| `p` | pisca agora |
| `d` | liga/desliga o modo demonstração (troca de expressão a cada 5 s) |
| `t` | teste de montagem: acende um grupo por vez, de 1 a 8, e volta ao rosto |
| `?` | ajuda |

Ao ligar, o robô começa em `NEUTRO` com a demonstração ativa. Isso permite ver
todas as expressões sem computador.

### Ajustes (`config.h`)

| Constante | Padrão | O que faz |
|-----------|--------|-----------|
| `PINO_*` | ver tabela de grupos | pino de cada grupo |
| `NIVEL_LED_ACESO` | `HIGH` | `HIGH` com driver no GND (ULN2803A/NPN/MOSFET). `LOW` com LED ligado direto entre 3,3 V e o pino |
| `EXPRESSAO_INICIAL` | `NEUTRO` | expressão ao ligar |
| `DEMO_AO_LIGAR` / `DEMO_INTERVALO_MS` | `true` / 5000 | modo demonstração |
| `PISCADA_INTERVALO_MIN_MS` / `_MAX_MS` | 2500 / 6000 | espera sorteada entre piscadas |
| `PISCADA_FECHADO_MS` | 150 | tempo com os olhos fechados |
| `PISCADA_DUPLA_CHANCE` | 20 | % de chance de piscar duas vezes (0 desliga) |
| `PISCADA_DUPLA_PAUSA_MS` | 120 | olhos abertos entre as duas piscadas |

### Como funciona

- `rosto` guarda **expressão** e **olhos** separados e monta a máscara de
  grupos combinando olhos, sobrancelhas e boca. A piscada só chama
  `rostoDefinirOlhos()`, então não tem como alterar sobrancelhas ou boca.
  Trocar de expressão no meio de uma piscada também não a interrompe.
- `gruposAplicar()` escreve só nos pinos que mudaram. Feliz → Feliz-Bravo, por
  exemplo, mexe em um único pino.
- Nada usa `delay()`. Piscada, demonstração e teste comparam `millis()` a cada
  volta do `loop()`, e as contas funcionam mesmo quando o `millis()` dá a volta
  (~49 dias).

### Adicionar uma expressão

1. Em `expressoes.h`, acrescente o nome no `enum class Expressao`, antes de
   `QUANTIDADE`.
2. Em `expressoes.cpp`, acrescente a linha correspondente na tabela
   `EXPRESSOES`, na mesma posição.

A tecla `6` passa a selecioná-la e a demonstração a inclui. Se a expressão
precisar de uma sobrancelha ou boca nova, acrescente o valor em `Sobrancelhas`
ou `Boca` e diga em `rosto.cpp` (`mascaraSobrancelhas` / `mascaraBoca`) quais
grupos ele acende.

## Montagem passo a passo (sugestão)

1. Grave o código com nada ligado e confira a ajuda no Monitor Serial.
2. Monte o ULN2803A e **um** grupo pequeno (ex.: `BOCA_RETA`). Use `t` e `4`
   para conferir.
3. Acrescente os outros grupos um por vez, rodando `t` e comparando com
   `docs/mapa-leds.svg`. O número em cada LED do mapa é o grupo dele.
4. Com tudo funcionando na bancada, passe para a caixa de MDF.
