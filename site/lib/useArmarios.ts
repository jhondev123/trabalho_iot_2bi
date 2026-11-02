"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import mqtt, { type MqttClient } from "mqtt";
import { PREFIXO, WS_URL, topico, type EstadoArmario } from "./mqtt-config";

export type Armario = Partial<EstadoArmario> & {
  id: string;
  online?: boolean;
  atualizadoEm?: number; // quando a página recebeu o último estado
};

export type EventoAoVivo = { id: string; armario: string; evento: string; tentativas?: number; hora: number };

// Conecta no broker pelo navegador (WebSocket) e mantém a lista de armários em memória.
// O ESP32 publica o estado com "retain", então ao assinar o estado mais recente já chega.
export function useArmarios() {
  const [armarios, setArmarios] = useState<Record<string, Armario>>({});
  const [eventos, setEventos] = useState<EventoAoVivo[]>([]);
  const [conectado, setConectado] = useState(false);
  const cliente = useRef<MqttClient | null>(null);

  useEffect(() => {
    const c = mqtt.connect(WS_URL, { reconnectPeriod: 3000, clientId: "site-" + Math.random().toString(16).slice(2, 10) });
    cliente.current = c;

    c.on("connect", () => {
      setConectado(true);
      c.subscribe([`${PREFIXO}/+/estado`, `${PREFIXO}/+/online`, `${PREFIXO}/+/evento`]);
    });
    c.on("close", () => setConectado(false));

    c.on("message", (topic, payload) => {
      const partes = topic.split("/");
      const tipo = partes[partes.length - 1];
      const id = partes[partes.length - 2];
      let dados: any;
      try {
        dados = JSON.parse(payload.toString());
      } catch {
        return; // mensagem que não é JSON: ignora
      }
      // Broker público: qualquer um pode publicar. Ignora o que não for um objeto JSON
      // (ex.: "null"), senão o erro derruba a entrega das próximas mensagens.
      if (!dados || typeof dados !== "object") return;

      if (tipo === "estado") {
        setArmarios((antes) => ({ ...antes, [id]: { ...antes[id], ...dados, id, atualizadoEm: Date.now() } }));
      } else if (tipo === "online") {
        setArmarios((antes) => ({ ...antes, [id]: { ...antes[id], id, online: !!dados.online } }));
      } else if (tipo === "evento") {
        const novo: EventoAoVivo = {
          id: crypto.randomUUID(),
          armario: id,
          evento: String(dados.evento ?? ""),
          tentativas: dados.tentativas,
          hora: Date.now(),
        };
        setEventos((antes) => [novo, ...antes].slice(0, 50));
      }
    });

    return () => {
      c.end(true);
      cliente.current = null;
    };
  }, []);

  // Publica um comando no tópico cmd do armário. Retorna false se ainda não conectou.
  const enviar = useCallback((id: string, comando: Record<string, string>) => {
    if (!cliente.current?.connected) return false;
    cliente.current.publish(topico(id, "cmd"), JSON.stringify(comando));
    return true;
  }, []);

  return { armarios, eventos, conectado, enviar };
}
