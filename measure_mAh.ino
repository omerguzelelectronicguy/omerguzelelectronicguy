#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Pin definitions
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 6, 7, 10); // CLK, DIN, DC, CE, RST
#define LCD_BL 5 // Backlight pin (moved to D5)
#define BUZZER_PIN 2 // Buzzer pin
#define BUTTON_PIN 3 // Button pin

// Variables for measurements
float total_mAh = 0.0; // Accumulated mAh
unsigned long last_time = 0; // Last measurement time (ms)
int low_voltage_count = 0; // Counter for consecutive low voltage readings
const float VOLTAGE_CORRECTION = 4.125 / 4.35; // Correction factor for voltage
float shunt_resistor = 10.0; // Initial shunt resistor (ohms, toggles between 10 and 27)

// Button state
bool last_button_state = HIGH; // Initial state (HIGH due to pull-up)
unsigned long last_debounce_time = 0; // For debouncing
const unsigned long DEBOUNCE_DELAY = 50; // Debounce time in ms

// Buffer for storing last 10 voltage measurements
#define BUFFER_SIZE 10
float voltage_buffer[BUFFER_SIZE]; // Array to store voltage readings
int buffer_index = 0; // Current index in buffer
int buffer_count = 0; // Number of valid readings in buffer

// Function to read and filter voltage using median of last 10 measurements
float readFilteredVoltage() {
  // Read single ADC value
  int raw_adc = analogRead(A0); // Read raw ADC value
  float voltage_arduino = (raw_adc / 1023.0) * 5.0; // Convert to Arduino voltage
  float voltage_real = voltage_arduino * VOLTAGE_CORRECTION; // Apply correction

  // Add to buffer
  voltage_buffer[buffer_index] = voltage_real;
  buffer_index = (buffer_index + 1) % BUFFER_SIZE; // Circular buffer
  if (buffer_count < BUFFER_SIZE) buffer_count++; // Increment until buffer is full

  // Copy buffer to temporary array for sorting
  float sorted_buffer[BUFFER_SIZE];
  for (int i = 0; i < buffer_count; i++) {
    sorted_buffer[i] = voltage_buffer[i];
  }

  // Sort the readings (bubble sort)
  for (int i = 0; i < buffer_count - 1; i++) {
    for (int j = 0; j < buffer_count - i - 1; j++) {
      if (sorted_buffer[j] > sorted_buffer[j + 1]) {
        float temp = sorted_buffer[j];
        sorted_buffer[j] = sorted_buffer[j + 1];
        sorted_buffer[j + 1] = temp;
      }
    }
  }

  // Return median (middle value)
  return sorted_buffer[buffer_count / 2];
}

void setup() {
  // Initialize backlight
  pinMode(LCD_BL, OUTPUT);
  analogWrite(LCD_BL, 150); // Set backlight brightness (0-255)

  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Buzzer off initially

  // Initialize button with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize serial (for debugging)
  Serial.begin(9600);

  // Initialize display
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);

  // Initialize voltage buffer
  for (int i = 0; i < BUFFER_SIZE; i++) {
    voltage_buffer[i] = 0.0; // Clear buffer
  }
}

void loop() {
  // Handle button press to toggle shunt resistor
  bool current_button_state = digitalRead(BUTTON_PIN);
  if (current_button_state != last_button_state && (millis() - last_debounce_time) > DEBOUNCE_DELAY) {
    if (current_button_state == LOW) { // Button pressed (LOW due to pull-up)
      shunt_resistor = (shunt_resistor == 10.0) ? 27.0 : 10.0; // Toggle between 10 and 27 ohms
      Serial.print("Shunt Resistor Changed to: "); Serial.println(shunt_resistor);
    }
    last_debounce_time = millis(); // Update debounce time
  }
  last_button_state = current_button_state;

  // Read and correct voltage from A0
  float voltage_real = readFilteredVoltage(); // Use filtered voltage

  // Calculate current (I = V/R)
  float current_mA = (voltage_real / shunt_resistor) * 1000.0; // Current in mA

  // Calculate mAh (Δt = 2 seconds)
  unsigned long current_time = millis();
  if (current_time - last_time >= 2000) { // Ensure 2-second interval
    float delta_t = (current_time - last_time) / 1000.0; // Time in seconds
    total_mAh += (current_mA * delta_t) / 3600.0; // mAh = (mA * s) / 3600
    last_time = current_time;
  }

  // Buzzer logic: activate if 10 consecutive readings < 3.2V
  if (voltage_real < 3.2) {
    low_voltage_count++;
    if (low_voltage_count >= 10) {
      tone(BUZZER_PIN, 1000); // 1kHz tone
    }
  } else {
    low_voltage_count = 0; // Reset counter if voltage >= 3.2V
    noTone(BUZZER_PIN); // Turn off buzzer
  }

  // Update display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("time ");
  display.println(current_time / 1000); // Time in seconds
  display.setCursor(0, 8);
  display.print("volt ");
  display.println(voltage_real, 2); // Display corrected voltage (2 decimals)
  display.setCursor(0, 16);
  display.print("curr ");
  display.println(current_mA, 2); // Display current in mA (2 decimals)
  display.setCursor(0, 24);
  display.print("mAh ");
  display.println(total_mAh, 2); // Display total mAh (2 decimals)
  display.setCursor(0, 32);
  display.print("R: ");
  display.print(shunt_resistor, 0); // Display shunt resistor (no decimals)
  display.println(" ohm");
  display.display();

  // Debug output
  Serial.print("Real V: "); Serial.print(voltage_real, 2);
  Serial.print(" | Current mA: "); Serial.print(current_mA, 2);
  Serial.print(" | mAh: "); Serial.print(total_mAh, 2);
  Serial.print(" | Shunt R: "); Serial.println(shunt_resistor, 0);

  delay(2000); // Wait 2 seconds for next measurement
}