import { promises as fs } from "fs";
import os from "os";
import path from "path";
import { randomUUID } from "crypto";

// Registros salvos num arquivo JSON (data/registros.json).
//
// ATENÇÃO na Vercel: o disco do projeto é somente leitura e o que é escrito em /tmp
// some quando a função "dorme" ou quando a Vercel usa outra instância. Então lá os
// registros funcionam para a demo, mas não são permanentes. Rodando local (npm run dev)
// o arquivo data/registros.json guarda tudo de verdade.

export type Registro = {
  id: string;
  data: string; // ISO 8601
  usuario: string;
  papel: string;
  armario: string; // "-" quando não se aplica (ex.: login)
  acao: string; // login | parear | abrir | admin_abrir
  detalhe?: string;
};

const ARQUIVO = process.env.VERCEL
  ? path.join(os.tmpdir(), "registros.json")
  : path.join(process.cwd(), "data", "registros.json");

const LIMITE = 500; // guarda só os últimos 500 para o arquivo não crescer sem fim

export async function lerRegistros(): Promise<Registro[]> {
  try {
    const texto = await fs.readFile(ARQUIVO, "utf-8");
    const lista = JSON.parse(texto);
    return Array.isArray(lista) ? lista : [];
  } catch {
    return []; // arquivo ainda não existe ou está vazio
  }
}

export async function salvarRegistro(dados: Omit<Registro, "id" | "data">) {
  const registro: Registro = { id: randomUUID(), data: new Date().toISOString(), ...dados };
  const lista = await lerRegistros();
  lista.push(registro);
  await fs.mkdir(path.dirname(ARQUIVO), { recursive: true });
  await fs.writeFile(ARQUIVO, JSON.stringify(lista.slice(-LIMITE), null, 2), "utf-8");
  return registro;
}
