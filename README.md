# Geregelte Probenheizung

Firmware für eine **geregelte Heizpatrone mit Touchscreen-Bedienung**, entwickelt für ein ESP32-Mikrocontrollerboard. Das System misst die Ist-Temperatur über einen Pt1000-Widerstandssensor, vergleicht sie mit einer über Touchscreen einstellbaren Soll-Temperatur und regelt die Heizpatrone entsprechend – inklusive Statusanzeige, akustischem Signal und Speicherung des Sollwerts im EEPROM.

Das Projekt entstand im Rahmen einer IPA (Individuelle Praktische Arbeit).

## Funktionsweise

Die Firmware läuft auf einem ESP32 und nutzt dessen zwei Kerne parallel (FreeRTOS-Tasks):

- **Kern 0** liest kontinuierlich die Ist-Temperatur über den ADC (MCP3426) aus.
- **Kern 1** kümmert sich um Touchscreen-Eingaben, Display-Updates und die eigentliche Regelungslogik (Zustandsautomat).

### Temperaturmessung

Die Temperatur wird über einen **Pt1000-Widerstandssensor** ermittelt. Aus der gemessenen Spannung wird über eine Callendar-Van-Dusen-Näherung (Konstanten `A`, `B`, `RNULL`) die Temperatur berechnet und anschliessend mit einer aus einer Systemkalibrierung gewonnenen Korrekturfunktion feinjustiert.

### Regelung (Zustandsautomat)

Die Heizpatrone wird über folgende Zustände gesteuert:

| Zustand | Beschreibung |
|---|---|
| `EVALUATE` | Ermittelt anhand der Temperaturdifferenz, in welchen Modus gewechselt wird |
| `HEATING` | Volle Heizleistung, bis die berechnete Abschalttemperatur (inkl. Nachlauf/Überschwingen) erreicht ist |
| `SOFTHEATING` | Reduzierte Leistung (ca. 20 % PWM) für sanftes Annähern an die Solltemperatur |
| `COOLING` | Heizpatrone aus, da Ist- über Solltemperatur liegt |
| `REGULATING` | Hält die Solltemperatur mittels PWM (10–20 % Leistung, abhängig vom Temperaturbereich) |
| `STANDBY` | Heizpatrone deaktiviert |

Die Abschalttemperatur beim Aufheizen wird über eine quadratische Näherungsfunktion (`AKONST`, `BKONST`, `CKONST`) so berechnet, dass das thermische Überschwingen der Heizpatrone möglichst genau auf der Solltemperatur landet.

### Bedienung

- Ein resistiver **Touchscreen** dient zur Eingabe:
  - Zwei Pfeiltasten zum Erhöhen/Verringern der Solltemperatur (Bereich 20–100 °C)
  - Eine START/STOPP-Taste zum Aktivieren/Deaktivieren der Regelung
- Ein **Farbdisplay** (HX8357, angesteuert über Adafruit-GFX-Bibliotheken) zeigt Soll- und Ist-Temperatur, eine Status-Ampel (rot = nicht geregelt, grün = im Sollbereich) sowie den Start/Stopp-Status.
- Ein **Buzzer** gibt bei Tastendruck sowie beim Erreichen der Regelung ein akustisches Signal.
- Die zuletzt eingestellte Solltemperatur wird im **EEPROM** gespeichert und beim Neustart automatisch geladen.

## Hardware

| Komponente | Beschreibung |
|---|---|
| Mikrocontroller | ESP32 (Dual-Core, FreeRTOS) |
| Temperatursensor | Pt1000, ausgelesen über MCP3426 (16-Bit-ADC via I²C) |
| Display | Adafruit HX8357 TFT-Farbdisplay |
| Touch-Eingabe | Resistiver Touchscreen (`TouchScreen`-Bibliothek) |
| Aktor | Heizpatrone, geschaltet über Digitalpin (Pin 33) |
| Signalgeber | Buzzer (Pin 27) |

### Pinbelegung (Auszug)

- **TFT:** CS = 21, DC = 14, RST = nicht verbunden
- **Touchscreen:** YP = A5, XM = A1, YM = 15, XP = 32
- **Heizpatrone:** Pin 33
- **Buzzer:** Pin 27

## Projektstruktur

```
Geregelte_Probenheizung/
├── Geregelte_Probenheizung.ino   Hauptprogramm: Setup, Regelungslogik, Zustandsautomat
├── Display.h / Display.cpp       GUI-Aufbau und Display-Updates (HX8357/Adafruit-GFX)
├── MCP3426.h / MCP3426.cpp       Ansteuerung des ADC zur Temperaturmessung
├── Buzzer.h / Buzzer.cpp         Ansteuerung des akustischen Signalgebers
├── debug.cfg                     OpenOCD-Konfiguration für JTAG-Debugging (ESP32-WROVER-KIT)
├── debug_custom.json             Debug-Konfiguration (z. B. für PlatformIO/VS Code)
└── esp32.svd                     Peripherie-Beschreibung des ESP32 für den Debugger
```

## Benötigte Bibliotheken

Zum Kompilieren mit der Arduino-IDE bzw. PlatformIO werden folgende Bibliotheken benötigt:

- `Adafruit_GFX`
- `Adafruit_HX8357`
- `TouchScreen` (Adafruit)
- `MCP342x`
- `Wire` (im ESP32-Core enthalten)
- `EEPROM` (im ESP32-Core enthalten)
- ESP32-Board-Unterstützung (Arduino-ESP32-Core) inkl. FreeRTOS

## Hinweis zur Kalibrierung

Die in `Geregelte_Probenheizung.ino` verwendeten Konstanten (`A`, `B`, `RNULL`, `AKONST`, `BKONST`, `CKONST`) stammen aus einer spezifischen **Pt1000- bzw. Systemkalibrierung** des verwendeten Aufbaus. Bei Verwendung eines anderen Sensors, einer anderen Heizpatrone oder eines abweichenden Messaufbaus müssen diese Werte neu ermittelt werden, damit Temperaturmessung und Überschwing-Kompensation korrekt funktionieren.
