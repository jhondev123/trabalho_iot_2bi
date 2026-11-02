"use client";

import { useState } from "react";
import { useRouter } from "next/navigation";

export default function Login() {
  const router = useRouter();
  const [usuario, setUsuario] = useState("");
  const [senha, setSenha] = useState("");
  const [erro, setErro] = useState("");
  const [enviando, setEnviando] = useState(false);

  async function entrar(e: React.FormEvent) {
    e.preventDefault();
    setErro("");
    setEnviando(true);
    try {
      const r = await fetch("/api/login", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ usuario, senha }),
      });
      const corpo = await r.json().catch(() => ({}));
      if (!r.ok) return setErro(corpo.erro ?? "Não foi possível entrar.");
      router.push(corpo.papel === "admin" ? "/admin" : "/");
      router.refresh();
    } catch {
      setErro("Sem conexão com o servidor. Tente de novo.");
    } finally {
      setEnviando(false);
    }
  }

  return (
    <main className="conteudo estreito">
      <h1>Armário Inteligente</h1>
      <form className="cartao" onSubmit={entrar}>
        <label className="campo">
          Usuário
          <input value={usuario} onChange={(e) => setUsuario(e.target.value)} autoComplete="username" autoFocus />
        </label>
        <label className="campo">
          Senha
          <input type="password" value={senha} onChange={(e) => setSenha(e.target.value)} autoComplete="current-password" />
        </label>
        {erro && <p className="aviso">{erro}</p>}
        <button disabled={enviando || !usuario || !senha}>{enviando ? "Entrando..." : "Entrar"}</button>
      </form>
    </main>
  );
}
