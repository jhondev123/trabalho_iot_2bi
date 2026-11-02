import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Armário Inteligente",
  description: "Controle de armários inteligentes com ESP32 e MQTT",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="pt-BR">
      <body>{children}</body>
    </html>
  );
}
