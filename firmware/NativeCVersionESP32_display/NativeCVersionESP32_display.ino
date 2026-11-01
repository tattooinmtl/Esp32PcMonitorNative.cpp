#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define BLACK 0x0000
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREY 0x7BEF
#define DARKGREY 0x39E7

#define LCD_WIDTH 240
#define LCD_HEIGHT 240

#define LCD_SCLK 7
#define LCD_MOSI 6
#define LCD_DC   15
#define LCD_CS   14
#define LCD_RST  21
#define LCD_BL   22

#define BOOT_BUTTON 9

Arduino_DataBus *bus = new Arduino_ESP32SPI(
  LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, GFX_NOT_DEFINED
);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 0
);

String serialLine = "";

float cpu = 0;
float ram = 0;
float usedGb = 0;
float totalGb = 0;
float diskC = 0;
float netUp = 0;
float netDown = 0;
unsigned long pcUptime = 0;

int page = 1;
bool lastBootState = HIGH;
unsigned long lastButtonMs = 0;
unsigned long lastPacketMs = 0;

float lastCpu = -1;
float lastRam = -1;
float lastDisk = -1;
float lastNet = -1;
float lastUsedGb = -1;
float lastTotalGb = -1;
float lastNetUp = -1;
float lastNetDown = -1;
unsigned long lastPcUptime = 0;

float getVal(String s, const char *key) {
  String k = String(key) + "=";
  int a = s.indexOf(k);
  if (a < 0) return 0;
  a += k.length();
  int b = s.indexOf(",", a);
  if (b < 0) b = s.length();
  return s.substring(a, b).toFloat();
}

void parseLine(String s) {
  s.trim();
  if (!s.startsWith("CPU=")) return;

  cpu = getVal(s, "CPU");
  ram = getVal(s, "RAM");
  usedGb = getVal(s, "USED");
  totalGb = getVal(s, "TOTAL");
  diskC = getVal(s, "DISKC");
  netUp = getVal(s, "UP");
  netDown = getVal(s, "DOWN");
  pcUptime = (unsigned long)getVal(s, "UPTIME");

  lastPacketMs = millis();
}

void header(const char *title) {
  gfx->fillScreen(BLACK);
  gfx->setTextSize(1);
  gfx->setTextColor(CYAN);
  gfx->setCursor(8, 6);
  gfx->print(title);

  gfx->setTextColor(WHITE);
  gfx->setCursor(185, 6);
  gfx->print("P");
  gfx->print(page);
  gfx->print("/6");

  gfx->drawFastHLine(0, 22, 240, BLUE);
}

void donut(int cx, int cy, int r, int thick, float pct, uint16_t color, const char *label) {
  pct = constrain(pct, 0, 100);

  gfx->fillCircle(cx, cy, r, DARKGREY);
  gfx->fillCircle(cx, cy, r - thick, BLACK);

  int segs = map((int)pct, 0, 100, 0, 72);
  for (int i = 0; i < segs; i++) {
    float a = -90 + i * 5;
    float rad = a * DEG_TO_RAD;
    int x1 = cx + cos(rad) * (r - thick / 2);
    int y1 = cy + sin(rad) * (r - thick / 2);
    gfx->fillCircle(x1, y1, thick / 2, color);
  }

  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->setCursor(cx - 24, cy - 8);
  gfx->print((int)pct);
  gfx->print("%");

  gfx->setTextSize(1);
  gfx->setTextColor(color);
  gfx->setCursor(cx - 18, cy + r + 6);
  gfx->print(label);
}

void updateDonut(int cx, int cy, int r, int thick, float pct, uint16_t color, const char *label) {
  gfx->fillCircle(cx, cy, r, DARKGREY);
  gfx->fillCircle(cx, cy, r - thick, BLACK);

  pct = constrain(pct, 0, 100);
  int segs = map((int)pct, 0, 100, 0, 72);

  for (int i = 0; i < segs; i++) {
    float a = -90 + i * 5;
    float rad = a * DEG_TO_RAD;
    int x1 = cx + cos(rad) * (r - thick / 2);
    int y1 = cy + sin(rad) * (r - thick / 2);
    gfx->fillCircle(x1, y1, thick / 2, color);
  }

  gfx->fillRect(cx - 30, cy - 12, 62, 24, BLACK);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->setCursor(cx - 24, cy - 8);
  gfx->print((int)pct);
  gfx->print("%");

  gfx->setTextSize(1);
  gfx->setTextColor(color);
  gfx->setCursor(cx - 18, cy + r + 6);
  gfx->print(label);
}

void resetLastValues() {
  lastCpu = -1;
  lastRam = -1;
  lastDisk = -1;
  lastNet = -1;
  lastUsedGb = -1;
  lastTotalGb = -1;
  lastNetUp = -1;
  lastNetDown = -1;
  lastPcUptime = 0;
}

void pageOverview() {
  header("SYSTEM OVERVIEW");
  donut(62, 70, 38, 12, cpu, GREEN, "CPU");
  donut(178, 70, 38, 12, ram, YELLOW, "RAM");
  donut(62, 162, 38, 12, diskC, MAGENTA, "DISK");
  donut(178, 162, 38, 12, netDown > 999 ? 100 : netDown / 10, CYAN, "NET");
}

void pageCpuRam() {
  header("CPU / RAM");
  donut(120, 80, 52, 14, cpu, GREEN, "CPU");

  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(20, 155);
  gfx->print("RAM ");
  gfx->print(ram, 1);
  gfx->println("%");

  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(20, 185);
  gfx->print(usedGb, 1);
  gfx->print(" GB / ");
  gfx->print(totalGb, 1);
  gfx->println(" GB");
}

void pageDisk() {
  header("DISK / STORAGE");
  donut(120, 100, 60, 16, diskC, MAGENTA, "C:");

  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(45, 185);
  gfx->print("Disk C usage: ");
  gfx->print(diskC, 1);
  gfx->println("%");
}

void pageNetwork() {
  header("NETWORK");

  gfx->setTextSize(2);
  gfx->setTextColor(GREEN);
  gfx->setCursor(25, 65);
  gfx->print("UP ");
  gfx->print(netUp, 0);
  gfx->println(" KB/s");

  gfx->setTextColor(CYAN);
  gfx->setCursor(25, 125);
  gfx->print("DOWN ");
  gfx->print(netDown, 0);
  gfx->println(" KB/s");
}

void pageStatus() {
  header("SERVICE STATUS");

  gfx->setTextSize(2);
  gfx->setTextColor(GREEN);
  gfx->setCursor(25, 65);
  gfx->println("RUNNING");

  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(25, 110);
  gfx->print("Last packet: ");
  gfx->print((millis() - lastPacketMs) / 1000);
  gfx->println(" sec ago");

  gfx->setCursor(25, 135);
  gfx->print("PC uptime: ");
  gfx->print(pcUptime / 3600);
  gfx->println(" h");
}

void pageAbout() {
  header("ESP32 DASHBOARD");

  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  gfx->setCursor(20, 55);
  gfx->println("BOOT button changes pages");
  gfx->setCursor(20, 80);
  gfx->println("RST resets the ESP32");
  gfx->setCursor(20, 115);
  gfx->print("ESP32 heap: ");
  gfx->print(ESP.getFreeHeap());
  gfx->println(" B");
}

void drawPage() {
  resetLastValues();

  if (page == 1) pageOverview();
  else if (page == 2) pageCpuRam();
  else if (page == 3) pageDisk();
  else if (page == 4) pageNetwork();
  else if (page == 5) pageStatus();
  else pageAbout();
}

bool changed(float a, float b, float threshold) {
  return abs(a - b) >= threshold;
}

void updateValuesOnly() {
  if (page == 1) {
    float netPct = netDown > 999 ? 100 : netDown / 10;

    if (changed(cpu, lastCpu, 1.0)) {
      updateDonut(62, 70, 38, 12, cpu, GREEN, "CPU");
      lastCpu = cpu;
    }

    if (changed(ram, lastRam, 1.0)) {
      updateDonut(178, 70, 38, 12, ram, YELLOW, "RAM");
      lastRam = ram;
    }

    if (changed(diskC, lastDisk, 1.0)) {
      updateDonut(62, 162, 38, 12, diskC, MAGENTA, "DISK");
      lastDisk = diskC;
    }

    if (changed(netPct, lastNet, 1.0)) {
      updateDonut(178, 162, 38, 12, netPct, CYAN, "NET");
      lastNet = netPct;
    }
  }
  else if (page == 2) {
    if (changed(cpu, lastCpu, 1.0)) {
      updateDonut(120, 80, 52, 14, cpu, GREEN, "CPU");
      lastCpu = cpu;
    }

    if (changed(ram, lastRam, 0.1) || changed(usedGb, lastUsedGb, 0.1) || changed(totalGb, lastTotalGb, 0.1)) {
      gfx->fillRect(15, 150, 220, 55, BLACK);

      gfx->setTextColor(YELLOW);
      gfx->setTextSize(2);
      gfx->setCursor(20, 155);
      gfx->print("RAM ");
      gfx->print(ram, 1);
      gfx->println("%");

      gfx->setTextSize(1);
      gfx->setTextColor(WHITE);
      gfx->setCursor(20, 185);
      gfx->print(usedGb, 1);
      gfx->print(" GB / ");
      gfx->print(totalGb, 1);
      gfx->println(" GB");

      lastRam = ram;
      lastUsedGb = usedGb;
      lastTotalGb = totalGb;
    }
  }
  else if (page == 3) {
    if (changed(diskC, lastDisk, 1.0)) {
      updateDonut(120, 100, 60, 16, diskC, MAGENTA, "C:");

      gfx->fillRect(40, 180, 170, 20, BLACK);
      gfx->setTextSize(1);
      gfx->setTextColor(WHITE);
      gfx->setCursor(45, 185);
      gfx->print("Disk C usage: ");
      gfx->print(diskC, 1);
      gfx->println("%");

      lastDisk = diskC;
    }
  }
  else if (page == 4) {
    if (changed(netUp, lastNetUp, 1.0) || changed(netDown, lastNetDown, 1.0)) {
      gfx->fillRect(20, 55, 215, 110, BLACK);

      gfx->setTextSize(2);
      gfx->setTextColor(GREEN);
      gfx->setCursor(25, 65);
      gfx->print("UP ");
      gfx->print(netUp, 0);
      gfx->println(" KB/s");

      gfx->setTextColor(CYAN);
      gfx->setCursor(25, 125);
      gfx->print("DOWN ");
      gfx->print(netDown, 0);
      gfx->println(" KB/s");

      lastNetUp = netUp;
      lastNetDown = netDown;
    }
  }
  else if (page == 5) {
    unsigned long age = (millis() - lastPacketMs) / 1000;
    unsigned long upH = pcUptime / 3600;

    if (age != lastPcUptime) {
      gfx->fillRect(20, 105, 210, 45, BLACK);

      gfx->setTextSize(1);
      gfx->setTextColor(WHITE);
      gfx->setCursor(25, 110);
      gfx->print("Last packet: ");
      gfx->print(age);
      gfx->println(" sec ago");

      gfx->setCursor(25, 135);
      gfx->print("PC uptime: ");
      gfx->print(upH);
      gfx->println(" h");

      lastPcUptime = age;
    }
  }
  else if (page == 6) {
    gfx->fillRect(20, 110, 210, 20, BLACK);
    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(20, 115);
    gfx->print("ESP32 heap: ");
    gfx->print(ESP.getFreeHeap());
    gfx->println(" B");
  }
}

void checkButton() {
  bool bootState = digitalRead(BOOT_BUTTON);

  if (lastBootState == HIGH && bootState == LOW && millis() - lastButtonMs > 250) {
    lastButtonMs = millis();
    page++;
    if (page > 6) page = 1;
    drawPage();
  }

  lastBootState = bootState;
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_BUTTON, INPUT_PULLUP);

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  gfx->begin();
  lastPacketMs = millis();
  drawPage();
}

void loop() {
  checkButton();

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      parseLine(serialLine);
      serialLine = "";
      updateValuesOnly();
    } else if (c != '\r') {
      serialLine += c;
      if (serialLine.length() > 180) serialLine = "";
    }
  }
}