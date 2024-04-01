int PIN_BUTTON = 11;
int PIN_MAIN_LED = 13;
int PIN_RED_LED = 12;
int PIN_SPEAKER = 8;
int PIN_ULTRASONIC_ECHO = 9;
int PIN_ULTRASONIC_TRIG = 10;

void setup() {
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_MAIN_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_SPEAKER, OUTPUT);
  pinMode(PIN_ULTRASONIC_ECHO, INPUT);
  pinMode(PIN_ULTRASONIC_TRIG, OUTPUT);
  // Serial.begin(9600);

  tone(PIN_SPEAKER, 1000);
  digitalWrite(PIN_RED_LED, HIGH);
  delay(200);
  noTone(PIN_SPEAKER);
  digitalWrite(PIN_RED_LED, LOW);
  delay(1000);
}

bool signalizationOn = false;
bool redLedIsOn = false;
bool fail_defuse = false;
int frequencies[] = {200};//{2000, 1800};
const int quietFreq = 500;
const int loudFreq = 1500;
const int veryLoudFreq = 1800;
const int shortSignal = 200;
const int middleSignal = 500;
const int afterShortSignal = shortSignal + 100;
int delayPlayFrequencyMS = 400;
unsigned long signalizationOnFutureMillis;
unsigned long signalizationCheckButtonMillis = 0;
void loop() {
  // digitalWrite(PIN_MAIN_LED, signalizationOn);
  if (signalizationOn) {
    // Serial.print("1) "); Serial.println(signalizationCheckButtonMillis);
    // Serial.print("2) "); Serial.println((digitalRead(PIN_BUTTON) == 1));
    // перед тем как "вывести сигнализацию", проверим, не пора ли нам её остановить, а то получится не нужный гудок
    if (signalizationCheckButtonMillis != 0 && (millis() > signalizationCheckButtonMillis)) {
      if (digitalRead(PIN_BUTTON) == 1) {
        digitalWrite(PIN_RED_LED, LOW);
        signalizationOn = false;
        fail_defuse = false;
        signalizationCheckButtonMillis = 0;
        tone(PIN_SPEAKER, quietFreq, shortSignal);
        delay(3000);
        tone(PIN_SPEAKER, quietFreq, middleSignal);
        return;
      } else fail_defuse = true;
    }
    // Serial.print("3) "); Serial.println((millis() > signalizationOnFutureMillis));
    // Serial.print("4) "); Serial.println((millis() > signalizationCheckButtonMillis));
    // Serial.print("5) "); Serial.println(fail_defuse);
    if (millis() > signalizationOnFutureMillis  // если истек кулдаун до активации
        && (millis() > signalizationCheckButtonMillis  // если кнопку нажали (запланировано отключение сигнализации)
        || fail_defuse))  // но если уже попытались выключить сигнализацию (неудачно), то сигналы всё равно будут
      for (int freq : frequencies) {
        digitalWrite(PIN_RED_LED, redLedIsOn);
        tone(PIN_SPEAKER, freq);
        delay(delayPlayFrequencyMS);
        noTone(PIN_SPEAKER);
        redLedIsOn = !redLedIsOn;
      }
    // Serial.print("6) "); Serial.println(digitalRead(PIN_BUTTON) == 1);
    // Serial.print("7) "); Serial.println((millis() > signalizationCheckButtonMillis));
    if (digitalRead(PIN_BUTTON) == 1) {
      if (millis() > signalizationCheckButtonMillis) {  // если кнопку нажали (запланировано отключение сигнализации), то необрабатываем её второй раз
        if (!fail_defuse) {
          redLedIsOn = false;
          digitalWrite(PIN_RED_LED, LOW);
          tone(PIN_SPEAKER, quietFreq, shortSignal);
        }
        signalizationCheckButtonMillis = millis() + 5000;
      }
    }
    delay(50);
    return;
  }
  digitalWrite(PIN_ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRASONIC_TRIG, LOW);
  int duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH);
  float cm = duration / 58;
  if (cm < 15) {
    signalizationOn = true;
    signalizationOnFutureMillis = millis() + 10000;
  }
  if (digitalRead(PIN_BUTTON) == 1) {
    tone(PIN_SPEAKER, loudFreq, shortSignal);
    delay(afterShortSignal);
    for (int s10 = 0; s10 < 3; s10++) {
      tone(PIN_SPEAKER, loudFreq, shortSignal);
      delay(afterShortSignal);
      for (int second = 0; second < 10; second += 2) {
        delay(2000);
        if (digitalRead(PIN_BUTTON) == 1) {
          tone(PIN_SPEAKER, loudFreq, middleSignal);
          delay(2000);
          return;
        }
      }
    }
    tone(PIN_SPEAKER, veryLoudFreq, middleSignal);
    digitalWrite(PIN_RED_LED, LOW);
  }
  delay(100);
}
