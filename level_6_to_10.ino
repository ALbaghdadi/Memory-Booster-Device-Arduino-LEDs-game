#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2); // شاشة 16x2

// === تعريف LEDs RGB ===
int leds[5][3] = {
  {5,12,1},   // LED1
  {2,3,4},    // LED2
  {8,6,9},    // LED3
  {10,11,0},  // LED4
  {7,14,13}   // LED5
};

// === أزرار المستخدم ===
int buttons[3] = {15,16,17}; // Button1,2,3
int buttonA6 = A6;           // Button A6 → LED2
int buttonA7 = A7;           // Button A7 → LED3

// === تعريف الألوان ===
// درجات PWM لكل لون لتحسين الأصفر
int colorRed[3]    = {255,0,0};
int colorGreen[3]  = {0,255,0};
int colorBlue[3]   = {0,0,255};
int colorYellow[3] = {255,90,0}; // أصفر أفضل
int colorWhite[3]  = {255,255,255};

// ألوان الأزرار للمستوى السادس
int buttonColors[5][3] = {
  {255,255,255},  // Button1 → White
  {0,255,0},      // Button2 → Green
  {0,0,255},      // Button3 → Blue
  {255,0,0},      // Button4 → Red
  {255,180,0}     // Button5 → Yellow
};

// === وظائف التحكم بالـLEDs ===
void setLED_PWM(int ledNumber,int color[3]){
  if(ledNumber<1 || ledNumber>5) return;
  int index = ledNumber-1;
  analogWrite(leds[index][0], color[0]); // R
  analogWrite(leds[index][1], color[1]); // G
  analogWrite(leds[index][2], color[2]); // B
}

void offLED(int ledNumber){
  if(ledNumber<1 || ledNumber>5) return;
  int index = ledNumber-1;
  analogWrite(leds[index][0],0);
  analogWrite(leds[index][1],0);
  analogWrite(leds[index][2],0);
}

// قراءة الزر المضغوط → رقم LED الصحيح
int readButton(){
  if(digitalRead(buttons[0])==HIGH) return 5; // Button1 → LED5
  if(digitalRead(buttons[1])==HIGH) return 4; // Button2 → LED4
  if(digitalRead(buttons[2])==HIGH) return 1; // Button3 → LED1
  if(analogRead(buttonA6)>=500) return 2;      // A6 → LED2
  if(analogRead(buttonA7)>=500) return 3;      // A7 → LED3
  return 0;
}

// === المتغيرات الأساسية ===
int level = 1;
int maxLevel = 10;

// تسلسل LEDs لكل مستوى (1-5) — 0 يعني فارغ
int levelSequences[5][5] = {
  {1,2,3,0,0},     // Level1
  {1,2,3,4,0},     // Level2
  {1,2,3,4,5},     // Level3
  {1,4,2,3,5},     // Level4
  {0,0,0,0,0}      // Level5 → عشوائي
};

// === دوال اللعبة ===
int sequence[5];
int userInput[5];
int sequenceLength;

// إعداد مستوى
void prepareLevel(){
  if(level<6){ // مستويات 1-5
    for(int i=0;i<5;i++) sequence[i] = levelSequences[level-1][i];
    sequenceLength = 0;
    for(int i=0;i<5;i++) if(sequence[i]!=0) sequenceLength++;
  }
}

// عرض تسلسل LEDs (أحمر للمستويات 1-5)
void showSequence(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Level ");
  lcd.print(level);
  lcd.setCursor(0,1);
  lcd.print("Watch LEDs");
  delay(1000);

  for(int i=0;i<sequenceLength;i++){
    setLED_PWM(sequence[i], colorRed);
    delay(400);
    offLED(sequence[i]);
    delay(200);
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Level ");
  lcd.print(level);
  lcd.setCursor(0,1);
  lcd.print("Your turn");
}

// تحقق إجابة المستخدم للمستوى 1-5
bool getUserInput(){
  for(int i=0;i<sequenceLength;i++){
    bool pressed=false;
    while(!pressed){
      int btn = readButton();
      if(btn){
        pressed=true;
        userInput[i]=btn;
        setLED_PWM(btn, colorRed);
        delay(200);
        offLED(btn);
        while(readButton()==btn);
      }
    }
  }
  for(int i=0;i<sequenceLength;i++){
    if(userInput[i]!=sequence[i]) return false;
  }
  return true;
}

// === المستوى السادس: ألوان عشوائية في أي LED ===
void playLevel6() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Level 6");
  delay(1000);

  for(int numLEDs=1; numLEDs<=5; numLEDs++){
    int ledSeq[5];
    int ledColors[5][3];
    int usedColors[5] = {0,0,0,0,0};

    // توليد تسلسل ألوان عشوائية لأماكن LEDs مختلفة
    for(int i=0;i<numLEDs;i++){
      int ledIdx = random(1,6);      // LED أي رقم من 1 إلى 5
      ledSeq[i] = ledIdx;

      int colorIdx;
      do { colorIdx = random(0,5); } while(usedColors[colorIdx]);
      usedColors[colorIdx]=1;

      for(int c=0;c<3;c++)
        ledColors[i][c] = buttonColors[colorIdx][c];
    }

    // عرض الـLEDs مع الألوان
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Watch Colors");
    for(int i=0;i<numLEDs;i++){
      setLED_PWM(ledSeq[i], ledColors[i]);
      delay(600);
      offLED(ledSeq[i]);
      delay(200);
    }

    // إدخال المستخدم ومقارنته بالألوان
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Press Colors");

    for(int i=0;i<numLEDs;i++){
      bool pressed=false;
      while(!pressed){
        int btn = readButton();
        if(btn){
          pressed=true;
          setLED_PWM(ledSeq[i], ledColors[i]); // اضاءة LED عند الضغط
          delay(200);
          offLED(ledSeq[i]);

          // تحقق اللون
          bool match=true;
          for(int c=0;c<3;c++){
            if(buttonColors[btn-1][c] != ledColors[i][c]){
              match=false;
              break;
            }
          }

          if(!match){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Wrong! Level1");
            delay(1500);
            level=1;
            return; // خطأ → العودة للمستوى الأول
          }

          while(readButton()==btn); // انتظار رفع الزر
        }
      }
    }

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Correct!");
    delay(500);
  }

  level=7; // بعد إتمام المستوى السادس
}

// === setup ===
void setup(){
  lcd.begin();
  lcd.backlight();

  for(int i=0;i<5;i++){
    for(int j=0;j<3;j++){
      pinMode(leds[i][j], OUTPUT);
      analogWrite(leds[i][j], 0); // استخدم PWM
    }
  }
  for(int i=0;i<3;i++) pinMode(buttons[i], INPUT);
  pinMode(buttonA6, INPUT);
  pinMode(buttonA7, INPUT);

  randomSeed(analogRead(0));

  prepareLevel();
  showSequence();
}

// === loop ===
void loop(){
  if(level<6){
    if(getUserInput()){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Correct!");
      delay(1000);
      level++;
      if(level>10) level=1;
    } else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Wrong! Level1");
      delay(1500);
      level=1;
    }
    prepareLevel();
    showSequence();
  } else if(level==6){
    playLevel6();
  } else {
    // مستويات 7-10: LED أحمر عشوائي
    int numLEDs = random(3,6); // 3 إلى 5 أضواء
    for(int i=0;i<numLEDs;i++) sequence[i]=random(1,6);
    sequenceLength=numLEDs;
    showSequence();
    if(getUserInput()){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Correct!");
      delay(1000);
      level++;
      if(level>10) level=1;
    } else {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Wrong! Level1");
      delay(1500);
      level=1;
    }
  }
}
