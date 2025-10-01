<script lang="ts">
    import { BleClient, numberToUUID } from '@capacitor-community/bluetooth-le';
    import { onMount } from 'svelte';

    const service = numberToUUID(0x1800)
    const deviceName = numberToUUID(0x2A00)

    let message = $state("")

    onMount(async () => {
        await BleClient.initialize();
    })

    async function test() {
        const device = await BleClient.requestDevice();
        await BleClient.connect(device.deviceId);

        const result = await BleClient.read(device.deviceId, service, deviceName);

        message = Buffer.from(result.buffer).toString()
    }
</script>

<h1>Welcome to SvelteKit</h1>
<p>Visit <a href="https://svelte.dev/docs/kit">svelte.dev/docs/kit</a> to read the documentation</p>
<button onclick={test}>request device</button>

<hr />
<p>{message}</p>
