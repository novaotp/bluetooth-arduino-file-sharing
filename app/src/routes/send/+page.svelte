<script lang="ts">
    /* import { BleClient, numberToUUID } from '@capacitor-community/bluetooth-le'; */
    import { Button, button } from '@sveltique/components/button';
    import { Field } from '@sveltique/components/field';
    import { FileInput } from '@sveltique/components/file-input';
    import { Progress } from '@sveltique/components/progress';
    import { onMount, untrack } from 'svelte';

    // Update based on Arduino
    const BYTES_PER_CHUNK = 128;

    type SendStatus = "loading" | "ready" | "sending" | "busy" | "error";

    let arduinoConnected = $state(false) 
    let arduinoReady = $state(false)

    let canSend = $state<SendStatus>("loading")
    let readyForNextChunk = $state(true)
    let isLastChunk = $derived.by(() => {
        if (!fileBytes) return false;

        const chunks = Math.ceil(fileBytes.byteLength / BYTES_PER_CHUNK);
        return fileCursorIndex >= chunks - 1;
    })

    let file = $state<File>();
    let fileBytesPromise = $derived(getFileAsBytes());
    let fileBytes = $derived(await fileBytesPromise);
    let fileCursorIndex = $state(0);
    let fileError = $derived.by(() => {
        return file && getSizeKB(file) > 1 ? "File too big" : undefined;
    });

    let progressValue = $derived.by(() => {
        if (!fileBytes) return 0;
        
        const chunks = Math.ceil(fileBytes.byteLength / BYTES_PER_CHUNK);
        return Math.min(Math.round(((fileCursorIndex + 1) / chunks) * 100), 100);
    })
    
    function getSizeKB(file: File): number {
        return file.size / 1024;
    }

    async function updateSendStatus(): Promise<void> {
        // TODO : Ask the arduino if there is another device
        // If yes, show form to select
        // Else, show error message and retry button

        // Simulate wait
        await new Promise<void>((resolve) => {
            setTimeout(() => {
                resolve();
            }, 1500);
        });

        canSend = "ready";
    }

    async function startSendProcess() {
        if (!fileBytes) return;

        canSend = "sending";

        console.log("Starting send process")

        // TODO : Send file size

        await streamFileToArduino(fileCursorIndex++, false);
    }

    async function streamFileToArduino(cursor: number, isLast: boolean) {
        if (!fileBytes || !readyForNextChunk) return;
        if (cursor * BYTES_PER_CHUNK >= fileBytes.byteLength) return;
        
        canSend = "busy";
        readyForNextChunk = false;

        const chunk = fileBytes.slice(cursor * BYTES_PER_CHUNK, (cursor + 1) * BYTES_PER_CHUNK);
        console.log("[START] Sending chunk to Arduino")

        // TODO: send chunk
        await new Promise(r => setTimeout(r, 100)); // simulate latency

        readyForNextChunk = true;
        canSend = "sending";
        if (isLast) {
            canSend = "ready";
            console.log("[END] Last chunk sent")
        }
    }

    async function listenToArduino() {
        while (arduinoConnected) {
            if (arduinoReady && readyForNextChunk) {
                await streamFileToArduino(fileCursorIndex++, isLastChunk);
            }
            
            await new Promise(r => setTimeout(r, 50));
        }
    }
    
    async function getFileAsBytes() {
        if (!file) return;
        
        return new Uint8Array(await file.arrayBuffer());
    }
    
    onMount(updateSendStatus);
    $effect(() => {
        if (!arduinoConnected) return;

        if (canSend === "sending" && readyForNextChunk) {
            untrack(listenToArduino);
        }
    });

    onMount(() => {
        arduinoConnected = true;
        arduinoReady = true;
    });

    /* const service = numberToUUID(0x1800)
    const deviceName = numberToUUID(0x2A00)

    let message = $state("")

    async function test() {
        const device = await BleClient.requestDevice();
        await BleClient.connect(device.deviceId);

        const result = await BleClient.read(device.deviceId, service, deviceName);

        message = Buffer.from(result.buffer).toString()
    } */
</script>

<svelte:head>
    <title>Envoi de fichiers - BAFS</title>
</svelte:head>

<div class="relative flex flex-col justify-center items-center w-full h-full">
    <header class="relative flex justify-between items-center p-6 w-full">
        <a href="/" class={button({ variant: "text" })}>
            <svg
                xmlns="http://www.w3.org/2000/svg"
                width="24"
                height="24"
                viewBox="0 0 24 24"
                fill="none"
                stroke="currentColor"
                stroke-width="2"
                stroke-linecap="round"
                stroke-linejoin="round"
                class="icon-tabler icon-tabler-arrow-narrow-left icons-tabler-outline icon"
            >
                <path stroke="none" d="M0 0h24v24H0z" fill="none" />
                <path d="M5 12l14 0" />
                <path d="M5 12l4 4" />
                <path d="M5 12l4 -4" />
            </svg>
            <span>Retour</span>
        </a>
    </header>
    <div class="relative flex flex-col justify-start items-center gap-6 p-6 w-full h-full">
        <h1 class="font-bold text-4xl">Envoi</h1>
        {#if canSend === "loading"}
            <p>Vérification...</p>
        {:else if canSend === "ready"}
            <p class="text-center text-balance">Choisissez un fichier à envoyer.</p>
            <Field label="Fichier" helper="Taille max de 1 KO" error={fileError}>
                {#snippet input({ props })}
                    <FileInput bind:file {...props} />
                {/snippet}
            </Field>
            <Button onclick={startSendProcess}>Envoyer</Button>
        {:else if canSend === "sending"}
            <Field label="Progrès de l'envoi">
                {#snippet input({ props })}
                    <Progress value={progressValue} {...props} />
                {/snippet}
            </Field>
        {:else}
            <p>Impossible d'envoyer pour le moment.</p>
        {/if}
    </div>
</div>
