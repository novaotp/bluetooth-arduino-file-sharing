import { sveltekit } from "@sveltejs/kit/vite";
import { internalIpV4 } from "internal-ip";
import { defineConfig } from "vite";

// @ts-expect-error process is a nodejs global
const mobile = true /* !!/android|ios/.exec(process.env.TAURI_ENV_PLATFORM) */;

// https://vite.dev/config/
export default defineConfig(async () => ({
    plugins: [sveltekit()],
    // 1. prevent Vite from obscuring rust errors
    clearScreen: false,
    // 2. tauri expects a fixed port, fail if that port is not available
    server: {
        port: 1420,
        strictPort: true,
        host: mobile ? "0.0.0.0" : false,
        hmr: mobile
            ? {
                  protocol: "ws",
                  host: await internalIpV4(),
                  port: 1421
              }
            : undefined,
        watch: {
            // 3. tell Vite to ignore watching `src-tauri`
            ignored: ["**/src-tauri/**"]
        }
    }
}));
