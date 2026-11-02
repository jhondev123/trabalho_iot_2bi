// Logins fixos em código (só para a demo da faculdade).
// Para trocar uma senha, edite aqui e suba de novo.

export type Papel = "aluno" | "admin";

export const USUARIOS: { usuario: string; senha: string; papel: Papel }[] = [
  { usuario: "aluno", senha: "aluno123", papel: "aluno" },
  { usuario: "admin", senha: "admin123", papel: "admin" },
];

export function validarLogin(usuario: string, senha: string) {
  return USUARIOS.find((u) => u.usuario === usuario && u.senha === senha) ?? null;
}
