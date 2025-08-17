/*
  ====== Sistema de Alerta Temprana de Talud con ESP32 ======
  Adaptado a:
    - I2C: SDA=21, SCL=22
    - LCD 16x2 I2C (0x27)
    - MPU6050 I2C (0x68)
    - Vibración: GPIO2 (pulso por interrupción)  *OJO: GPIO2 es pin de arranque*
    - Lluvia: A0=GPIO36 (ADC1_CH0), D0=GPIO4
    - Humedad suelo: A0=GPIO39 (ADC1_CH3)
    - Temperatura: DS18B20 en GPIO5 (OneWire)
    - LEDs: Verde=13, Amarillo=12, Naranja=14, Rojo=27
    - Buzzer: GPIO25

  Lógica de fusión: basada en el modelo ponderado con sinergias (inclinación, vibración, lluvia, humedad, temperatura).
*/

/******************* LIBRERÍAS *******************/
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/******************* PINES *******************/
// I2C
#define SDA_PIN 21
#define SCL_PIN 22

// LCD I2C
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// MPU6050
Adafruit_MPU6050 mpu;

// Vibración (pulso por interrupción)
const int PIN_VIBRATION = 32; // recomendado para evitar problemas de arranque

// Lluvia analógica + digital
const int PIN_RAIN_A = 36;    // ADC1_CH0
const int PIN_RAIN_D = 4;     // digital (umbral del módulo)

// Humedad de suelo (ADC)
const int PIN_SOIL_A = 39;    // ADC1_CH3
const int SOIL_ADC_DRY = 3200;   // calib: seco
const int SOIL_ADC_WET = 1400;   // calib: saturado

// Temperatura DS18B20
const int PIN_DS18B20 = 5;
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds(&oneWire);

// LEDs
const int LED_GREEN  = 13;
const int LED_YELLOW = 12;
const int LED_ORANGE = 14;
const int LED_RED = 27;

// Buzzer
const int PIN_BUZZER = 25;

/******************* ESTADO *******************/
// Vibración (eventos/minuto)
volatile unsigned long vibPulses = 0;
volatile unsigned long lastVibMicros = 0; // anti-rebote por tiempo
unsigned long lastVibCalc = 0;
const unsigned long VIB_WINDOW_MS = 10000; // ventana corta para pruebas (10 s)
float vibracion_per_min = 0.0f;

// Lluvia (mm/h estimado desde analógico)
float lluvia_mm_h = 0.0f;
unsigned long lastLcdTick = 0;

// Inclinación
float delta_inclinacion_deg = 0.0f; // |pitch| o |roll| dominante

// Humedad de suelo (%)
float humedad_suelo_pct = 0.0f;

// Temperatura (°C)
float temperatura_c = 0.0f;

/******************* UTILIDADES *******************/
static inline float clampf(float x, float lo, float hi){ return (x<lo)?lo: (x>hi)?hi: x; }

void IRAM_ATTR isrVibration(){
  unsigned long now = micros();
  if (now - lastVibMicros > 15000) { // 15 ms de guarda contra rebotes/ruido
    vibPulses++;
    lastVibMicros = now;
  }
}

// Buzzer: usa tone() (el core ESP32 lo soporta)
static void buzz(uint16_t f, uint16_t ms){ tone(PIN_BUZZER, f, ms); }

void beepPattern(uint8_t nivel){
  static unsigned long t0=0; unsigned long now=millis();
  switch(nivel){
    case 0: break; // silencio
    case 1: if (now-t0>3000){ t0=now; buzz(1200,100);} break;
    case 2: if (now-t0>650){ t0=now; buzz(1500,120); delay(80); buzz(1500,120);} break;
    case 3: if (now-t0>300){ t0=now; buzz(2000,180);} break;
  }
}

/******************* LÓGICA DE RIESGO *******************/
float calcularRiesgoFusion(float delta_inclinacion, float vibracion_per_min, float intensidad_lluvia,
                           float humedad_suelo_pct, float temperatura_celsius){
  // Inclinación
  float s_inc = (delta_inclinacion>5.0)?100.0:(delta_inclinacion>2.0)? 60.0+(delta_inclinacion-2.0)*13.33:
                (delta_inclinacion>0.5)? 20.0+(delta_inclinacion-0.5)*26.67: delta_inclinacion*40.0;
  // Vibración
  float s_vib = (vibracion_per_min>15.0)?100.0:(vibracion_per_min>8.0)? 50.0+(vibracion_per_min-8.0)*7.14:
                (vibracion_per_min>3.0)? 15.0+(vibracion_per_min-3.0)*7.0: vibracion_per_min*5.0;
  // Lluvia
  float s_llu = (intensidad_lluvia>25.0)?100.0:(intensidad_lluvia>10.0)? 60.0+(intensidad_lluvia-10.0)*2.67:
                (intensidad_lluvia>2.0)? 20.0+(intensidad_lluvia-2.0)*5.0: intensidad_lluvia*10.0;
  // Suelo
  float s_sue = (humedad_suelo_pct>85.0)?100.0:(humedad_suelo_pct>70.0)? 65.0+(humedad_suelo_pct-70.0)*2.33:
                (humedad_suelo_pct>40.0)? 25.0+(humedad_suelo_pct-40.0)*1.33: humedad_suelo_pct*0.625;
  // Temperatura
  float s_tmp = (temperatura_celsius<2.0)? 80.0-temperatura_celsius*10.0: (temperatura_celsius>35.0)? 40.0:
                (temperatura_celsius>30.0)? 20.0+(temperatura_celsius-30.0)*4.0:
                (temperatura_celsius<5.0)? 30.0-(temperatura_celsius-2.0)*6.67: 0.0;
  // Sinergias
  float f_iv=1.0; if (s_inc>40 && s_vib>30) f_iv=1.5; if (s_inc>70 && s_vib>50) f_iv=2.0;
  float f_sat=1.0; if (s_llu>30 && s_sue>50) f_sat=1.3; if (s_llu>60 && s_sue>70) f_sat=1.6;
  float f_frz = (temperatura_celsius<0 && humedad_suelo_pct>60)? 1.4: 1.0;
  // Pesos
  float w_inc=0.40, w_vib=0.25, w_sue=0.20, w_llu=0.10, w_tmp=0.05;
  float base = s_inc*w_inc + s_vib*w_vib + s_sue*w_sue + s_llu*w_llu + s_tmp*w_tmp;
  float fin = clampf(base * f_iv * f_sat * f_frz, 0.0, 100.0);
  return fin;
}

int obtenerNivelRiesgo(float score){
  if (score>=76.0) return 3; else if (score>=51.0) return 2; else if (score>=26.0) return 1; else return 0;
}

/******************* LECTURA DE SENSORES *******************/
float leerInclinacionDeg(){
  sensors_event_t a,g,t; mpu.getEvent(&a,&g,&t);
  float ax=a.acceleration.x, ay=a.acceleration.y, az=a.acceleration.z;
  float roll = atan2(ay, az) * 180.0/PI;
  float pitch= atan2(-ax, sqrt(ay*ay+az*az)) * 180.0/PI;
  return max(fabs(roll), fabs(pitch));
}

float leerVibracionPerMin(){
  unsigned long now = millis();
  if (now - lastVibCalc >= VIB_WINDOW_MS){
    noInterrupts();
    unsigned long p = vibPulses; vibPulses = 0;
    interrupts();
    vibracion_per_min = (float)p * (60000.0f / (float)VIB_WINDOW_MS); // escala a eventos/min
    lastVibCalc = now;
  }
  return vibracion_per_min;
}


float leerLluviaMMh(){
  int adc = analogRead(PIN_RAIN_A); // 0..4095
  float pct = 100.0f * (4095 - adc) / 4095.0f; // 0 seco, 100 mojado
  int d = digitalRead(PIN_RAIN_D);            // D0 del módulo (en muchos: HIGH=seco, LOW=mojado)
  if (d == HIGH && pct < 8.0f) pct = 0.0f;    // filtro: si digital dice seco y el % es pequeño, fuerza 0
  return pct * 0.30f;                         // conversión aprox a mm/h para el modelo
}

float leerHumedadSueloPct(){
  int adc = analogRead(PIN_SOIL_A);
  adc = constrain(adc, min(SOIL_ADC_WET,SOIL_ADC_DRY), max(SOIL_ADC_WET,SOIL_ADC_DRY));
  float pct = 100.0f * (float)(SOIL_ADC_DRY - adc) / (float)(SOIL_ADC_DRY - SOIL_ADC_WET);
  return clampf(pct, 0.0f, 100.0f);
}

float leerTemperaturaC(){
  ds.requestTemperatures();
  float t = ds.getTempCByIndex(0);
  if (t==DEVICE_DISCONNECTED_C) return 25.0f; // valor por defecto si falla
  return t;
}

/******************* UI EN LCD *******************/
const char* nivelToText(int n){ switch(n){case 0:return "NORMAL";case 1:return "PRECAU";case 2:return "ALERTA";case 3:return "EMERG";} return "?"; }

void drawLCD(float score, int nivel){
  static uint8_t page=0; page=(page+1)%4;
  lcd.clear();
  switch(page){
    case 0:
      lcd.setCursor(0,0); lcd.print("Riesgo:"); lcd.print(score,0); lcd.print(" "); lcd.print(nivelToText(nivel));
      lcd.setCursor(0,1); lcd.print("Inc:"); lcd.print(delta_inclinacion_deg,1); lcd.print((char)223); lcd.print(" Vib:"); lcd.print(vibracion_per_min,0);
      break;
    case 1:
      lcd.setCursor(0,0); lcd.print("Llu:"); lcd.print(lluvia_mm_h,1); lcd.print(" mm/h");
      lcd.setCursor(0,1); lcd.print("Suelo:"); lcd.print(humedad_suelo_pct,0); lcd.print("%  ");
      break;
    case 2:
      lcd.setCursor(0,0); lcd.print("Temp:"); lcd.print(temperatura_c,1); lcd.print("C   ");
      lcd.setCursor(0,1); lcd.print("RainD:"); lcd.print(digitalRead(PIN_RAIN_D)?"NO":"SI");
      break;
    case 3:
      lcd.setCursor(0,0); lcd.print("Modo Talud ESP32 ");
      lcd.setCursor(0,1); lcd.print("SDA21 SCL22 BZ25");
      break;
  }
}

/******************* SETUP & LOOP *******************/
void setup(){
  Serial.begin(115200);
  delay(200);

  // I2C y LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init(); lcd.backlight();
  lcd.clear(); lcd.setCursor(0,0); lcd.print("Talud ESP32"); lcd.setCursor(0,1); lcd.print("Inicializando...");

  // MPU6050
  if (!mpu.begin(0x68)) { // intenta 0x68
    if (!mpu.begin(0x69)) { // intenta 0x69 si AD0=HIGH
      Serial.println("[ERR] MPU6050 no encontrado (0x68/0x69)");
      lcd.clear(); lcd.setCursor(0,0); lcd.print("MPU6050 ERROR");
    } else { Serial.println("MPU en 0x69"); }
  } else {
    Serial.println("MPU en 0x68");
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // DS18B20
  ds.begin();

  // LEDs y buzzer
  pinMode(LED_GREEN,OUTPUT); pinMode(LED_YELLOW,OUTPUT); pinMode(LED_ORANGE,OUTPUT); pinMode(LED_RED,OUTPUT);
  pinMode(PIN_BUZZER,OUTPUT);

  // Vibración y lluvia digital
  pinMode(PIN_VIBRATION, INPUT); // el módulo ya entrega 0/3.3V; no forzar pullups // evita flotar (módulos con colector abierto)
  attachInterrupt(digitalPinToInterrupt(PIN_VIBRATION), isrVibration, FALLING);
  pinMode(PIN_RAIN_D, INPUT_PULLUP);

  // ADC
  analogReadResolution(12);

  // Aviso listo
  lcd.clear(); lcd.setCursor(0,0); lcd.print("Sistema listo!"); lcd.setCursor(0,1); lcd.print("Uni Sabana");
  Serial.println("Sistema iniciado");
}

int obtenerNivelLED(float score){ if(score>=76) return 3; if(score>=51) return 2; if(score>=26) return 1; return 0; }
void setLEDs(int nivel){
  digitalWrite(LED_GREEN,  nivel==0);
  digitalWrite(LED_YELLOW, nivel==1);
  digitalWrite(LED_ORANGE, nivel==2);
  digitalWrite(LED_RED,    nivel==3);
}

void loop(){
  // 1) Lecturas
  delta_inclinacion_deg = leerInclinacionDeg();
  vibracion_per_min     = leerVibracionPerMin();
  lluvia_mm_h           = leerLluviaMMh();
  humedad_suelo_pct     = leerHumedadSueloPct();
  temperatura_c         = leerTemperaturaC();

  // 2) Score y nivel
  float score = calcularRiesgoFusion(delta_inclinacion_deg, vibracion_per_min, lluvia_mm_h, humedad_suelo_pct, temperatura_c);
  int nivel = obtenerNivelRiesgo(score);

  // 3) Actuaciones
  setLEDs(nivel);
  beepPattern(nivel);

  // 4) UI
  if (millis()-lastLcdTick>1500){ lastLcdTick=millis(); drawLCD(score, nivel); }

  // 5) Log
  static unsigned long lastLog=0; if (millis()-lastLog>2000){ lastLog=millis();
    Serial.printf("Inc=%.2f deg | Vib=%.1f ev/min | Llu=%.2f mm/h | HumSuelo=%.1f %% | Temp=%.1f C | Score=%.1f | Nivel=%d\n",
      delta_inclinacion_deg, vibracion_per_min, lluvia_mm_h, humedad_suelo_pct, temperatura_c, score, nivel);
  }

  delay(80);
}

/* Notas:
 - Si la carga falla por usar GPIO2 (strapping), mueva PIN_VIBRATION a 32/33.
 - Calibre SOIL_ADC_DRY/WET en su instalación real.
 - Ajuste la conversión de lluvia (pct→mm/h) según su sensor.
*/