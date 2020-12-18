#include <SD.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <FastLED.h>
#define DATA_PIN 6

#define NEXT_PIN 5
#define PREV_PIN 4
#define SD_PIN 10
#define NUM_LEDS 95
#define SIZE NUM_LEDS*3
#define BUTTON_DELAY 250

const char fileName[12];
CRGB leds[NUM_LEDS];

bool prevPushed = false;
bool nextPushed = false;

int dirPos = 0;
int fileCount = 0;
long lineLen = SIZE/3;
long lineN = 0;
int baseTime = 0;
unsigned long nextChange = 0;
File root;
File selectedFile;

void setup() {

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_PIN)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");
    while (true);
  }
   
  pinMode(NEXT_PIN, INPUT_PULLUP);
  pinMode(PREV_PIN, INPUT_PULLUP);
  Serial.println("Initialization done.");
  root = SD.open("/");
  
  root.rewindDirectory();
  while (true){
    selectedFile.close();
    selectedFile = root.openNextFile();
    if (!selectedFile) break;
    if (!selectedFile.isDirectory())
      fileCount++;
  }
  loadSelectedFile();
  
  FastLED.addLeds<UCS1903, DATA_PIN, BRG>(leds, NUM_LEDS);
}

void loop() {
  bool next = !digitalRead(NEXT_PIN);
  bool prev = !digitalRead(PREV_PIN);

  if (prev) {
    if (!prevPushed) {
      Serial.println("PREV is down");
      dirPos = max(0, dirPos-1);
      prevPushed = true;
      delay(BUTTON_DELAY);
      loadSelectedFile();
    }
  } else {
    prevPushed = false;
  }

  if (next) {
    if (!nextPushed) {
      Serial.println("NEXT is down");
      dirPos = min(fileCount-1, dirPos+1);
      nextPushed = true;
      delay(BUTTON_DELAY);
      loadSelectedFile();
    }
  } else {
    nextPushed = false;
  }

  if (millis() >= nextChange) {
    readNextLine();
    FastLED.show(); 
  }
}

void readNextLine() {
  if (1+lineN*(lineLen*3+1) > selectedFile.size())
    lineN = 0;
  selectedFile.seek(1+lineN*(lineLen*3+1));
  nextChange = millis()+(baseTime*(int)selectedFile.read())/255;
  memset(leds, 0, SIZE);
  int len = (SIZE < (lineLen*3) ? SIZE : (lineLen*3));
  selectedFile.read(leds, len);
  lineN++;
}

void loadSelectedFile() {
  root.rewindDirectory();
  for (int i = 0; i <= dirPos; i++) {
    selectedFile.close();
    selectedFile = root.openNextFile();
    if (selectedFile.isDirectory())
      i--;
  }
  lineN = 0;
  Serial.println(selectedFile.name());
  baseTime = (selectedFile.name()[5]-'0')*100+(selectedFile.name()[6]-'0')*10+(selectedFile.name()[7]-'0');
  lineLen = (int)selectedFile.read();
}
