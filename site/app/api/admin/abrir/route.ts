import { NextResponse } from "next/server";
import mqtt from "mqtt";
import { getSessao } from "@/lib/sessao";
import { salvarRegistro } from "@/lib/registros";
import { CODIGO_VALIDO, topico } from "@/lib/mqtt-config";

export const runtime = "nodejs";

// O admin pede para abrir um armário. Quem publica o comando no MQTT é o servidor,
// assim a senha de admin do ESP32 fica nas variáveis de ambiente e não vai para o navegador.
export async function POST(req: Request) {
  const sessao = await getSessao();
  if (!sessao || sessao.papel !== "admin") {
    return NextResponse.json({ erro: "Só o admin pode abrir." }, { status: 403 });
  }

  const corpo = await req.json().catch(() => null);
  const armario = String(corpo?.armario ?? "").toUpperCase();
  if (!CODIGO_VALIDO.test(armario)) {
    return NextResponse.json({ erro: "Código de armário inválido." }, { status: 400 });
  }

  const url = process.env.MQTT_URL_SERVIDOR ?? "mqtt://broker.hivemq.com:1883";
  const senha = process.env.ADMIN_SENHA_ARMARIO ?? "admin123";

  let cliente: mqtt.MqttClient | undefined;
  try {
    cliente = await mqtt.connectAsync(url, { connectTimeout: 8000, reconnectPeriod: 0 });
    await cliente.publishAsync(topico(armario, "cmd"), JSON.stringify({ acao: "abrir", admin: senha }));
  } catch {
    return NextResponse.json({ erro: "Não consegui falar com o broker MQTT." }, { status: 502 });
  } finally {
    await cliente?.endAsync().catch(() => {});
  }

  await salvarRegistro({ usuario: sessao.usuario, papel: sessao.papel, armario, acao: "admin_abrir" });
  return NextResponse.json({ ok: true });
}
