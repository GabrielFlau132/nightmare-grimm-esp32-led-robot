#include "controle_wifi.h"

#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#include "comandos.h"
#include "config.h"
#include "musica.h"
#include "rosto.h"

namespace {

WebServer servidor(80);
bool redeCriada = false;
DNSServer dns;  // responde qualquer endereço com o IP do robô, para a página abrir sozinha

// Os botões são montados a partir da lista que /estado devolve, então uma
// expressão nova em expressoes.cpp aparece no celular sem mexer aqui.
const char PAGINA[] = R"HTML(<!doctype html>
<html lang="pt-br"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Nightmare Grimm</title>
<style>
body{margin:0;padding:20px 16px;font-family:system-ui,sans-serif;background:#121212;color:#eee;
display:flex;flex-direction:column;align-items:center}
h1{font-size:22px;margin:4px 0}
#atual{color:#aaa;font-size:15px;margin-bottom:18px;min-height:20px;text-align:center}
.grade{display:grid;grid-template-columns:1fr 1fr;gap:12px;width:100%;max-width:420px}
button{font-size:17px;padding:16px 8px;border:0;border-radius:14px;background:#2a2a2e;color:#eee}
button b{display:block;font:bold 26px monospace;margin-bottom:4px}
button.ativo{background:#c62828;color:#fff}
#demo,.musica{grid-column:1/-1;background:#1d3550}
#demo.ativo,.musica.ativo{background:#1976d2}
h2{font-size:15px;color:#aaa;font-weight:normal;margin:6px 0 0;grid-column:1/-1}
</style></head><body>
<h1>Nightmare Grimm</h1>
<div id="atual">Conectando...</div>
<div class="grade" id="grade"></div>
<script>
const grade=document.getElementById('grade'), atual=document.getElementById('atual');
let demo=null, montado=false;
function nomeBonito(n){n=n.replace(/_/g,' ').toLowerCase();return n[0].toUpperCase()+n.slice(1);}
function montar(lista,musicas){
  lista.forEach(e=>{const b=document.createElement('button');b.dataset.e=e.nome;
    b.innerHTML='<b></b>';b.firstChild.textContent=e.simbolo;b.append(nomeBonito(e.nome));
    b.onclick=()=>pedir('/expressao?nome='+e.nome);grade.append(b);});
  demo=document.createElement('button');demo.id='demo';demo.textContent='Demonstração';
  demo.onclick=()=>pedir('/demo?ligada='+(demo.classList.contains('ativo')?0:1));grade.append(demo);
  const h2=document.createElement('h2');h2.textContent='Músicas';grade.append(h2);
  musicas.forEach((nome,i)=>{const b=document.createElement('button');b.className='musica';b.dataset.m=i;
    b.textContent='♪ '+nome;
    b.onclick=()=>pedir(b.classList.contains('ativo')?'/musica':'/musica?tocar='+i);grade.append(b);});
  montado=true;}
function mostrar(s){
  if(!montado)montar(s.lista,s.musicas);
  grade.querySelectorAll('[data-e]').forEach(b=>b.classList.toggle('ativo',b.dataset.e==s.expressao));
  demo.classList.toggle('ativo',s.demo);
  grade.querySelectorAll('[data-m]').forEach(b=>b.classList.toggle('ativo',b.dataset.m==s.musica));
  atual.textContent=(s.demo?'Demonstração ligada · ':'')+nomeBonito(s.expressao);}
function pedir(u){fetch(u).then(r=>r.json()).then(mostrar)
  .catch(()=>{atual.textContent='Sem conexão com o robô';});}
pedir('/estado');setInterval(()=>pedir('/estado'),2000);
</script></body></html>)HTML";

void enviarEstado() {
  String json = "{\"expressao\":\"";
  json += expressaoDefinicao(rostoExpressao()).nome;
  json += "\",\"demo\":";
  json += comandosDemoAtiva() ? "true" : "false";
  json += ",\"musica\":";
  json += musicaTocando() ? String(musicaSelecionada()) : String(-1);
  json += ",\"musicas\":[";
  for (uint8_t i = 0; i < musicaQuantidade(); i++) {
    if (i > 0) {
      json += ",";
    }
    json += "\"";
    json += musicaNome(i);  // nomes sem aspas nem barra invertida
    json += "\"";
  }
  json += "]";
  json += ",\"lista\":[";
  for (uint8_t i = 0; i < uint8_t(Expressao::QUANTIDADE); i++) {
    const DefinicaoExpressao& definicao = expressaoDefinicao(Expressao(i));
    if (i > 0) {
      json += ",";
    }
    json += "{\"nome\":\"";
    json += definicao.nome;
    json += "\",\"simbolo\":\"";
    json += definicao.simbolo;
    json += "\"}";
  }
  json += "]}";
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(200, "application/json", json);
}

void tratarExpressao() {
  const String nome = servidor.arg("nome");
  for (uint8_t i = 0; i < uint8_t(Expressao::QUANTIDADE); i++) {
    if (nome.equalsIgnoreCase(expressaoDefinicao(Expressao(i)).nome)) {
      comandosEscolherExpressao(Expressao(i));
      enviarEstado();
      return;
    }
  }
  servidor.send(400, "text/plain", "expressao desconhecida");
}

void tratarMusica() {
  Serial.printf("Celular pediu musica: %s\n", servidor.hasArg("tocar") ? servidor.arg("tocar").c_str() : "parar");
  if (servidor.hasArg("tocar")) {
    musicaTocar(servidor.arg("tocar").toInt(), millis());
  } else {
    musicaParar();
  }
  enviarEstado();
}

void tratarDemo() {
  comandosDefinirDemo(servidor.arg("ligada") == "1");
  enviarEstado();
}

// Qualquer outro endereço (inclusive os testes de internet do celular) vai
// para a página, o que faz ela abrir sozinha em muitos aparelhos.
void redirecionar() {
  servidor.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
  servidor.send(302, "text/plain", "");
}

}  // namespace

void controleWifiIniciar() {
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(WIFI_NOME, WIFI_SENHA)) {
    Serial.println(F("Wi-Fi: FALHOU ao criar a rede. O controle pelo celular nao vai funcionar."));
    return;
  }
  redeCriada = true;
  dns.start(53, "*", WiFi.softAPIP());

  servidor.on("/", HTTP_GET, [] { servidor.send(200, "text/html", PAGINA); });
  servidor.on("/estado", HTTP_GET, enviarEstado);
  servidor.on("/expressao", HTTP_GET, tratarExpressao);
  servidor.on("/demo", HTTP_GET, tratarDemo);
  servidor.on("/musica", HTTP_GET, tratarMusica);
  servidor.onNotFound(redirecionar);
  servidor.begin();

  Serial.printf("Wi-Fi: rede \"%s\", senha \"%s\". Abra http://%s no celular.\n", WIFI_NOME, WIFI_SENHA,
                WiFi.softAPIP().toString().c_str());
}

void controleWifiMostrarEstado() {
  const wifi_mode_t modo = WiFi.getMode();
  const bool noAr = redeCriada && (modo == WIFI_AP || modo == WIFI_AP_STA);
  Serial.printf("Wi-Fi: rede %s | nome \"%s\" | canal %d | IP %s | %d aparelho(s) conectado(s)\n",
                noAr ? "NO AR" : "FORA DO AR", WiFi.softAPSSID().c_str(), WiFi.channel(),
                WiFi.softAPIP().toString().c_str(), WiFi.softAPgetStationNum());
}

void controleWifiAtualizar() {
  if (!redeCriada) {
    return;
  }
  dns.processNextRequest();
  servidor.handleClient();
}
