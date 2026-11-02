// Configuração do MQTT usada pelo navegador e pelo servidor.
// Tem que bater com o ESP32: tópicos <prefixo>/<id>/estado, evento, online e cmd.

export const PREFIXO = process.env.NEXT_PUBLIC_MQTT_PREFIXO ?? "academia/armario";
export const WS_URL = process.env.NEXT_PUBLIC_MQTT_WS_URL ?? "wss://broker.hivemq.com:8884/mqtt";

export const topico = (id: string, tipo: "estado" | "evento" | "online" | "cmd") =>
  `${PREFIXO}/${id}/${tipo}`;

// Código do armário: uma letra e dois números, como A07.
export const CODIGO_VALIDO = /^[A-Za-z][0-9]{2}$/;

export type EstadoArmario = {
  id: string;
  estado: "livre" | "pareado" | "ocupado";
  porta: "aberta" | "fechada";
  travado: boolean;
  distancia_cm: number;
};
