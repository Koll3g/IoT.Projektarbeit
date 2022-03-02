# IoT Projektarbeit
Repository zur Projektarbeit im Workshop IoT

## Contributors
- Corina Widmer
- Kevin Heeb
- Liam Ormond
- Angelo Lütolf

##Tools
- Alle "Node" Projekte basieren auf Embedded C und C++
- Empfohlene Entwicklungsumgebung: VS Code mit PlatformIO

## Structure
### Bewässerungseinheit
#### WaterValveNode
- Wasserventil zur Steuerung der Wasserzufuhr basierend auf Soll- und Istwert der entsprechenden Bewässerungseinheit
- Falls kein Sollwert von der Cloud verfügbar ist, wird ein Wert von 50% angenommen (Ausfallsicherheit)
- Board: esp32dev
- Sparkfun LED Stick simuliert öffnen / schliessen des Ventils

####SoilMoistureNode
- Bodenfeuchtigkeitssensor zum Messen der Bodenfeuchtigkeit der entsprechenden Bewässerungseinheit
- Board: esp32dev
- Sparkfun Moisture Sensor wird zum Messen der Bodenfeuchtigkeit verwendet
