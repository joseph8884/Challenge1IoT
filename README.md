# **Challenge #1**

Sistema IoT para monitoreo temprano de deslizamientos de tierra usando un ESP32. Se integran sensores vía I2C y señales analógicas para monitorear vibración, lluvia, humedad de suelo y temperatura. La lógica de fusión calcula un nivel de riesgo y activa alertas visuales y sonoras.

- Objetivo: detectar cambios de inclinación y condiciones ambientales que indiquen riesgo de deslizamiento y notificar a tiempo.
- Plataforma: ESP32 (3.3 V), bus I2C compartido para sensores/actuadores compatibles y entradas analógicas para módulos que lo requieran.
- Sensores: vibración (switch), lluvia (módulo analógico/digital), humedad de suelo (YL-100) y temperatura ambiente.
- Actuadores: indicadores LED/pantalla I2C y buzzer piezoeléctrico.
- Comunicación interna: I2C + GPIO analógicos/digitales. 

<details>
<summary>

# **Lista de contenidos del proyecto**
</summary>

- Resumen General
- Motivación
- Justificación
- Solución propuesta
	- Restricciones de diseño
	- Arquitectura propuesta
	- Desarrollo técnico modular
	- Configuración experimental
- Resultados
- Conclusiones y trabajo futuro
- Anexos

</details>


<details>
<summary>

# **Resumen General**
</summary>

Se propone un sistema de monitoreo continuo para zonas con susceptibilidad a deslizamientos, como el propuesto en Tabio y Cajica. El ESP32 integra múltiples sensores para detectar inclinaciones del terreno, vibraciones anómalas y condiciones de humedad/lluvia que incrementan el riesgo. Con una lógica de fusión, el sistema clasifica el estado en Normal, Precaución, Alerta o Emergencia y activa actuadores (pantalla/LED y buzzer) para aviso local. El diseño prioriza bajo consumo, robustez y facilidad de despliegue.

</details>



<details>
<summary>

# **Motivación**
</summary>

- Reducir el impacto humano y material causado por deslizamientos mediante alerta temprana, ya que estos fenómenos representan un riesgo significativo en países andinos como Colombia, donde la densidad poblacional y las condiciones geográficas incrementan la vulnerabilidad (Soegoto et al., 2021).
- Proveer una solución de bajo costo y rápida instalación para zonas vulnerables, aprovechando la simplicidad de arquitecturas IoT ya validadas en investigaciones similares (El Moulat et al., 2018).
- Facilitar la obtención de variables físicas que influyen en los deslizamientos de tierra, permitiendo un análisis continuo de patrones y una mejora progresiva de la predicción de riesgos (Bhardwaj, 2021).

</details>



<details>
<summary>

# **Justificación**
</summary>

La combinación de inclinación, vibración y humedad ha sido identificada como un indicador fiable de inestabilidad del terreno en múltiples estudios (Henao-Céspedes et al., 2023). Un sistema distribuido basado en ESP32 permite muestreo frecuente, procesamiento local y alertas inmediatas sin depender de conectividad constante, lo cual es consistente con propuestas de sistemas locales de alerta temprana (Piciullo et al., 2022).  
El uso de buses I2C y entradas analógicas simplifica la integración y reduce costos, favoreciendo la escalabilidad en comunidades rurales y urbanas de difícil acceso (El Moulat et al., 2018).

</details>



<details>
<summary>

# **Solución propuesta**
</summary>

## **Tabla de umbrales propuestos**

Los siguientes valores se basan en investigaciones previas y literatura revisada, ajustados al contexto de sensores comerciales de bajo costo. Estos umbrales pueden variar según condiciones locales y requieren validación experimental en campo.

| Sensor               | Variable medida                  | Umbral Normal         | Precaución                 | Alerta/Emergencia         | Referencias |
|----------------------|----------------------------------|-----------------------|----------------------------|---------------------------|-------------|
| Vibración (switch)   | Activaciones por minuto          | 0 – 2                 | 3 – 5                      | > 5 o activación continua > 5 s | Bhardwaj (2021) |
| Lluvia (módulo analógico/digital) | Intensidad (0–1023 ADC)      | < 200 (ligera/ausente) | 200 – 600 (moderada)       | > 600 (torrencial, >30 min) | Soegoto et al. (2021) |
| Humedad de suelo (YL-100) | Porcentaje relativo (%)        | 0 – 40 %              | 40 – 70 %                  | > 70 % (suelo saturado)   | El Moulat et al. (2018), Piciullo et al. (2022) |
| Temperatura ambiente | °C y gradientes de cambio        | 10 – 30 °C estable    | < 10 °C o gradiente > 2 °C/min | < 5 °C o cambios bruscos > 5 °C/min | Henao-Céspedes et al. (2023) |

---

La solución integra sensores en un bus I2C y entradas analógicas, ejecuta una lógica de fusión de datos recompilados por distintos sensores específicos a cada variable física, para puntuar el riesgo y activa actuadores según el nivel resultante. Se contemplan módulos de adquisición, filtrado, decisión y notificación.

Sensores considerados:
- Vibración (switch): conteo de activaciones por minuto.
- Lluvia (módulo analógico/digital): intensidad y estado de lluvia.
- Humedad de suelo (YL-100): medición de humedad relativa.
- Temperatura ambiente: medición de temperatura y gradientes.

Actuadores considerados:
- Pantalla/indicadores LED (idealmente I2C u opcionalmente GPIO).
- Buzzer (GPIO/PWM) con distintos patrones según el nivel.

El detalle de parámetros y umbrales se encuentra en `ParametrosYsensores.md`.

## **Restricciones de diseño**

- Plataforma: ESP32 a 3.3 V; todos los sensores/actuadores deben ser compatibles o incluir nivelación adecuada.
- Robustez: operación estable en intemperie; protección contra humedad; pull-ups I2C adecuados;
- Latencia: detección y actualización de estado en segundos, con señales visuales o auditivas, con ventanas de suavizado para evitar falsos positivos.
- Costo: uso de módulos comerciales económicos y disponibilidad local.
- Usar solo dispositivos embebidos como (ESP32, Arduino, Intel galileo)

## **Arquitectura propuesta**

![Diagrama de alto nievel](/Images/Diagrama%20de%20alto%20nivel%20challenge.png)

Flujo de datos:
1) Obtencio de datos periódico de sensores 
2) Filtrado y cálculo de variables físicas.
3) Puntuación de riesgo por reglas y tabla de decisión.
4) Accionamiento de alertas locales y generación de eventos.

Notas de implementación:
- Evitar direcciones I2C en conflicto; documentar el escaneo de bus.
- Usar resistencias pull-up en SDA/SCL (típ. 4.7 kΩ) si no están en los módulos.
- Mantener cables I2C cortos o usar topología adecuada para ambientes ruidosos.

## **Desarrollo tecnico modular**
- Diagramas modulares
- Diagrama de flujo
- Esquematico de hardware desarrollado 
- Estandares de diseño de ingenieria aplicados

Módulos propuestos:
![Diagrama animado](/Images/Diagrama%20animado.png)

- Adquisición de datos: drivers I2C/ADC, temporización de muestreo.
- Fusión/decisión: reglas por umbral.
- Alertas: control de LED/pantalla y patrones de buzzer.

Diagrama de flujo (general):

![Diagrama de flujo](/Images/Diagrama%20de%20flujo.png)

1) Inicio.
2) Lectura de vibración + lluvia + humedad + temperatura.
3) Filtrado y cálculo de indicadores (activaciones/min, % humedad, intensidad lluvia).
4) Cálculo de puntaje de riesgo y mapeo a estado.
5) Actualizar actuadores y notificar evento si cambia el estado.

Diagrama de flujo de algoritmo avanzado para detección de deslizamientos. 

## **Configuracion experimental**

Objetivo: validar umbrales y la matriz de decisión reduciendo falsos positivos/negativos.

Escenarios de prueba:
- Vibración: pulsos mecánicos de distinta frecuencia y duración; prueba de activación continua > 5 s.
- Lluvia: simulación de intensidades (seco→torrencial) y persistencia > 30 min.
- Humedad de suelo: transición de seco→saturado y combinación con lluvia.
- Temperatura: pruebas en rangos bajos (<5 °C) y cambios rápidos (si el sensor disponible lo permite).

Métricas:
- Tiempo de detección por nivel (s), tasa de falsas alarmas, estabilidad del estado, consumo promedio.

Notas:
- Calibración inicial: valores base de suelo seco y nivel cero de inclinación en reposo.
- Registrar series temporales para análisis posterior.

</details>


<details>
<summary>

# **Resultados**
- Analisis
</summary>

No se incluyen mediciones definitivas en esta versión. Propuesta de reporte:
- Tabla con tiempos de reacción por escenario y nivel.
- Curva de vibración (activaciones/min) vs. estado.
- Evolución de % humedad y lluvia en eventos prolongados.
- Matriz de confusión preliminar (TP/FP/TN/FN) por clases de riesgo.

Observaciones esperadas:
- La combinación de inclinación + vibración incrementa la precisión frente a usar un solo sensor.
- Lluvia persistente y suelo saturado elevan el nivel 1 punto en promedio.

Pendientes (TBD):
- Capturar dataset en campo/laboratorio y ajustar umbrales finos.

</details>


<details>
<summary>

# **Conclusiones**
- retos y trabajo futuro
</summary>

Conclusiones preliminares:
- La fusión de señales mejora la detección temprana de inestabilidad del terreno.
- La arquitectura basada en ESP32 con I2C/ADC simplifica el cableado y reduce costos.

Retos y trabajo futuro:
- Validación en campo y ajuste de umbrales por sitio.
- Integración de comunicación externa (LoRa/WiFi) para telemetría (TBD).
- Gestión de energía avanzada para operación prolongada con baterías (TBD).
- Esquemático y PCB robustos para intemperie (TBD).

</details>


<details>
<summary>

# **Anexos**
</summary>

- Parámetros, umbrales y lógica detallada: `ParametrosYsensores.md`.
- Enunciado del reto: `Enunciado_Chx1_IoT_252 1.pdf`.

### Referencias consultadas

- El Moulat, M.; Debauche, O.; Mahmoudi, S.; Aït Brahim, L.; Manneback, P.; Lebeau, F. (2018). "Monitoring System Using Internet of Things For Potential Landslides". Procedia Computer Science, Vol. 134, pp. 26-34. DOI: 10.1016/j.procs.2018.07.140. Breve: Propuesta de arquitectura IoT (sensores, adquisición y procesamiento) para monitoreo y alerta temprana de deslizamientos. Enlace: https://doi.org/10.1016/j.procs.2018.07.140
- Soegoto, E. S.; Fauzi, F. A.; Luckyardi, S. (2021). "Internet of things for flood and landslide early warning". Journal of Physics: Conference Series 1764 012190. DOI: 10.1088/1742-6596/1764/1/012190. Breve: Uso de IoT como soporte a sistemas de alerta temprana para inundaciones y deslizamientos en contextos turísticos. Enlace: https://doi.org/10.1088/1742-6596/1764/1/012190
- Bhardwaj, R. B. (2021). "Landslide Detection System Based on IOT". (Preprint / artículo en ResearchGate). Breve: Implementación conceptual de un sistema de detección de deslizamientos apoyado en sensores IoT para monitoreo continuo. Enlace: https://www.researchgate.net/publication/350069472_Landslide_Detection_System_Based_on_IOT
- (Vladimir Henao-Céspedes1, Yeison Alberto Garcés-Gómez1, María Nancy Marín Olaya). (2023). "Landslide early warning systems: a perspective from the internet of things. Documento PDF (cloudfront). Enlace: [IJECE](https://d1wqtxts1xzle7.cloudfront.net/97071057/99_28430_EMr_15sep22_16Mei22_20_K-libre.pdf)
- (Natural Hazards) DOI: 10.1007/s11069-022-05524-3. (2022). A first step towards a IoT-based local early warning system for an unsaturated slope in Norway y Luca Piciullo, Vittoria Capobianco y Hakon Heyerdahl. Enlace: `Investigacion/s11069-022-05524-3.pdf`.

</details>
