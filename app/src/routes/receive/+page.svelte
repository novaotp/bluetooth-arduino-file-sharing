<script lang="ts">
    /* import { BleClient } from '@capacitor-community/bluetooth-le'; */
    import { Button, button } from '@sveltique/components/button';
    import { Field } from '@sveltique/components/field';
    import { Progress } from '@sveltique/components/progress';
    import { onMount } from 'svelte';

    const BYTES_PER_CHUNK = 128;

    type ReceiveStatus = "loading" | "waiting" | "receiving" | "done" | "error";

    let arduinoConnected = $state(false);
    let arduinoReady = $state(false);

    let receiveStatus = $state<ReceiveStatus>("loading");
    let expectedFileSize = $state(0);
    let receivedBytes = $state<Uint8Array>(new Uint8Array());
    let receivedChunks = $state(0);

    let progressValue = $derived.by(() => {
        if (!expectedFileSize) return 0;
        return Math.min(Math.round((receivedBytes.byteLength / expectedFileSize) * 100), 100);
    });

    onMount(async () => {
        await updateConnectionStatus();
        if (arduinoConnected && arduinoReady) listenToArduino();
    });

    async function updateConnectionStatus() {
        // TODO : connect to arduino
        await new Promise(r => setTimeout(r, 1000));

        arduinoConnected = true;
        arduinoReady = true;
        receiveStatus = "waiting";
    }

    async function listenToArduino() {
        console.log("Listening for file from Arduino...");
        while (arduinoConnected) {
            if (receiveStatus === "waiting") {
                const fileSize = await waitForArduinoFileSize();
                expectedFileSize = fileSize;
                receiveStatus = "receiving";
            }

            if (receiveStatus === "receiving") {
                const chunk = await receiveChunkFromArduino();
                receivedBytes = concat(receivedBytes, chunk);
                receivedChunks++;
                if (receivedBytes.byteLength >= expectedFileSize) {
                    receiveStatus = "done";
                    break;
                }
            }

            await new Promise(r => setTimeout(r, 50));
        }
    }

    async function waitForArduinoFileSize(): Promise<number> {
        // TODO: listen for size message from Arduino
        await new Promise(r => setTimeout(r, 500));

        return 1024; // simulated 1 KB file
    }

    async function receiveChunkFromArduino(): Promise<Uint8Array> {
        // TODO: read chunk over BLE
        await new Promise(r => setTimeout(r, 100));

        return new Uint8Array(BYTES_PER_CHUNK).fill(65); // 'A' bytes
    }

    function concat(a: Uint8Array, b: Uint8Array): Uint8Array {
        const result = new Uint8Array(a.length + b.length);
        result.set(a, 0);
        result.set(b, a.length);
        return result;
    }

    function downloadReceivedFile() {
        const blob = new Blob([Buffer.from(receivedBytes)], { type: "application/octet-stream" });
        const url = URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "arduino_received.bin";
        a.click();
        URL.revokeObjectURL(url);
    }
</script>

<svelte:head>
    <title>Réception de fichiers - BAFS</title>
</svelte:head>

<div class="relative flex flex-col justify-center items-center w-full h-full">
    <header class="relative flex justify-between items-center p-6 w-full">
        <a href="/" class={button({ variant: "text" })}>
            <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24"
                viewBox="0 0 24 24" fill="none" stroke="currentColor"
                stroke-width="2" stroke-linecap="round" stroke-linejoin="round"
                class="icon-tabler icon-tabler-arrow-narrow-left">
                <path stroke="none" d="M0 0h24v24H0z" fill="none" />
                <path d="M5 12l14 0" />
                <path d="M5 12l4 4" />
                <path d="M5 12l4 -4" />
            </svg>
            <span>Retour</span>
        </a>
    </header>

    <div class="relative flex flex-col justify-start items-center gap-6 p-6 w-full h-full">
        <h1 class="font-bold text-4xl">Réception</h1>

        {#if receiveStatus === "loading"}
            <p>Vérification...</p>

        {:else if receiveStatus === "waiting"}
            <p>En attente d'un fichier depuis l'Arduino...</p>

        {:else if receiveStatus === "receiving"}
            <Field label="Réception en cours">
                {#snippet input({ props })}
                    <Progress value={progressValue} {...props} />
                {/snippet}
            </Field>

        {:else if receiveStatus === "done"}
            <p>Fichier reçu avec succès.</p>
            <Button onclick={downloadReceivedFile}>Télécharger</Button>

        {:else}
            <p>Erreur pendant la réception.</p>
        {/if}
    </div>
</div>
