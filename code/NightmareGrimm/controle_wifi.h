// Controle pelo celular: o ESP32 cria uma rede Wi-Fi própria (WIFI_NOME /
// WIFI_SENHA em config.h) e serve uma página com um botão por expressão.
// Conecte o celular na rede e abra http://192.168.4.1 (em muitos celulares a
// página abre sozinha ao conectar).
#pragma once

void controleWifiIniciar();

// Chamar a cada volta do loop.
void controleWifiAtualizar();

// Mostra na serial se a rede está no ar, canal e aparelhos conectados.
void controleWifiMostrarEstado();
