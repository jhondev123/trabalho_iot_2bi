"use client";

import { useRouter } from "next/navigation";

export default function Cabecalho({ usuario, papel }: { usuario: string; papel: string }) {
  const router = useRouter();

  async function sair() {
    await fetch("/api/logout", { method: "POST" });
    router.push("/login");
    router.refresh();
  }

  return (
    <header className="cabecalho">
      <strong>Armário Inteligente</strong>
      <span className="direita">
        {usuario} ({papel}) <button className="link" onClick={sair}>Sair</button>
      </span>
    </header>
  );
}
