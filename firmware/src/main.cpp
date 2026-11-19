#include <M5Unified.h>

// M5StickC Plus shake detector (built-in MPU6886 IMU). Sends one line per shake to the browser.
static float previousX = 0, previousY = 0, previousZ = 0;
static bool havePrevious = false;
static bool armed = true;
static uint32_t lastShakeMs = 0;
static uint32_t quietSinceMs = 0;

// Tune on the actual device if necessary. Units are approximately g.
constexpr float SHAKE_DELTA_THRESHOLD = 1.25f;
constexpr float QUIET_DELTA_THRESHOLD = 0.30f;
constexpr uint32_t SHAKE_COOLDOWN_MS = 900;
constexpr uint32_t QUIET_REARM_MS = 350;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  delay(300);
  M5.Display.setRotation(1);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(8, 12);
  M5.Display.println("Labu Dice Ready");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, 42);
  M5.Display.println("M5StickC Plus / MPU6886");
  M5.Display.setCursor(8, 60);
  M5.Display.println("Shake to roll the dice");
  Serial.println("LABU_DICE_READY");
}

void loop() {
  M5.update();
  if (!M5.Imu.isEnabled()) {
    M5.Display.fillRect(0, 88, M5.Display.width(), 24, BLACK);
    M5.Display.setCursor(8, 88);
    M5.Display.println("IMU unavailable");
    delay(500);
    return;
  }

  float x = 0, y = 0, z = 0;
  if (!M5.Imu.getAccelData(&x, &y, &z)) {
    delay(20);
    return;
  }

  if (!havePrevious) {
    previousX = x; previousY = y; previousZ = z;
    havePrevious = true;
    delay(20);
    return;
  }

  const float dx = x - previousX;
  const float dy = y - previousY;
  const float dz = z - previousZ;
  const float delta = sqrtf(dx * dx + dy * dy + dz * dz);
  previousX = x; previousY = y; previousZ = z;
  const uint32_t now = millis();

  if (delta < QUIET_DELTA_THRESHOLD) {
    if (quietSinceMs == 0) quietSinceMs = now;
    if (!armed && now - quietSinceMs >= QUIET_REARM_MS) armed = true;
  } else {
    quietSinceMs = 0;
  }

  if (armed && delta >= SHAKE_DELTA_THRESHOLD && now - lastShakeMs >= SHAKE_COOLDOWN_MS) {
    Serial.println("SHAKED");
    lastShakeMs = now;
    armed = false;
    quietSinceMs = 0;
    M5.Display.fillRect(0, 88, M5.Display.width(), 30, BLACK);
    M5.Display.setCursor(8, 90);
    M5.Display.setTextSize(2);
    M5.Display.println("SHAKED!");
  }
  delay(20);
}
