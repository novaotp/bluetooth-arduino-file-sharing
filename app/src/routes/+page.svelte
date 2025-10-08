<script lang="ts">
    import { button } from "@sveltique/components/button";
</script>

<svelte:head>
    <title>Home - BAFS</title>
</svelte:head>

<main class="relative flex flex-col justify-center items-center gap-6 p-6 w-full h-full">
    <h1 class="font-bold text-4xl">BAFS</h1>
    <p class="text-center text-balance">Un système de partage de fichiers via le Bluetooth et des Arduinos</p>
    <div class="relative flex flex-col gap-3 w-full">
        <a href="/send" class={button()}>Envoyer</a>
        <a href="/receive" class={button()}>Recevoir</a>
    </div>
</main>
