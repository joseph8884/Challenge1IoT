# Parámetros y Lógica de Sensores para Detección de Deslizamientos

> **Nota:** Los valores aquí presentados se basan en referencias científicas y se adaptan al contexto de sensores IoT de bajo costo. Requieren validación en campo para cada zona específica.

## 🎯 **LÓGICA DE FUSIÓN DE SENSORES**

---

### **1. Vibration Switch - Detector de Vibraciones**

**Variables a medir:**
- Estado digital (HIGH/LOW)
- Frecuencia de activaciones por minuto

**Umbrales críticos (referencia: Bhardwaj, 2021):**
- NORMAL: 0-2 activaciones/minuto
- PRECAUCIÓN: 3-5 activaciones/minuto
- ALERTA: >5 activaciones/minuto
- EMERGENCIA: activación continua > 5 segundos



**Algoritmo:**
- Contar pulsos en ventana de 60 s.
- Filtrar vibraciones espurias (p. ej. viento/animales) con duración < 200 ms.
- Activación continua > 5 s = EMERGENCIA.

---

### **2. Rain Detection Module - Detector de Lluvia**

**Variables a medir:**
- Intensidad de lluvia (0–1023 ADC)
- Estado lluvia (digital TRUE/FALSE)

**Umbrales críticos (Soegoto et al., 2021):**
- Sin lluvia: < 200
- Lluvia ligera: 200 – 300
- Lluvia moderada: 301 – 600
- Lluvia intensa: 601 – 900
- Lluvia torrencial: > 900 o persistente > 30 min



**Algoritmo:**
- Promediar intensidad en ventana de 5 min.
- Incrementar riesgo si la lluvia es persistente > 30 min.
- Multiplicar puntaje de riesgo cuando lluvia coincide con alta humedad de suelo.

---

### **3. YL-100 - Sensor de Humedad del Suelo**

**Variables a medir:**
- Humedad del suelo (% relativo)
- Saturación respecto al valor base (suelo seco inicial)

**Umbrales críticos (El Moulat et al., 2018; Piciullo et al., 2022):**
- Suelo seco: 0 – 40 %
- Suelo húmedo: 41 – 70 %
- Suelo saturado: > 70 % (riesgo alto)


**Algoritmo:**
- Establecer calibración inicial en suelo seco.
- Incremento de humedad > 30 % respecto al valor base = riesgo elevado.
- Saturación > 70 % combinada con lluvia = condición de alerta.

---

### **4. Temperature Sensor - Sensor de Temperatura**

**Variables a medir:**
- Temperatura ambiente (°C)
- Gradiente de cambio (°C/min)

**Umbrales críticos (Henao-Céspedes et al., 2023):**
- Normal: 10 – 30 °C
- Precaución: < 10 °C o gradiente > 2 °C/min
- Alerta/Emergencia: < 5 °C o cambios bruscos > 5 °C/min


**Algoritmo:**
- Añadir +1 nivel de riesgo si T < 5 °C.
- Considerar ciclos de hielo-deshielo como factores de inestabilidad.
- Evaluar gradientes de cambio rápidos como precursores de fractura.

---

## 🔥 **ALGORITMO DE FUSIÓN INTELIGENTE**

### **Matriz de Decisión**

| Vibración | Humedad | Lluvia | Temperatura | **Resultado** |
|-----------|---------|--------|-------------|---------------|
| Baja      | Baja    | Baja   | Normal      | **NORMAL** 🟢 |
| Media     | Baja    | Baja   | Normal      | **PRECAUCIÓN** 🟡 |
| Baja      | Media   | Media  | Normal      | **PRECAUCIÓN** 🟡 |
| Alta      | Media   | Media  | Normal      | **ALERTA** 🟠 |
| Alta      | Alta    | Alta   | Normal      | **EMERGENCIA** 🔴 |
| Cualquiera| Cualquiera | Cualquiera | < 5 °C | **+1 Nivel** ❄️ |

---

### **Código de Lógica (Pseudocódigo)**

```cpp
int riskScore = 0;

// Evaluar vibración
if (vibrationCount > 5 || vibrationSwitch == HIGH) {
    riskScore++;
}

// Evaluar humedad
if (soilMoisture > 70) {
    riskScore++;
}

// Evaluar lluvia
if (rainIntensity > 600 || rainPersistente > 30min) {
    riskScore++;
}

// Evaluar temperatura
if (temperature < 5 || gradienteTemp > 5) {
    riskScore++;
}

// Determinar estado
if (riskScore == 0) estado = NORMAL;
else if (riskScore == 1) estado = PRECAUCION;
else if (riskScore == 2) estado = ALERTA;
else estado = EMERGENCIA;
```

### Sistemas de alertas
Buzzer:

- Silencio: Normal
- Beep corto cada 10s: Precaución
- Beep intermitente cada 2s: Alerta
- Beep continuo: Emergencia


### 📚 Referencias

El Moulat, M. et al. (2018). Monitoring System Using Internet of Things For Potential Landslides. Procedia Computer Science, 134, 26–34.

Soegoto, E. S. et al. (2021). Internet of things for flood and landslide early warning. J. Phys.: Conf. Ser. 1764 012190.

Bhardwaj, R. B. (2021). Landslide Detection System Based on IoT. ResearchGate preprint.

Henao-Céspedes, V., Garcés-Gómez, Y., & Marín Olaya, M. N. (2023). Landslide early warning systems: a perspective from IoT. IJECE.

Piciullo, L., Capobianco, V., & Heyerdahl, H. (2022). A first step towards a IoT-based local early warning system for an unsaturated slope in Norway. Natural Hazards.