include .vscode/default.mk

# Mit 'CFLAGS' können zusätzliche Optionen, für den Bau des Programms angegeben werden.
# Durch die Option '-g' werden zusätzliche Informationen für das Debugging erzeugt.
# Die Option '-O' ermöglicht die Angabe der Optimierungsstufe.
# Der Wert '0' bedeutet in diesem Fall keine Optimierung angewendet wird.
CFLAGS += -ggdb -O0

# Die Zielfiles (`.elf`) sind von den Objektdateien (`.o`) abhängig,
# welche sich im Verzeichnis `build/` befinden. Die Quelldateien (`.c`)
# liegen im Verzeichnis `src/`. Daher müssen die Abhängigkeiten im Format `build/*.o`
# angegeben werden, um die implizite Regel zur Erstellung der Objektdateien aus den Quelldateien
# zu nutzen (definiert in `default.mk`).

# Um `TerminXAufgabeY` zu erstellen, benötigen wir `TerminXAufgabeY.o`, welches aus `src/TerminXAufgabeY.c` erstellt wird.
build/main.elf: build/main.o
