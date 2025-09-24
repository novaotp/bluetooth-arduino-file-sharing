#set page(footer: context [
  #if counter(page).get().first() > 1 [
    Bluetooth Arduino File Sharing
    #h(1fr)
    #counter(page).display("1")
  ]
])

#set text(
  font: ("Calibri", "Bookman Old Style"),
)

#show heading.where(level: 1): it => [
  #set block(below: 1em)
  #set text(size: 24pt)

  #it
]

#show heading.where(level: 2): it => [
  #set block(above: 1.4em, below: 1em)
  #set text(size: 18pt)

  #it
]

#show heading.where(level: 3): set text(size: 16pt)

#set text(size: 12pt)

#align(center + horizon)[
  #heading(level: 1, outlined: false)[Bluetooth Arduino File Sharing]

  #[
    #set text(size: 14pt);

    Un système de partage de fichiers via des arduinos.
  ]

  #block(above: 3cm)[
    #set text(size: 14pt);

    Sajidur Rahman

    Gabriel Vannay

    24.09.2025
  ]
]

#pagebreak()

#outline(
  title: [Table des matières],
  indent: n => (n - 1) * 1.4em,
)

#pagebreak()

== Introduction

Un système de partage de fichiers entre téléphones, basé sur des cartes Arduino, utilisant les technologies Bluetooth et Ethernet.

== Besoins & Objectifs

- Lire et écrire des fichiers depuis un appareil mobile
- Transférer et recevoir des fichiers entre un mobile et un Arduino
- Échanger des données entre deux Arduinos

== Cahier des charges

Un système de partage de fichiers entre téléphones, basé sur l’utilisation de cartes Arduino, permettrait de faciliter le transfert de données entre deux départements de nos bureaux à Sion.

En raison de problèmes de connectivité Wi-Fi, le recours à la technologie Bluetooth offrirait une solution sans fil efficace pour l’échange de fichiers entre appareils mobiles, sans nécessiter de connexion à Internet.

Concrètement, un Arduino équipé d’un module Bluetooth pourrait assurer l’envoi et la réception des fichiers à partir d’un téléphone. De son côté, un second Arduino connecté au réseau local via Ethernet agirait comme une passerelle, permettant de relayer les fichiers vers d’autres appareils.

Ce dispositif hybride tirerait parti des avantages respectifs du Bluetooth (mobilité, simplicité) et de l’Ethernet (stabilité, rapidité), tout en contournant les limitations actuelles de l'infrastructure réseau.

=== Matériel

- 2 cartes Arduino Uno
  - Module HM-10 BLE Bluetooth 4.0
  - Shield Ethernet
- 2 câble Ethernet
- 2 câbles USB-A vers USB-B
- 1 Firewall Routeur

=== Livrables

- Une application mobile permettant le transfert et la réception de fichiers
- Un programme Arduino permettant l’échange de fichiers entre deux cartes

=== Durée

21 heures

#pagebreak()

=== Prix

#table(
  columns: (2fr, 1fr, 1.5fr, 1fr),
  inset: 10pt,
  align: horizon,
  table.header([*Objet*], [*Prix unitaire*], [*Quantité*], [*Total*]),
  [Cartes Arduino], [30 CHF], [2], [60 CHF],
  [Shield Ethernet], [40 CHF], [2], [80 CHF],
  [Module HM-10], [30 CHF], [2], [60 CHF],
  [Câble USB-A vers USB-B], [5 CHF], [2], [10 CHF],
  [Câble Ethernet], [40 CHF], [2], [80 CHF],
  [Firewall Routeur], [250 CHF], [1], [250 CHF],
  [Développeur], [50 CHF], [21], [1050 CHF],
  table.cell(colspan: 3, [*Total*]), [1590 CHF],
)
