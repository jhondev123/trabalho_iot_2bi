"use client";

import { useEffect, useState } from "react";
import { useArmarios } from "@/lib/useArmarios";
import { CODIGO_VALIDO } from "@/lib/mqtt-config";
import EstadoBadge from "./EstadoBadge";

const chaveToken = (id: string) => `armario:${id}`;

async function registrar(armario: string, acao: "parear" | "abrir") {
  await fetch("/api/registros", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ armario, acao }),
  }).catch(() => {});
}

export default function PainelAluno() {
  const { armarios, eventos, conectado, enviar } = useArmarios();
  const [codigo, setCodigo] = useState("A07");
  const [aviso, setAviso] = useState("");
  const [temToken, setTemToken] = useState(false);

  const id = codigo.trim().toUpperCase();
  const valido = CODIGO_VALIDO.test(id);
  const armario = valido ? armarios[id] : undefined;
  const estado = armario?.estado;

  // Lembra o último código usado neste celular.
  useEffect(() => {
    const salvo = localStorage.getItem("armario:ultimo");
    if (salvo) setCodigo(salvo);
  }, []);

  // Confere se este navegador tem o token do armário escolhido.
  // Regra do desenho: se o armário aparece LIVRE, o token antigo não vale mais e é apagado.
  useEffect(() => {
    if (!valido) return setTemToken(false);
    if (estado === "livre") localStorage.removeItem(chaveToken(id));
    setTemToken(!!localStorage.getItem(chaveToken(id)));
  }, [id, valido, estado]);

  function escolher(valor: string) {
    setCodigo(valor);
    setAviso("");
    if (CODIGO_VALIDO.test(valor.trim())) localStorage.setItem("armario:ultimo", valor.trim().toUpperCase());
  }

  function usar() {
    const token = crypto.randomUUID();
    localStorage.setItem(chaveToken(id), token);
    if (!enviar(id, { acao: "parear", token })) {
      localStorage.removeItem(chaveToken(id));
      return setAviso("Sem conexão com o broker. Tente de novo em instantes.");
    }
    registrar(id, "parear");
    setTemToken(true);
    setAviso("Pedido enviado. Quando o armário ficar Pareado, guarde suas coisas e feche a porta.");
  }

  function abrir() {
    const token = localStorage.getItem(chaveToken(id));
    if (!token) return;
    if (!enviar(id, { acao: "abrir", token })) return setAviso("Sem conexão com o broker. Tente de novo em instantes.");
    registrar(id, "abrir");
    setAviso("Comando de abrir enviado. Abra a porta em até 10 segundos.");
  }

  // Avisos do próprio armário escolhido (token errado, bloqueio...).
  const ultimoEvento = eventos.find((e) => e.armario === id && Date.now() - e.hora < 15000);

  return (
    <main className="conteudo">
      <h1>Meu armário</h1>

      <label className="campo">
        Código do armário (ex.: A07)
        <input value={codigo} onChange={(e) => escolher(e.target.value)} maxLength={3} placeholder="A07" />
      </label>
      {!valido && <p className="aviso">Use uma letra e dois números, como A07.</p>}

      {valido && (
        <section className="cartao">
          <div className="linha">
            <strong>Armário {id}</strong>
            <EstadoBadge armario={armario} />
          </div>

          {!armario?.estado && (
            <p className="mudo">
              {conectado
                ? "Esse armário ainda não apareceu. Confira o código e se o armário está ligado."
                : "Conectando ao broker..."}
            </p>
          )}
          {armario?.online === false && <p className="aviso">O armário está offline no momento.</p>}

          {armario?.estado && armario.online !== false && (
            <p className="mudo">
              Porta {armario.porta} · trinco {armario.travado ? "travado" : "destravado"}
            </p>
          )}

          {estado === "livre" && armario?.online !== false && (
            <button onClick={usar}>Usar este armário</button>
          )}
          {(estado === "pareado" || estado === "ocupado") && temToken && armario?.online !== false && (
            <button onClick={abrir}>Abrir armário</button>
          )}
          {estado === "ocupado" && !temToken && (
            <p className="mudo">Este armário está em uso. Se for seu e você perdeu o acesso, procure a recepção.</p>
          )}
          {estado === "pareado" && !temToken && <p className="mudo">Outra pessoa está guardando algo neste armário.</p>}

          {ultimoEvento?.evento === "token_invalido" && (
            <p className="aviso">Token inválido ({ultimoEvento.tentativas}/3).</p>
          )}
          {ultimoEvento?.evento === "bloqueado" && (
            <p className="aviso">Armário bloqueado por 1 minuto por tentativas erradas.</p>
          )}
          {aviso && <p className="info">{aviso}</p>}
        </section>
      )}

      <p className="mudo pequeno">{conectado ? "Conectado ao broker" : "Desconectado do broker"}</p>
    </main>
  );
}
