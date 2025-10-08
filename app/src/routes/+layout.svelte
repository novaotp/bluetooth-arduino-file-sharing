<script lang="ts">
    import { BleClient } from '@capacitor-community/bluetooth-le';
    import { onMount } from 'svelte';
	import favicon from '$lib/assets/favicon.svg';
	import '../app.css';
	
	let { children } = $props();

    onMount(async () => {
        try {
            await BleClient.initialize();
        } catch {
            // Bluetooth is unavailable on device
        }
    })
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

{@render children?.()}
