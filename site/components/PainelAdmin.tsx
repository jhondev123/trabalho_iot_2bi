"use client";

import { useCallback, useEffect, useState } from "react";
import { useArmarios } from "@/lib/useArmarios";
import type { Registro } from "@/lib/registros";
import EstadoBadge from "./EstadoBadge";

const hora = (t: number | string) => new Date(t).toLocaleTimeString("pt-BR");
const dataHora = (t: string) => new Date(t).toLocaleString("pt-BR");

export default function PainelAdmin() {
  const { armarios, eventos, conectado } = useArmarios();
  const [registros, setRegistros] = useState<Registro[]>([]);
  const [mensagem, setMensagem] = useState("");

  const carregarRegistros = useCallback(async () => {
    try {
      const r = await fetch("/api/registros", { cache: "no-store" });
      if (r.ok) setRegistros(await r.json());
    } catch {
      // sem rede: tenta de novo no próximo ciclo
    }
  }, []);

  useEffect(() => {
    carregarRegistros();
    const t = setInterval(carregarRegistros, 10000);
    return () => clearInterval(t);
  }, [carregarRegistros]);

  async function abrir(id: string) {
    setMensagem(`Enviando comando para o armário ${id}...`);
    try {
      const r = await fetch("/api/admin/abrir", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ armario: id }),
      });
      const corpo = await r.json().catch(() => ({}));
      setMensagem(r.ok ? `Comando de abrir enviado ao armário ${id}.` : corpo.erro ?? "Falha ao enviar o comando.");
      carregarRegistros();
    } catch {
      setMensagem("Sem conexão com o servidor. Tente de novo.");
    }
  }

  const lista = Object.values(armarios).sort((a, b) => a.id.localeCompare(b.id));

  return (
    <main className="conteudo largo">
      <h1>Painel do admin</h1>
      <p className="mudo pequeno">{conectado ? "Conectado ao broker" : "Desconectado do broker"}</p>
      {mensagem && <p className="info">{mensagem}</p>}

      <h2>Armários</h2>
      <table>
        <thead>
          <tr>
            <th>Armário</th><th>Estado</th><th>Porta</th><th>Trinco</th><th>Distância</th><th>Atualizado</th><th></th>
          </tr>
        </thead>
        <tbody>
          {lista.length === 0 && (
            <tr><td colSpan={7} className="mudo">Nenhum armário apareceu ainda. Rode a simulação no Wokwi.</td></tr>
          )}
          {lista.map((a) => (
            <tr key={a.id}>
              <td><strong>{a.id}</strong></td>
              <td><EstadoBadge armario={a} /></td>
              <td>{a.porta ?? "-"}</td>
              <td>{a.travado === undefined ? "-" : a.travado ? "travado" : "destravado"}</td>
              <td>{a.distancia_cm !== undefined ? `${a.distancia_cm} cm` : "-"}</td>
              <td>{a.atualizadoEm ? hora(a.atualizadoEm) : "-"}</td>
              <td><button onClick={() => abrir(a.id)} disabled={a.online === false}>Abrir</button></td>
            </tr>
          ))}
        </tbody>
      </table>

      <div className="colunas">
        <section>
          <h2>Eventos ao vivo</h2>
          <ul className="lista">
            {eventos.length === 0 && <li className="mudo">Nenhum evento desde que a página abriu.</li>}
            {eventos.map((e) => (
              <li key={e.id}>
                {hora(e.hora)} · <strong>{e.armario}</strong> · {e.evento}
                {e.evento === "token_invalido" && e.tentativas !== undefined && ` (tentativa ${e.tentativas}/3)`}
              </li>
            ))}
          </ul>
        </section>

        <section>
          <h2>Registros salvos</h2>
          <ul className="lista">
            {registros.length === 0 && <li className="mudo">Sem registros ainda.</li>}
            {registros.map((r) => (
              <li key={r.id}>
                {dataHora(r.data)} · {r.usuario} · {r.acao}
                {r.armario !== "-" && ` · ${r.armario}`}
              </li>
            ))}
          </ul>
        </section>
      </div>
    </main>
  );
}
