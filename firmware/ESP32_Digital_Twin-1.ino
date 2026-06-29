/*
 * === ESP32 Resource-Constrained Real-Time Digital Twin of LM2596 Buck ===
 * Author: Ranjith Sharan J.
 * Version: Final Stable (Adaptive + Overflow-Protected)
 */

#include <Arduino.h>

// --- Pin Configuration ---
const int PIN_VIN  = 34;
const int PIN_VOUT = 35;
const int PIN_IOUT = 32;

// --- Voltage Dividers ---
const float VIN_R_TOP = 10000.0f, VIN_R_BOTTOM = 3300.0f;
const float VOUT_R_TOP = 2300.0f, VOUT_R_BOTTOM = 3300.0f;
const float CURR_R_TOP = 2300.0f, CURR_R_BOTTOM = 3300.0f;

// --- Calibration ---
const float VIN_CAL = 0.972f;
const float VOUT_CAL = 1.000f;

// --- ACS712-20A ---
const float ACS_SENSITIVITY = 0.100f;  // 100 mV/A
float g_acs_offset_mv = 0.0f;
const float CURRENT_SIGN = +1.0f;
const float I_GAIN = 1.00f;

// --- Twin Model ---
float L_H = 33e-6f;
float C_F = 220e-6f;
float rL  = 0.08f;
float R_Ohm = 10.0f;     // adaptive load estimate
float iL = 0.0f;
float vC = 0.0f;

// --- Timing ---
const float Ts = 0.001f;   // 1 ms internal simulation step
unsigned long lastPrint = 0;

// --- Residual thresholds ---
const float RES_V_ABS_TH = 0.25f;
const float RES_I_ABS_TH = 0.15f;

// === Helper Functions ===
float readAvgmV(int pin, int n = 16) {
  long sum = 0;
  for (int i = 0; i < n; i++) sum += analogReadMilliVolts(pin);
  return (float)sum / n;
}

void autoZeroACS() {
  Serial.println("\n[Zero] Ensure NO LOAD on 5 V output...");
  const int N = 300;
  float acc = 0;
  for (int i = 0; i < N; i++) {
    float mv = readAvgmV(PIN_IOUT);
    float v  = mv / 1000.0f;
    float v_acs = v * (CURR_R_TOP + CURR_R_BOTTOM) / CURR_R_BOTTOM;
    acc += v_acs * 1000.0f;
    delay(10);
  }
  g_acs_offset_mv = acc / N;
  Serial.print("[Zero] ACS offset = ");
  Serial.print(g_acs_offset_mv, 2);
  Serial.println(" mV");
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  delay(500);

  Serial.println("\n=== ESP32 Real-Time Digital Twin of LM2596 Buck ===");
  Serial.println("Project by Ranjith Sharan J.");
  Serial.println("----------------------------------------------------");
  autoZeroACS();

  // --- Initialize Twin with actual measured steady values ---
  float vin_mV = readAvgmV(PIN_VIN);
  float vout_mV = readAvgmV(PIN_VOUT);
  float i_mV = readAvgmV(PIN_IOUT);

  float Vin = (vin_mV / 1000.0f) * (VIN_R_TOP + VIN_R_BOTTOM) / VIN_R_BOTTOM * VIN_CAL;
  float Vout = (vout_mV / 1000.0f) * (VOUT_R_TOP + VOUT_R_BOTTOM) / VOUT_R_BOTTOM * VOUT_CAL;
  float Vacs = (i_mV / 1000.0f) * (CURR_R_TOP + CURR_R_BOTTOM) / CURR_R_BOTTOM;
  float I_meas = I_GAIN * CURRENT_SIGN *
                 (Vacs * 1000.0f - g_acs_offset_mv) / (ACS_SENSITIVITY * 1000.0f);

  vC = Vout;
  iL = I_meas;

  Serial.println("[Init] Twin synced with real converter.\n");
  Serial.println("t_ms,Vin,Vout_meas,I_meas,vC_pred,iL_pred,resV,resI");
}

// === Main Loop ===
void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'z' || c == 'Z') autoZeroACS();
  }

  // Print once per second (readable)
  if (millis() - lastPrint < 1000) return;
  lastPrint = millis();

  // --- Sensor Readings ---
  float vin_mV  = readAvgmV(PIN_VIN);
  float vout_mV = readAvgmV(PIN_VOUT);
  float i_mV    = readAvgmV(PIN_IOUT);

  float Vin  = (vin_mV / 1000.0f) * (VIN_R_TOP + VIN_R_BOTTOM) / VIN_R_BOTTOM * VIN_CAL;
  float Vout = (vout_mV / 1000.0f) * (VOUT_R_TOP + VOUT_R_BOTTOM) / VOUT_R_BOTTOM * VOUT_CAL;
  float Vacs = (i_mV / 1000.0f) * (CURR_R_TOP + CURR_R_BOTTOM) / CURR_R_BOTTOM;
  float I_meas = I_GAIN * CURRENT_SIGN *
                 (Vacs * 1000.0f - g_acs_offset_mv) / (ACS_SENSITIVITY * 1000.0f);

  // --- Estimate duty cycle safely ---
  float d = 0.0f;
  if (Vin > 0.1f) d = constrain(Vout / Vin, 0.0f, 0.98f);

  // --- Adaptive load estimation (avoid div by zero) ---
  if (fabs(I_meas) > 0.01f)
    R_Ohm = constrain(Vout / fabs(I_meas), 5.0f, 200.0f);

  // --- Run internal twin simulation (10 safe steps) ---
  for (int k = 0; k < 10; k++) {
    float diL = ((d * Vin) - vC - rL * iL) / L_H;
    float dvC = (iL - vC / R_Ohm) / C_F;

    // Limit physically plausible rates
    diL = constrain(diL, -1e5f, 1e5f);   // ≤100 kA/s
    dvC = constrain(dvC, -1e4f, 1e4f);   // ≤10 kV/s

    iL += Ts * diL;
    vC += Ts * dvC;

    // Self-healing if diverged
    if (!isfinite(iL)) iL = I_meas;
    if (!isfinite(vC)) vC = Vout;
  }

  // --- Residuals ---
  float resV = Vout - vC;
  float resI = I_meas - iL;

  // --- Print results ---
  Serial.print(millis()); Serial.print(",");
  Serial.print(Vin,3); Serial.print(",");
  Serial.print(Vout,3); Serial.print(",");
  Serial.print(I_meas,3); Serial.print(",");
  Serial.print(vC,3); Serial.print(",");
  Serial.print(iL,3); Serial.print(",");
  Serial.print(resV,3); Serial.print(",");
  Serial.println(resI,3);
  Serial.flush();

  // --- Anomaly detection ---
  if (fabs(resV) > RES_V_ABS_TH || fabs(resI) > RES_I_ABS_TH) {
    Serial.println("[ANOMALY] Mismatch detected!\n");
  }
}
