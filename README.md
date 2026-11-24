# Práctica 5 · Distancia por infrarrojos

ESP32-DevKitC y Sharp GP2Y0A41SK0F. Montaje de la práctica: VCC a 5 V, GND común y Vo a GPIO34 (ADC1_CH6). La conversión supone conexión directa, sin divisor.

En una terminal ESP-IDF, desde esta carpeta:

    idf.py menuconfig
    idf.py build
    idf.py -p PUERTO flash monitor

Sustituye PUERTO por el puerto USB real. En «Practica 5» se seleccionan las lecturas por medida (16 por defecto). Cada segundo el componente promedia las tensiones calibradas y el programa muestra la distancia al recibir el evento.

Se aproxima la curva mediante d(cm) = 12320 / (V(mV) - 40), ajustada a los dos ejemplos del enunciado (8 cm y 22 cm). Verifica con papel blanco y regla a 8, 15 y 22 cm. El intervalo útil es 4–30 cm; por debajo de 4 cm la tensión puede parecer válida. La fórmula es una aproximación y puede perder precisión cerca de los extremos.
