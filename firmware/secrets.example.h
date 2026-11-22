// Modelo das credenciais do firmware. COPIE para "secrets.h" e troque os valores.
// O secrets.h esta no .gitignore e nunca deve ir para o repositorio.
// No Wokwi: crie uma aba chamada "secrets.h" (botao "+" ao lado das abas) e cole este conteudo.
// Se o secrets.h nao existir, o sketch.ino usa este arquivo (valores de demonstracao).

#pragma once

// ---------- Wi-Fi ----------
const char* WIFI_SSID = "Wokwi-GUEST";  // rede do simulador, sem senha
const char* WIFI_PASS = "";

// ---------- Broker MQTT ----------
// Demo: broker publico da HiveMQ, sem usuario e senha.
// MQTT_USAR_TLS 0 -> porta 1883 (sem criptografia, configuracao testada em 02/11)
// MQTT_USAR_TLS 1 -> porta 8883 com TLS e verificacao do certificado (MQTT_CA_RAIZ abaixo)
#define MQTT_USAR_TLS 0
const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = MQTT_USAR_TLS ? 8883 : 1883;
const char* MQTT_USUARIO = "";  // vazio = sem autenticacao (ex.: HiveMQ Cloud pede usuario e senha)
const char* MQTT_SENHA   = "";

// Prefixo dos topicos. Tem que ser IGUAL ao NEXT_PUBLIC_MQTT_PREFIXO do site.
// Num broker publico, prefira um prefixo unico do grupo (ex.: academia/armario-g7k2q9).
const char* MQTT_PREFIXO = "academia/armario";

// ---------- Administrador ----------
// Senha que o comando {"acao":"abrir","admin":"..."} precisa trazer.
// Tem que ser igual ao ADMIN_SENHA_ARMARIO do site (.env.local). Troque no secrets.h real.
const char* ADMIN_SENHA = "admin123";

// ---------- Certificado raiz do broker (so usado com MQTT_USAR_TLS 1) ----------
// broker.hivemq.com:8883 usa certificado da Amazon (cadeia: mqttdashboard.com ->
// Amazon RSA 2048 M04 -> Amazon Root CA 1). Conferido com openssl em 23/11/2026.
// Certificado publico (nao e segredo); valido ate 2038.
const char* MQTT_CA_RAIZ = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";
