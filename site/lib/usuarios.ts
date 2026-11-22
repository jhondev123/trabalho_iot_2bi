// Logins de demonstração (dois usuários fixos, sem banco de dados).
// As senhas vêm das variáveis de ambiente (.env.local ou Vercel), não do código.
// No npm run dev, sem as variáveis, valem as senhas de teste aluno123 / admin123.
// Em produção, sem a variável, o login daquele usuário fica desativado (de propósito).

export type Papel = "aluno" | "admin";

function senhaDe(variavel: string, senhaDeTeste: string): string | null {
  const senha = process.env[variavel];
  if (senha) return senha;
  return process.env.NODE_ENV === "production" ? null : senhaDeTeste;
}

function usuarios(): { usuario: string; senha: string | null; papel: Papel }[] {
  return [
    { usuario: "aluno", senha: senhaDe("SENHA_ALUNO", "aluno123"), papel: "aluno" },
    { usuario: "admin", senha: senhaDe("SENHA_ADMIN", "admin123"), papel: "admin" },
  ];
}

export function validarLogin(usuario: string, senha: string) {
  return usuarios().find((u) => u.senha !== null && u.usuario === usuario && u.senha === senha) ?? null;
}
