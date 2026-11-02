// Armario inteligente de academia - esqueleto v0 (Wokwi)
// Placa: ESP32 | Bibliotecas: ESP32Servo, PubSubClient, ArduinoJson
// Comecamos simples: tudo em RAM, broker publico sem senha.
// Depois: trocar para HiveMQ Cloud (usuario/senha/TLS), guardar token na flash e usar secrets.h.

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ---------- Configuracao ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const char* MQTT_HOST = "broker.hivemq.com";  // publico, so para testar
const int   MQTT_PORT = 1883;

const char* ID = "A07";
const char* ADMIN_SENHA = "admin123";  // so para teste, depois vai para o secrets.h

// Prefixo dos topicos: academia/armario/A07/...
String T_CMD    = String("academia/armario/") + ID + "/cmd";
String T_ESTADO = String("academia/armario/") + ID + "/estado";
String T_EVENTO = String("academia/armario/") + ID + "/evento";
String T_ONLINE = String("academia/armario/") + ID + "/online";

// ---------- Pinos ----------
const int PIN_SERVO = 13;
const int PIN_REED  = 27;  // LOW = porta fechada (INPUT_PULLUP)
const int PIN_TRIG  = 32;
const int PIN_ECHO  = 33;
const int PIN_LED_VERDE    = 25;
const int PIN_LED_AMARELO  = 26;
const int PIN_LED_VERMELHO = 19;

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

WiFiClient wifi;
PubSubClient mqtt(wifi);
Servo servo;
unsigned long ultimaTentativa = 0, ultimoPublish = 0;

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

void travar()    { servo.write(ANGULO_TRAVADO);    travado = true; }
void destravar() { servo.write(ANGULO_DESTRAVADO); travado = false; tDestravou = millis(); }

void atualizarLeds() {
  digitalWrite(PIN_LED_VERDE, estado == LIVRE);
  digitalWrite(PIN_LED_VERMELHO, estado == OCUPADO);
  // amarelo pisca no PAREADO
  digitalWrite(PIN_LED_AMARELO, estado == PAREADO && (millis() / 400) % 2 == 0);
}

// ---------- MQTT ----------
const char* nomeEstado() {
  if (estado == LIVRE) return "livre";
  if (estado == PAREADO) return "pareado";
  return "ocupado";
}

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
  tEstado = millis();
  if (novo == LIVRE) token = "";
  if (novo == PAREADO) portaAbriuNaJanela = false;
  publicarEstado();
}

void tokenInvalido() {
  tokensErrados++;
  publicarEvento("token_invalido");
  if (tokensErrados >= 3) {
    bloqueadoAte = millis() + BLOQUEIO_MS;
    tokensErrados = 0;
    publicarEvento("bloqueado");
  }
}

void aoReceber(char* topic, byte* payload, unsigned int len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) return;  // JSON invalido: ignora
  const char* acao = doc["acao"] | "";
  const char* tk = doc["token"] | "";
  const char* adm = doc["admin"] | "";

  if (millis() < bloqueadoAte) return;  // bloqueado por tokens errados

  if (strcmp(acao, "parear") == 0) {
    if (estado == LIVRE && strlen(tk) > 0) {
      token = tk;
      destravar();
      mudarEstado(PAREADO);
    }
  } else if (strcmp(acao, "abrir") == 0) {
    bool ehAdmin = strlen(adm) > 0 && strcmp(adm, ADMIN_SENHA) == 0;
    bool ehDono = strlen(tk) > 0 && token == tk;
    if (ehAdmin || ehDono) {
      tokensErrados = 0;
      destravar();
    } else {
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
    if (mqtt.connect(clientId.c_str(), nullptr, nullptr,
                     T_ONLINE.c_str(), 0, true, "{\"online\":false}")) {
      mqtt.publish(T_ONLINE.c_str(), "{\"online\":true}", true);
      mqtt.subscribe(T_CMD.c_str());
      publicarEstado();
    }
  }
}

// ---------- Regras locais (R2) ----------
void regras() {
  bool fechada = digitalRead(PIN_REED) == LOW;

  // porta acabou de abrir
  if (portaFechada && !fechada) {
    if (estado == PAREADO) portaAbriuNaJanela = true;
    publicarEstado();
  }

  // porta acabou de fechar: trava sempre e decide o estado pela distancia
  if (!portaFechada && fechada) {
    delay(300);  // espera o servo/porta assentar
    travar();
    distancia = lerDistanciaCm();
    if (estado == PAREADO || estado == OCUPADO) {
      mudarEstado(distancia < LIMITE_CM ? OCUPADO : LIVRE);
    }
    publicarEstado();
  }
  portaFechada = fechada;

  // PAREADO sem abrir a porta na janela: trava e volta para LIVRE
  if (estado == PAREADO && !portaAbriuNaJanela && millis() - tEstado > JANELA_PAREADO_MS) {
    travar();
    mudarEstado(LIVRE);
  }

  // destravou (dono/admin) e a porta nao abriu em 10 s: trava de novo
  if (estado == OCUPADO && !travado && portaFechada && millis() - tDestravou > TEMPO_ABRIR_PORTA_MS) {
    travar();
    publicarEstado();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_REED, INPUT_PULLUP);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  servo.attach(PIN_SERVO);
  travar();

  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(aoReceber);
  portaFechada = digitalRead(PIN_REED) == LOW;
  distancia = lerDistanciaCm();
}

void loop() {
  cuidarConexao();
  if (mqtt.connected()) mqtt.loop();
  regras();
  atualizarLeds();

  // publica o estado de tempos em tempos
  if (millis() - ultimoPublish > 10000) {
    distancia = lerDistanciaCm();
    publicarEstado();
  }
}
