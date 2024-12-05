#include <CapacitiveSensor.h>
#include <Adafruit_NeoPixel.h>

#define LED_COUNT 60
#define LED_PIN 7
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

long time = 0;
int state = HIGH;

boolean yes;
boolean previous = false;

int debounce = 100;
unsigned long lastUpdate = 0;  // For non-blocking delays
unsigned long interval = 10;   // Default interval for pixel updates
long previousReading = 0;

// 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
CapacitiveSensor cs_4_2 = CapacitiveSensor(4, 2);
int currentMode = 0;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  strip.begin();
  strip.setBrightness(127);
  strip.show();
  strip.clear();
}

void loop() {
  // put your main code here, to run repeatedly:
  tapCap();
  switch (currentMode) {
    case 0:
      strip.clear();
      off();
      break;
    case 1:
      colorWipeNonBlocking(strip.Color(255, 0, 0));  // Red
      break;
    case 2:
      colorWipeNonBlocking(strip.Color(0, 255, 0));  // Green
      break;
    case 3:
      colorWipeNonBlocking(strip.Color(0, 0, 255));  // Blue
      break;
    case 4:
      rainbowNonBlocking();  // Flowing rainbow cycle along the whole strip
      break;
    case 5:
      theaterChaseRainbowNonBlocking();  // Rainbow-enhanced theaterChase variant
      break;
  }
}

void tapCap() {
  long total1 = cs_4_2.capacitiveSensor(30);  // Current reading from the capacitive sensor

  // Check if the current reading is above the threshold, and the previous reading was not 0
  if (total1 > 300 && previousReading > 0) {
    strip.clear();
    currentMode += 1;  // Increment the mode
    if (currentMode >= 6) {
      currentMode = 0;  // Loop back to mode 0 after mode 4
    }
  }

  // Print current mode and update the previous reading
  Serial.print("Current Reading: ");
  Serial.print(total1);
  Serial.print(" | Previous Reading: ");
  Serial.print(previousReading);
  Serial.print(" | Current Mode: ");
  Serial.println(currentMode);

  previousReading = total1;  // Update the previous reading

  // Debounce logic for toggling state
  if (yes == true && previous == false && millis() - time > debounce) {
    state = !state;
    time = millis();
  }

  digitalWrite(LED_PIN, state);
  previous = yes;
}

void off() {
  for (int i = 0; i <= strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void colorWipeNonBlocking(uint32_t color) {
  static int i = 0;  // Keep track of the current pixel
  if (millis() - lastUpdate > interval) {
    lastUpdate = millis();
    if (i < strip.numPixels()) {
      strip.setPixelColor(i, color);  // Set pixel's color (in RAM)
      strip.show();                   // Update strip to match
      i++;
    } else {
      strip.clear();
      i = 0;  // Reset for the next wipe
    }
  }
}

void rainbowNonBlocking() {
  static long firstPixelHue = 0;  // Keep track of the hue
  if (millis() - lastUpdate > interval) {
    lastUpdate = millis();
    firstPixelHue += 256;  // Adjust hue for the next frame
    if (firstPixelHue >= 5 * 65536) {
      firstPixelHue = 0;  // Reset after one full cycle
    }
    strip.rainbow(firstPixelHue);
    strip.show();
  }
}

void theaterChaseRainbowNonBlocking() {
  static int a = 0, b = 0;
  static int firstPixelHue = 0;

  if (millis() - lastUpdate > interval) {
    lastUpdate = millis();
    strip.clear();
    for (int c = b; c < strip.numPixels(); c += 3) {
      int hue = firstPixelHue + c * 65536L / strip.numPixels();
      uint32_t color = strip.gamma32(strip.ColorHSV(hue));  // hue -> RGB
      strip.setPixelColor(c, color);
    }
    strip.show();

    b = (b + 1) % 3;  // Cycle b through 0, 1, 2
    if (b == 0) {
      a++;
      firstPixelHue += 65536 / 90;
    }
    if (a >= 30) {
      a = 0;  // Reset after completing 30 cycles
    }
  }
}
