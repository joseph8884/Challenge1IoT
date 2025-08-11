# Parámetros y Lógica de Sensores para Detección de Deslizamientos

// **Nota:** Los valores aquí presentados son estimaciones iniciales y son generados con IA

## 🎯 **LÓGICA DE FUSIÓN DE SENSORES**

### **1. MPU6050 - Sensor de Inclinación y Aceleración**

**Variables a medir:**
- **Ángulo de inclinación X, Y, Z** (grados)
- **Aceleración en los 3 ejes** (m/s²)
- **Velocidad angular** (deg/s)

**Umbrales críticos:**
```
NORMAL:     Δ inclinación < 0.5° en 10 min
PRECAUCIÓN: Δ inclinación 0.5° - 1.0° en 10 min  
ALERTA:     Δ inclinación 1.0° - 2.0° en 10 min
EMERGENCIA: Δ inclinación > 2.0° en 10 min

Aceleración anómala: > 0.2g en cualquier eje
```

**Algoritmo:**
- Calcular inclinación cada 1 segundo
- Promedio móvil de 10 lecturas
- Detectar cambios graduales Y súbitos

---

### **2. Vibration Switch - Detector de Vibraciones**

**Variables a medir:**
- **Estado digital** (HIGH/LOW)
- **Frecuencia de activaciones** por minuto

**Umbrales críticos:**
```
NORMAL:     0-2 activaciones/minuto
PRECAUCIÓN: 3-5 activaciones/minuto
ALERTA:     6-10 activaciones/minuto  
EMERGENCIA: >10 activaciones/minuto o activación continua
```

**Algoritmo:**
- Contar pulsos en ventana de 60 segundos
- Filtrar vibraciones por viento/animales (< 200ms)
- Activación continua > 5 segundos = EMERGENCIA

---

### **3. Rain Detection Module - Detector de Lluvia**

**Variables a medir:**
- **Intensidad de lluvia** (analógica 0-1023)
- **Estado lluvia** (digital TRUE/FALSE)

**Umbrales críticos:**
```
Sin lluvia:     0-100 (valor analógico)
Lluvia ligera:  101-300
Lluvia moderada: 301-600
Lluvia intensa: 601-900
Lluvia torrencial: >900
```

**Algoritmo:**
- Promedio de intensidad en 5 minutos
- Detectar lluvia persistente > 30 minutos
- Factor multiplicador de riesgo según intensidad

---

### **4. YL-100 - Sensor de Humedad del Suelo**

**Variables a medir:**
- **Humedad del suelo** (% 0-100)
- **Saturación relativa** comparada con valor base

**Umbrales críticos:**
```
Suelo seco:     0-30%
Suelo húmedo:   31-60%  
Suelo mojado:   61-80%
Suelo saturado: 81-100%
```

**Algoritmo:**
- Establecer valor base en suelo seco
- Calcular incremento relativo
- Saturación > 80% + lluvia = RIESGO ALTO

---

### **5. Temperature Sensor - Sensor de Temperatura**

**Variables a medir:**
- **Temperatura ambiente** (°C)
- **Gradiente térmico** (cambios rápidos)

**Umbrales críticos:**
```
Riesgo por congelación: < 5°C
Riesgo por deshielo:    5°C - 15°C tras período frío
Temperatura normal:     > 15°C
```

**Algoritmo:**
- Factor de riesgo adicional en temperaturas extremas
- Ciclos hielo-deshielo aumentan inestabilidad del suelo

---

## 🔥 **ALGORITMO DE FUSIÓN INTELIGENTE**

### **Matriz de Decisión:**

| Inclinación | Vibración | Humedad | Temperatura | **RESULTADO** |
|-------------|-----------|---------|-------------|---------------|
| ❌ | ❌ | ❌ | Normal | **NORMAL** 🟢 |
| ✅ | ❌ | ❌ | Normal | **PRECAUCIÓN** 🟡 |
| ❌ | ✅ | ❌ | Normal | **PRECAUCIÓN** 🟡 |
| ❌ | ❌ | ✅ | Normal | **PRECAUCIÓN** 🟡 |
| ✅ | ✅ | ❌ | Normal | **ALERTA** 🟠 |
| ✅ | ❌ | ✅ | Normal | **ALERTA** 🟠 |
| ❌ | ✅ | ✅ | Normal | **ALERTA** 🟠 |
| ✅ | ✅ | ✅ | Normal | **EMERGENCIA** 🔴 |
| Cualquiera | Cualquiera | Cualquiera | < 5°C | **+1 Nivel** ❄️ |

### **Código de Lógica (Pseudocódigo):**

```cpp
int riskScore = 0;
bool riesgoInclinacion = false;
bool riesgoVibracion = false; 
bool riesgoHumedad = false;

// Evaluar cada sensor
if (deltaInclinacion > UMBRAL_INCLINACION) {
    riesgoInclinacion = true;
    riskScore++;
}

if (vibrationCount > UMBRAL_VIBRACION || vibrationSwitch == HIGH) {
    riesgoVibracion = true;
    riskScore++;
}

if (soilMoisture > 80 && rainIntensity > 300) {
    riesgoHumedad = true;
    riskScore++;
}

// Factor temperatura
if (temperature < 5) riskScore++;

// Determinar estado
if (riskScore == 0) estado = NORMAL;
else if (riskScore == 1) estado = PRECAUCION;
else if (riskScore == 2) estado = ALERTA;
else estado = EMERGENCIA;
```

---

## 📱 **SISTEMA DE ALERTAS**

### **Pantalla LED:**
- **Verde:** Datos normales en tiempo real
- **Amarillo:** Advertencia + valor del sensor en riesgo
- **Naranja:** Alerta + valores críticos parpadeando
- **Rojo:** Emergencia + mensaje "EVACUACIÓN"

### **Buzzer:**
- **Silencio:** Estado normal
- **Beep corto c/10s:** Precaución
- **Beep intermitente c/2s:** Alerta  
- **Beep continuo:** Emergencia

---

## 🚀 **FUNCIONES AVANZADAS PARA 5.0**

### **1. Algoritmo Predictivo:**
```cpp
bool detectarTendencia() {
    // Analizar últimas 10 lecturas
    if (ultimas10Lecturas[0] < ultimas10Lecturas[9]) {
        return true; // Tendencia empeorando
    }
    return false;
}
```

### **2. Auto-calibración:**
- Recalibrar sensores cada 24 horas
- Establecer nuevos valores base según condiciones

### **3. Memoria de eventos:**
- Guardar últimos 100 eventos en EEPROM
- Análisis de patrones históricos

### **4. Modo Sleep inteligente:**
- Reducir frecuencia de muestreo en condiciones normales
- Aumentar frecuencia cuando se detecta riesgo

Esta lógica garantiza una **detección temprana y precisa** combinando múltiples señales para minimizar falsos positivos.