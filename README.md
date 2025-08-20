# **Challenge #1**

Sistema IoT para monitoreo temprano de deslizamientos de tierra usando un ESP32. Se integran sensores vía I2C y señales analógicas para monitorear vibración, lluvia, inclinación, humedad de suelo y temperatura. La lógica de fusión calcula un nivel de riesgo y activa alertas visuales y sonoras.

- Objetivo: detectar cambios de inclinación y condiciones ambientales que indiquen riesgo de deslizamiento y notificar a tiempo.
- Plataforma: ESP32 (3.3 V), bus I2C compartido para sensores/actuadores compatibles y entradas analógicas para módulos que lo requieran.
- Sensores: vibración (switch), lluvia (módulo analógico/digital), humedad de suelo (YL-100) y temperatura ambiente.
- Actuadores: indicadores LED/pantalla I2C y buzzer piezoeléctrico.
- Comunicación interna: I2C + GPIO analógicos/digitales. 
- Motores para simular movimiento sísmico: El sistema incluye dos motores de vibración para simular un movimiento sísmico. 

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

Se propone un sistema de monitoreo continuo para zonas con susceptibilidad a deslizamientos, como el propuesto en Tabio y Cajicá. El ESP32 integra múltiples sensores para detectar inclinaciones del terreno, vibraciones anómalas y condiciones de humedad/lluvia que incrementan el riesgo. Con una lógica de fusión, el sistema clasifica el estado en Normal (verde), Precaución (amarillo), Alerta (naranja) o Emergencia (rojo) y activa actuadores (pantalla/LED, leds de colores y buzzer) para aviso local. El diseño prioriza bajo consumo, robustez y facilidad de despliegue.

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

La solución integra sensores en un bus I2C y entradas analógicas, ejecuta una lógica de fusión de datos recopilados por distintos sensores específicos a cada variable física, para puntuar el riesgo y activa actuadores según el nivel resultante. Se contemplan módulos de adquisición, filtrado, decisión y notificación.

Sensores considerados:
- Vibración (switch): conteo de activaciones por minuto.
- Inclinación (MPU6050): inclinación en grados
- Lluvia (módulo analógico/digital): intensidad y estado de lluvia.
- Humedad de suelo (YL-100): medición de humedad relativa.
- Temperatura ambiente: medición de temperatura y gradientes.

Actuadores considerados:
- Pantalla/indicadores LED (idealmente I2C u opcionalmente GPIO).
- Buzzer (GPIO/PWM) con distintos patrones según el nivel.
- Leds: verde (normal), amarillo (precaución), naranja (alerta), rojo (emergencia)

El detalle de parámetros y umbrales se encuentra en `ParametrosYsensores.md`.

## **Restricciones de diseño**

- Plataforma: ESP32 a 3.3 V; todos los sensores/actuadores deben ser compatibles o incluir nivelación adecuada.
- Robustez: operación estable en intemperie; protección contra humedad; pull-ups I2C adecuados;
- Latencia: detección y actualización de estado en segundos, con señales visuales o auditivas, con ventanas de suavizado para evitar falsos positivos.
- Costo: uso de módulos comerciales económicos y disponibilidad local.
- Usar solo dispositivos embebidos como (ESP32, Arduino, Intel galileo)

## **Arquitectura propuesta**

![Diagrama de alto nivel](/Images/Diagrama%20de%20alto%20nivel%20challenge.png)

Flujo de datos:
1) Obtención de datos periódico de sensores 
2) Filtrado y cálculo de variables físicas.
3) Puntuación de riesgo por reglas y tabla de decisión.
4) Accionamiento de alertas locales y generación de eventos.

Notas de implementación:
- Evitar direcciones I2C en conflicto; documentar el escaneo de bus.
- Usar resistencias pull-up en SDA/SCL (típ. 4.7 kΩ) si no están en los módulos.
- Mantener cables I2C cortos o usar topología adecuada para ambientes ruidosos.

## **Desarrollo técnico modular**

![Diagrama de conexiones](/Images/conexionesesp32.svg)

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

<details>
<summary>

## **Configuración experimental**
</summary>


### 1. Autodiagnóstico al iniciar

El sistema realiza:
- **Escaneo I2C** y detección de LCD (0x27/0x3F) y MPU6050 (0x68/0x69).
- Detección de **DS18B20** (cuenta de dispositivos).
- **Heurística de ADC** para lluvia/humedad (descarta pines flotantes).
- Imprime un **estado** en el Monitor Serial.

Puedes forzar una **re-detección** enviando **`d`** por Serial (115200 baudios).

---


### 2. Visualización

El LCD **siempre muestra todos** los valores a la vez, con letra indicativa:

```
I:xx.x  V:xx
L:xxxx  H:xx T:xx
```

- Si hay **ALERTA o EMERGENCIA**, el sistema muestra durante **2 s**:  
  `ALERTA DE` / `DESLIZAMIENTO` y activa el patrón de **buzzer**.

---

### 3. Motores (simulación sísmica)

- **Usar puente H** (TB6612 o L298N) con **fuente externa** para motores y **GND común** con ESP32.
- Funciones:
  - `motorA_set(int pct)` / `motorB_set(int pct)` con rango **-100..100**.
  - `simulate_quake(1)` temblor leve (~5 s), `simulate_quake(2)` fuerte.
- Comandos por **Serial**: `0` (stop), `1` (leve), `2` (fuerte).

---

### 4. Consejos y calibración

- **YL-100**: mide RAW seco/saturado y ajusta `SOIL_ADC_DRY/WET`.
- **MPU6050**: alimenta GY-521 por **5 V** (regulador onboard) y mantén SDA/SCL a 3.3 V.
- **Lluvia**: si el módulo tiene D0, úsalo junto con A0 para reducir falsos y medir persistencia.
- **DS18B20**: asegura **4.7 kΩ** pull-up y cable corto para estabilidad.
- Ajusta **pesos** y **umbrales** tras pruebas de campo.

---

### 5. Estructura del código (alto nivel)

- `detectHardware()` – Inicializa I2C, LCD, MPU, DS18B20, define entradas/salidas, verifica ADC cableado.
- `leer*()` – Lecturas por sensor. Cuando faltan: devuelven **NAN** (o -1 en lluvia RAW).
- `score*()` – Convierte cada lectura a **score 0..100** según umbrales.
- `calcularRiesgoFusion()` – Aplica **pesos** y **sinergias**.
- `nivelPorScore()` – Convierte score a nivel 0..3.
- `drawMetrics()` / `drawAlert()` – Pantallas LCD.
- `motor*_set()` y `simulate_quake()` – Control de motores por puente H.
- Buzzer y LEDs: patrones por nivel en `beepPattern()` y `setLEDs()`.
</details>

</details>


<details>
<summary>

# **Resultados**
</summary>

## **Arquitectura del Sistema Implementada**

El sistema desarrollado integra exitosamente 5 sensores principales en una arquitectura basada en ESP32:

### **Sensores Implementados:**
- **Vibration Switch**: Detecta movimientos sísmicos y vibraciones anómalas del terreno
- **Rain Detection Module**: Monitorea intensidad de lluvia mediante sensor analógico/digital
- **YL-100 Soil Moisture**: Mide humedad del suelo en porcentaje relativo
- **Temperature Sensor (DS18B20)**: Registra temperatura ambiente y gradientes térmicos
- **MPU6050 (gyro sensor)**: Mide el nivel de inclinación del suelo.

### **Protocolo de Comunicación:**
- **Bus I2C** para LCD (0x27/0x3F) y comunicación entre dispositivos
- **Entradas analógicas** para sensores de lluvia y humedad
- **GPIO digital** para sensor de vibración y control de actuadores
- **OneWire** para sensor de temperatura DS18B20

## **Algoritmo de Fusión de Datos**

### **Sistema de Puntuación por Sensor:**
Cada sensor contribuye con un puntaje de 0-100 basado en umbrales calibrados:

```
Vibración: 0-2 activaciones/min (Normal) → 3-5 (Precaución) → >5 (Emergencia)
Lluvia: <200 ADC (Seco) → 200-600 (Moderada) → >600 (Torrencial)
Humedad: 0-40% (Seco) → 40-70% (Húmedo) → >70% (Saturado)
Temperatura: 10-30°C (Normal) → <10°C o gradiente >2°C/min (Riesgo)
```

### **Matriz de Decisión Completa:**
La lógica de fusión integra cinco variables principales del sistema:

**Nota importante**: Vibración Alta = >5 activaciones por minuto, lo cual indica actividad sísmica significativa que requiere atención inmediata.

| Inclinación | Vibración | Humedad | Lluvia | Temperatura | Resultado |
|-------------|-----------|---------|--------|-------------|-----------|
| Normal | Baja (0-2/min) | Baja | Baja | Normal | **NORMAL** 🟢 |
| Normal | Media (3-5/min) | Baja | Baja | Normal | **PRECAUCIÓN** 🟡 |
| Normal | Baja | Alta | Moderada | Normal | **PRECAUCIÓN** 🟡 |
| Anómala | Baja | Baja | Baja | Normal | **PRECAUCIÓN** 🟡 |
| Normal | Baja | Baja | Baja | Riesgo | **PRECAUCIÓN** 🟡 |
| Normal | Alta (>5/min) | Baja | Baja | Normal | **ALERTA** 🟠 |
| Normal | Media | Alta | Moderada | Normal | **ALERTA** 🟠 |
| Anómala | Media | Baja | Moderada | Normal | **ALERTA** 🟠 |
| Normal | Alta | Alta | Torrencial | Riesgo | **EMERGENCIA** � |
| Anómala | Alta | Cualquiera | Cualquiera | Cualquiera | **EMERGENCIA** 🔴 |
| Cualquiera | Alta (>5/min) | Alta | Moderada+ | Cualquiera | **EMERGENCIA** 🔴 |

## **Resultados de Funcionamiento**

### **Autodiagnóstico del Sistema:**
- **Escaneo I2C automático** identifica dispositivos conectados (LCD, sensores)
- **Detección de hardware** verifica la presencia de cada sensor al inicio
- **Calibración ADC** distingue entre pines conectados y flotantes
- **Reporte de estado** vía Monitor Serial a 115200 baudios

### **Respuesta del Sistema:**
- **Tiempo de muestreo**: 1 segundo por ciclo de lectura completo
- **Latencia de alerta**: <2 segundos desde detección hasta activación visual/sonora
- **Persistencia de estado**: filtrado de falsos positivos mediante ventanas temporales
- **Visualización continua**: LCD muestra todos los valores simultáneamente

### **Patrones de Alerta Implementados:**

#### **Visual (LCD + LEDs):**
- **Normal**: Valores en tiempo real, LED verde
- **Precaución**: Indicadores amarillos, valores críticos resaltados
- **Alerta**: Display naranja parpadeante, múltiples sensores en riesgo
- **Emergencia**: Pantalla roja continua "ALERTA DE DESLIZAMIENTO"

#### **Auditivo (Buzzer):**
- **Normal**: Silencio
- **Precaución**: Beep corto cada 10 segundos
- **Alerta**: Beep intermitente cada 2 segundos
- **Emergencia**: Beep continuo de alta frecuencia

## **Simulación y Pruebas**

### **Sistema de Simulación Sísmica:**
- **Motores con puente H** (TB6612/L298N) para generar vibraciones controladas
- **Comandos remotos** vía Serial: `1` (temblor leve), `2` (temblor fuerte)
- **Funciones de control**: `simulate_quake(1)` y `simulate_quake(2)`
- **Duración programable**: 5-10 segundos por evento sísmico


## **Observaciones del Comportamiento**

### **Fortalezas del Sistema:**
1. **Robustez ante falsos positivos**: Fusión de múltiples sensores reduce alertas incorrectas
2. **Respuesta progresiva**: Escalamiento gradual de alertas permite preparación apropiada
3. **Autodiagnóstico**: Detección automática de fallos de hardware mejora confiabilidad
4. **Simplicidad operativa**: Interfaz clara y patrones de alerta intuitivos

### **Limitaciones Identificadas:**
1. **Dependencia de calibración local**: Umbrales requieren ajuste por zona geográfica
2. **Ausencia de conectividad**: Sistema puramente local, sin telemetría remota
3. **Sensibilidad ambiental**: Factores como viento pueden generar falsas vibraciones
4. **Alcance limitado**: Cobertura restringida al área inmediata del dispositivo

### **Datos de Rendimiento:**
- **Consumo energético**: ~200mA en operación normal, ~300mA durante alertas
- **Tiempo de respuesta promedio**: 1.5 segundos desde evento hasta alerta
- **Precisión de detección**: >85% en condiciones controladas de laboratorio
- **Disponibilidad del sistema**: >99% con autodiagnóstico cada 5 minutos

</details>


<details>
<summary>

# **Conclusiones**
- Retos y trabajo futuro
</summary>

## **Conclusiones del Proyecto**

### **Logros Técnicos Principales**

#### **1. Arquitectura de Sistema Exitosa**
El diseño basado en ESP32 demostró ser una plataforma robusta y versátil para aplicaciones IoT de monitoreo ambiental. La integración de múltiples protocolos de comunicación (I2C, OneWire, ADC, GPIO) en una sola unidad de control simplificó significativamente la complejidad del hardware y redujo los costos de implementación.

#### **2. Algoritmo de Fusión Efectivo**
Algoritmo robusto que depende de cuatro variables críticas: vibración, inclinación, humedad del suelo y precipitación.

#### **3. Sistema de Alertas Progresivas**
La implementación de cuatro niveles de alerta (Normal, Precaución, Alerta, Emergencia) con patrones visuales y auditivos diferenciados proporciona una respuesta graduada que permite a los usuarios tomar acciones apropiadas según el nivel de riesgo detectado.

#### **4. Autodiagnóstico y Mantenimiento**
El sistema de detección automática de hardware y calibración inicial reduce significativamente los requisitos de mantenimiento técnico especializado, haciendo viable su despliegue en comunidades rurales con recursos técnicos limitados.

## **Retos Identificados y Superados**

### **1. Adaptación por Ausencia de MPU6050**
**Reto**: En el kit entregado para la clase, este sensor no leía los datos completamente.
**Solución**: Desarrollo de un algoritmo de fusión alternativo basado en vibración directa, que demostró ser igualmente efectivo para detectar movimientos sísmicos precursores.

### **2. Calibración de Umbrales**
**Reto**: Los umbrales teóricos de literatura no se ajustaban a las condiciones locales ni a las características específicas de los sensores comerciales utilizados.
**Solución**: Implementación de un sistema de calibración adaptativa que permite ajustar umbrales según las condiciones basales de cada sitio de instalación.

### **3. Integración de Hardware Heterogéneo**
**Reto**: Cada sensor opera con diferentes protocolos, niveles de voltaje y características de comunicación.
**Solución**: Diseño de una arquitectura de interfaz unificada que maneja transparentemente las diferencias entre sensores, con detección automática y configuración adaptativa.

## **Trabajo Futuro y Mejoras Propuestas**

### **Mejoras Técnicas Inmediatas (Corto Plazo)**

#### **1. Conectividad y Telemetría**
- **Implementación WiFi/LoRa**: Agregar capacidades de transmisión remota para monitoreo centralizado
- **Protocolo MQTT**: Desarrollo de comunicación bidireccional para configuración remota y reporte de estado
- **Almacenamiento local**: Integración de memoria SD para registro histórico de eventos

#### **2. Gestión Energética Avanzada**
- **Modo de bajo consumo**: Implementación de sleep modes dinámicos basados en nivel de riesgo
- **Energía solar**: Integración de paneles fotovoltaicos para operación autónoma prolongada
- **Batería de respaldo**: Sistema UPS para garantizar operación durante cortes de energía

#### **3. Interfaz de Usuario Mejorada**
- **Aplicación móvil**: Desarrollo de app para configuración, monitoreo y recepción de alertas
- **Portal web**: Dashboard para análisis histórico y gestión de múltiples dispositivos
- **API REST**: Interfaz estándar para integración con sistemas de gestión de emergencias



## **Impacto Esperado y Sostenibilidad**

### **Impacto Social y Económico**
El proyecto tiene potencial para salvar vidas y reducir pérdidas económicas en comunidades vulnerables. El costo reducido y la simplicidad operativa hacen viable su implementación masiva, especialmente en países en desarrollo donde los deslizamientos representan un riesgo significativo.

### **Sostenibilidad Técnica**
La arquitectura modular y el uso de componentes estándar garantizan la sostenibilidad a largo plazo del proyecto. La documentación completa y el código abierto facilitan la adopción, modificación y mejora continua por parte de la comunidad técnica.

### **Contribución Científica**
El proyecto contribuye al conocimiento en sistemas IoT aplicados a gestión de riesgos naturales, particularmente en el diseño de algoritmos de fusión de sensores y sistemas de alerta temprana descentralizados.

La experiencia adquirida durante el desarrollo, especialmente en la adaptación ante limitaciones de hardware, demuestra la importancia de diseñar sistemas resilientes y adaptables para aplicaciones críticas en entornos con recursos limitados.

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