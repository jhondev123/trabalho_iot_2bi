// Armario inteligente de academia - v2
// v1: correcoes dos testes do v0 (procure por "CORRECAO", 6 pontos).
// v2: correcoes da revisao tecnica de 02/11 (procure por "REVISAO", 5 pontos)
//     e credenciais fora do codigo (secrets.h).
// v3: display LCD 20x4 com estado, porta, trinco e rede (procure por "DISPLAY")
//     e mensagens claras no Monitor Serial.
// Placa: ESP32 | Bibliotecas: ESP32Servo, PubSubClient, ArduinoJson, LiquidCrystal I2C
//
// Como usar no Wokwi:
//   chave PORTA: alavanca para a ESQUERDA = porta fechada, DIREITA = porta aberta
//   sensor HC-SR04: clique e mude a distancia (menos de 26 cm = tem objeto, 30 cm = vazio)
//   o display mostra tudo; os LEDs repetem o estado (verde livre, amarelo pareado, vermelho ocupado)
// Estado e token ficam em RAM. Broker publico (ver o relatorio, secao Seguranca).

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Configuracao ----------
// Wi-Fi, broker, prefixo dos topicos e senha de admin ficam no secrets.h,
// que NAO vai para o git (.gitignore). Modelo: secrets.example.h.
// No Wokwi: crie a aba "secrets.h" e cole o conteudo do secrets.example.h.
#include "secrets.h"

const char* ID = "A07";

// Topicos: <MQTT_PREFIXO>/A07/... (padrao academia/armario/A07/...)
String T_CMD    = String(MQTT_PREFIXO) + "/" + ID + "/cmd";
String T_ESTADO = String(MQTT_PREFIXO) + "/" + ID + "/estado";
String T_EVENTO = String(MQTT_PREFIXO) + "/" + ID + "/evento";
String T_ONLINE = String(MQTT_PREFIXO) + "/" + ID + "/online";

// ---------- Pinos ----------
const int PIN_SERVO = 13;
const int PIN_REED  = 27;  // LOW = porta fechada (INPUT_PULLUP)
const int PIN_TRIG  = 32;
const int PIN_ECHO  = 33;
const int PIN_LED_VERDE    = 25;
const int PIN_LED_AMARELO  = 26;
const int PIN_LED_VERMELHO = 19;
// DISPLAY: LCD 20x4 com modulo I2C no endereco 0x27 (SDA 21, SCL 22)

// ---------- Parametros (na demo os tempos sao curtos) ----------
const float LIMITE_CM = 26.0;                 // fundo a 30 cm; menos que isso = tem objeto
const unsigned long JANELA_PAREADO_MS = 30000;   // 2 min na versao final
const unsigned long TEMPO_ABRIR_PORTA_MS = 10000; // destravou e nao abriu: trava de novo
const unsigned long BLOQUEIO_MS = 60000;
const int ANGULO_TRAVADO = 0;
const int ANGULO_DESTRAVADO = 90;

enum Estado { LIVRE, PAREADO, OCUPADO };

Estado estado = LIVRE;
String token = "";
bool travado = true;
bool portaFechada = true;
bool portaAbriuNaJanela = false;
unsigned long tEstado = 0;      // quando entrou no estado atual
unsigned long tDestravou = 0;   // quando destravou
int tokensErrados = 0;
unsigned long bloqueadoAte = 0;
float distancia = 0;

#if MQTT_USAR_TLS
WiFiClientSecure wifi;  // TLS (porta 8883), confere o certificado do broker
#else
WiFiClient wifi;        // sem TLS (porta 1883)
#endif
PubSubClient mqtt(wifi);
Servo servo;
LiquidCrystal_I2C lcd(0x27, 20, 4);
unsigned long ultimaTentativa = 0, ultimoPublish = 0, ultimaMedida = 0;

// ---------- Hardware ----------
float lerDistanciaCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long us = pulseIn(PIN_ECHO, HIGH, 30000);
  if (us == 0) return 400;  // sem eco = muito longe
  return us * 0.0343 / 2.0;
}

void publicarEstado();  // declarada aqui porque destravar() usa

void travar() {
  if (!travado) Serial.println("[trinco] TRAVADO");
  servo.write(ANGULO_TRAVADO);
  travado = true;
}

void destravarSilencioso() {
  Serial.println("[trinco] DESTRAVADO");
  servo.write(ANGULO_DESTRAVADO);
  travado = false;
  tDestravou = millis();
}

// CORRECAO 3: destravar avisa o novo estado na hora (antes so aparecia no envio periodico)
void destravar() {
  destravarSilencioso();
  publicarEstado();
}

void atualizarLeds() {
  digitalWrite(PIN_LED_VERDE, estado == LIVRE);
  digitalWrite(PIN_LED_VERMELHO, estado == OCUPADO);
  // amarelo pisca no PAREADO
  digitalWrite(PIN_LED_AMARELO, estado == PAREADO && (millis() / 400) % 2 == 0);
}

const char* nomeEstado() {
  if (estado == LIVRE) return "livre";
  if (estado == PAREADO) return "pareado";
  return "ocupado";
}

// ---------- DISPLAY ----------
// 4 linhas: rede e distancia | estado | porta | trinco
String linhasLcd[4];
unsigned long ultimoLcd = 0;

// completa com espacos e so redesenha a linha que mudou (evita o display piscar)
void escreverLinha(int n, String texto) {
  while (texto.length() < 20) texto += ' ';
  if (texto == linhasLcd[n]) return;
  linhasLcd[n] = texto;
  lcd.setCursor(0, n);
  lcd.print(texto);
}

void atualizarDisplay() {
  if (millis() - ultimoLcd < 250) return;
  ultimoLcd = millis();

  // linha 0: conexao (ou bloqueio) a esquerda, distancia medida a direita
  String esquerda;
  if (millis() < bloqueadoAte) {
    esquerda = "BLOQUEADO " + String((bloqueadoAte - millis()) / 1000) + "s";
  } else if (mqtt.connected()) {
    esquerda = String(ID) + " ONLINE";
  } else if (WiFi.status() == WL_CONNECTED) {
    esquerda = String(ID) + " SEM MQTT";
  } else {
    esquerda = String(ID) + " SEM WI-FI";
  }
  String direita = String(distancia, 0) + "cm";
  String linha0 = esquerda;
  while (linha0.length() + direita.length() < 20) linha0 += ' ';
  escreverLinha(0, linha0 + direita);

  // linha 1: estado do armario (no PAREADO mostra quanto falta da janela)
  String linha1 = "Estado: ";
  if (estado == LIVRE) linha1 += "LIVRE";
  if (estado == OCUPADO) linha1 += "OCUPADO";
  if (estado == PAREADO) {
    linha1 += "PAREADO";
    long resta = ((long)JANELA_PAREADO_MS - (long)(millis() - tEstado)) / 1000;
    if (!portaAbriuNaJanela && resta >= 0) linha1 += " " + String(resta) + "s";
  }
  escreverLinha(1, linha1);

  // linha 2: porta (aberta com o trinco travado = forcada)
  if (portaFechada) escreverLinha(2, "Porta : FECHADA");
  else if (travado) escreverLinha(2, "Porta : FORCADA!");
  else escreverLinha(2, "Porta : ABERTA");

  // linha 3: trinco (servo)
  escreverLinha(3, travado ? "Trinco: TRAVADO" : "Trinco: DESTRAVADO");
}

// ---------- MQTT ----------

void publicarEstado() {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["id"] = ID;
  doc["estado"] = nomeEstado();
  doc["porta"] = portaFechada ? "fechada" : "aberta";
  doc["travado"] = travado;
  doc["distancia_cm"] = round(distancia * 10) / 10.0;
  char buf[200];
  serializeJson(doc, buf);
  mqtt.publish(T_ESTADO.c_str(), buf, true);  // retain
  ultimoPublish = millis();
}

void publicarEvento(const char* nome) {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["id"] = ID;
  doc["evento"] = nome;
  doc["tentativas"] = tokensErrados;
  char buf[120];
  serializeJson(doc, buf);
  mqtt.publish(T_EVENTO.c_str(), buf);
}

void mudarEstado(Estado novo) {
  estado = novo;
  Serial.printf("[estado] %s\n", nomeEstado());
  tEstado = millis();
  if (novo == LIVRE) token = "";
  // REVISAO 4: se parear com a porta ja aberta, a janela ja conta como "abriu"
  if (novo == PAREADO) portaAbriuNaJanela = !portaFechada;
  publicarEstado();
}

void tokenInvalido() {
  tokensErrados++;
  Serial.printf("[cmd] token invalido (%d de 3)\n", tokensErrados);
  publicarEvento("token_invalido");
  if (tokensErrados >= 3) {
    bloqueadoAte = millis() + BLOQUEIO_MS;
    Serial.println("[cmd] BLOQUEADO por 60 s (o admin continua abrindo)");
    publicarEvento("bloqueado");  // REVISAO 5: publica antes de zerar (antes saia tentativas:0)
    tokensErrados = 0;
  }
}

void aoReceber(char* topic, byte* payload, unsigned int len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) return;  // JSON invalido: ignora
  const char* acao = doc["acao"] | "";
  const char* tk = doc["token"] | "";
  const char* adm = doc["admin"] | "";
  bool ehAdmin = strlen(adm) > 0 && strcmp(adm, ADMIN_SENHA) == 0;
  Serial.printf("[cmd] recebido: %s\n", acao);

  // bloqueado por tokens errados (REVISAO 3: o admin continua podendo abrir)
  if (millis() < bloqueadoAte && !ehAdmin) return;

  if (strcmp(acao, "parear") == 0) {
    if (estado == LIVRE && strlen(tk) > 0) {
      token = tk;
      destravarSilencioso();  // mudarEstado ja publica
      mudarEstado(PAREADO);
    }
  } else if (strcmp(acao, "abrir") == 0) {
    bool ehDono = strlen(tk) > 0 && token == tk;
    if (ehAdmin || ehDono) {
      tokensErrados = 0;
      destravar();
    } else if (estado == OCUPADO && (strlen(tk) > 0 || strlen(adm) > 0)) {
      // CORRECAO 4: so conta tentativa errada quando ha um dono
      // REVISAO 2: {"acao":"abrir"} sem token nem senha e ignorado (nao conta como erro)
      tokenInvalido();
    }
  }
}

// reconexao nao bloqueante: tenta a cada 5 s, as regras continuam rodando
void cuidarConexao() {
  if (millis() - ultimaTentativa < 5000) return;
  ultimaTentativa = millis();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    return;
  }
  if (!mqtt.connected()) {
    String clientId = String("armario-") + ID + "-" + String(random(0xffff), HEX);
    // usuario/senha vazios = broker sem autenticacao (broker publico)
    const char* usuario = strlen(MQTT_USUARIO) > 0 ? MQTT_USUARIO : nullptr;
    const char* senha   = strlen(MQTT_SENHA) > 0 ? MQTT_SENHA : nullptr;
    if (mqtt.connect(clientId.c_str(), usuario, senha,
                     T_ONLINE.c_str(), 0, true, "{\"online\":false}")) {
      Serial.println("[rede] MQTT conectado");
      mqtt.publish(T_ONLINE.c_str(), "{\"online\":true}", true);
      mqtt.subscribe(T_CMD.c_str());
      publicarEstado();
    }
  }
}

// ---------- Regras locais (R2) ----------
void regras() {
  bool fechada = digitalRead(PIN_REED) == LOW;
  bool abriu = portaFechada && !fechada;
  bool fechou = !portaFechada && fechada;
  portaFechada = fechada;  // CORRECAO 1: atualiza ANTES de publicar (antes saia a porta antiga)

  // porta acabou de abrir
  if (abriu) {
    Serial.println(travado ? "[porta] ABERTA A FORCA (trinco travado)" : "[porta] ABERTA");
    if (travado) publicarEvento("porta_forcada");  // CORRECAO 5: abriu com o trinco travado
    if (estado == PAREADO) portaAbriuNaJanela = true;
    publicarEstado();
  }

  // porta acabou de fechar: trava sempre e decide o estado pela distancia
  if (fechou) {
    Serial.println("[porta] FECHADA");
    delay(300);  // espera o servo/porta assentar
    travar();
    distancia = lerDistanciaCm();
    if (estado == PAREADO || estado == OCUPADO) {
      mudarEstado(distancia < LIMITE_CM ? OCUPADO : LIVRE);  // ja publica
    } else {
      publicarEstado();  // CORRECAO 2: uma publicacao so (antes saiam duas)
    }
  }

  // PAREADO sem abrir a porta na janela: trava e volta para LIVRE
  bool janelaAcabou = millis() - tEstado > JANELA_PAREADO_MS;
  if (estado == PAREADO && !portaAbriuNaJanela && janelaAcabou) {
    travar();
    mudarEstado(LIVRE);
  }

  // destravou (dono/admin) e a porta nao abriu em 10 s: trava de novo
  // CORRECAO 4: vale para qualquer estado menos PAREADO
  // (antes o admin deixava um LIVRE destravado)
  bool passouTempo = millis() - tDestravou > TEMPO_ABRIR_PORTA_MS;
  if (estado != PAREADO && !travado && portaFechada && passouTempo) {
    travar();
    publicarEstado();
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  escreverLinha(0, String("Armario ") + ID);
  escreverLinha(1, "Iniciando...");
  pinMode(PIN_REED, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  servo.attach(PIN_SERVO);
  travar();

  WiFi.mode(WIFI_STA);
#if MQTT_USAR_TLS
  wifi.setCACert(MQTT_CA_RAIZ);
#endif
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setKeepAlive(30);  // CORRECAO 6: mais folga se a simulacao/rede ficar lenta
  mqtt.setCallback(aoReceber);
  portaFechada = digitalRead(PIN_REED) == LOW;
  distancia = lerDistanciaCm();
  // REVISAO 1 (B6): reiniciou com algo dentro -> fica OCUPADO sem token,
  // assim ninguem consegue parear; so o admin abre.
  if (portaFechada && distancia < LIMITE_CM) estado = OCUPADO;
  Serial.printf("[inicio] estado %s, porta %s, distancia %.1f cm\n",
                nomeEstado(), portaFechada ? "fechada" : "aberta", distancia);
}

void loop() {
  cuidarConexao();
  if (mqtt.connected()) mqtt.loop();
  regras();
  atualizarLeds();

  // mede a distancia a cada 0,5 s para o display acompanhar o sensor
  if (millis() - ultimaMedida > 500) {
    distancia = lerDistanciaCm();
    ultimaMedida = millis();
  }
  atualizarDisplay();

  // publica o estado de tempos em tempos
  if (millis() - ultimoPublish > 10000) publicarEstado();
}
