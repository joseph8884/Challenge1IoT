
# **Challenge #1**


Sistema IoT para monitoreo temprano de deslizamientos de tierra usando un ESP32. Se integran sensores vía I2C y señales analógicas para medir inclinación, vibración, lluvia, humedad de suelo y temperatura. La lógica de fusión calcula un nivel de riesgo y activa alertas visuales y sonoras.

- Objetivo: detectar cambios de inclinación y condiciones ambientales que indiquen riesgo de deslizamiento y notificar a tiempo.
- Plataforma: ESP32 (3.3 V), bus I2C compartido para sensores/actuadores compatibles y entradas analógicas para módulos que lo requieran.
- Sensores: MPU6050 (IMU), vibración (switch), lluvia (módulo analógico/digital), humedad de suelo (YL-100), temperatura ambiente.
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

- Reducir el impacto humano y material causado por deslizamientos mediante alerta temprana.
- Proveer una solución de bajo costo y rápida instalación para zonas vulnerables.
- Facilitar la obtencion de variables fisicas que influyen en los deslizamientos de tierra, para análisis de patrones y mejora continua.

</details>



<details>
<summary>

# **Justificación**
</summary>

La combinación de inclinación, vibración y humedad es un indicador fiable de inestabilidad del terreno. Un sistema distribuido basado en ESP32 permite muestreo frecuente, procesamiento local y alertas inmediatas sin depender de conectividad constante. El uso de I2C y entradas analógicas simplifica el cableado y reduce costos.

</details>



<details>
<summary>

# **Solución propuesta**
</summary>

La solución integra sensores en un bus I2C y entradas analógicas, ejecuta una lógica de fusión de datos recompilados por distintos sensores especificos a cada variable fisica, para puntuar el riesgo y activa actuadores según el nivel resultante. Se contemplan módulos de adquisición, filtrado, decisión y notificación.

Sensores considerados:
- MPU6050 (I2C): inclinación, aceleración y velocidad angular.
- Vibración (digital): conteo de eventos/activaciones por minuto.
- Lluvia (analógica/digital): intensidad y estado de lluvia.
- Humedad de suelo YL-100 (analógica): % relativo de humedad/saturación.
- Temperatura (interfaz según sensor disponible): °C y gradientes.

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


Vista de alto nivel del sistema:

```
[Sensores I2C]
	- MPU6050 (IMU)
	- (Opcional) Pantalla OLED/LED I2C

[Sensores analógicos/digitales]
	- Lluvia (A/D)
	- Humedad de suelo YL-100 (A)
	- Vibración (D)
	- Temperatura (según modelo)

[ESP32]
	- Módulo de adquisición + filtrado
	- Lógica de fusión y decisión
	- Módulo de alertas (LED/Pantalla + Buzzer)
	- (Opcional) Registro de eventos

[Actuadores]
	- LEDs/Pantalla (I2C/GPIO)
	- Buzzer (GPIO/PWM)
```

Flujo de datos:
1) Muestreo periódico de sensores (IMU a alta frecuencia; analógicos promediados).
2) Filtrado y cálculo de deltas/tendencias (e.g., Δinclinación en ventanas de tiempo).
3) Puntuación de riesgo por reglas y tabla de decisión.
4) Accionamiento de alertas locales y registro de eventos.

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
- Adquisición de datos: drivers I2C/ADC, temporización de muestreo.
- Fusión/decisión: reglas por umbral.
- Alertas: control de LED/pantalla y patrones de buzzer.

Diagrama de flujo (texto):
1) Inicio y auto-test de sensores.
2) Lectura IMU (inclinación/accel) + conteo vibración + lluvia + humedad + temperatura.
3) Filtrado y cálculo de indicadores (Δinclinación, activaciones/min, % humedad, intensidad lluvia).
4) Cálculo de puntaje de riesgo y mapeo a estado.
5) Actualizar actuadores y registrar evento si cambia el estado.
6) Ajustar frecuencia de muestreo según estado (modo sleep inteligente en Normal).

Esquemático de hardware (alto nivel):
- Bus I2C: ESP32 SDA/SCL a IMU y (opcional) pantalla I2C, con pull-ups compartidos.
- Entradas analógicas: lluvia y YL-100 a ADC del ESP32 (referencia 3.3 V, rango 0–4095).
- Entrada digital: vibración a GPIO con interrupción/anti-rebote por software.
- Actuadores: buzzer a GPIO/PWM; LEDs a GPIO o pantalla I2C.
- Alimentación: 5 V/3.3 V según módulos, regulación estable y común a sensores.

Estándares y buenas prácticas:
- Frecuencias de muestreo documentadas; timestamps consistentes.
- Filtrado anti-ruido y promedios móviles para analógicas.
- Registros de eventos con marca temporal y estado.
- Convenciones de código: separación por módulos, constantes/umbrales centralizadas.


## **Configuracion experimental** 


Objetivo: validar umbrales y la matriz de decisión reduciendo falsos positivos/negativos.

Escenarios de prueba:
- Inclinación: variaciones controladas en lapsos de 10 minutos para evaluar Δinclinación por rangos (Normal→Emergencia).
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
- Referencias adicionales: por definir (TBD).

</details>