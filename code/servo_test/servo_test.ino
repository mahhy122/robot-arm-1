#include <Servo.h>

// 滑らかに目標角度へ向かうサーボクラス
class SmoothServo {
  private:
    Servo servo;
    int pin;
    unsigned long previousMillis;
    int updateInterval; // 速度（何msごとにパルスを変化させるか）

    int currentPulse; // 現在のパルス幅
    int targetPulse;  // 目標のパルス幅

  public:
    // コンストラクタ：接続ピンと、動くスピードを指定
    SmoothServo(int attachPin, int speedInterval) {
      pin = attachPin;
      updateInterval = speedInterval;
      previousMillis = 0;
      currentPulse = 1500; // 初期位置(90度付近)
      targetPulse = 1500;
    }

    void begin() {
      servo.attach(pin, 500, 2500);
      servo.writeMicroseconds(currentPulse);
    }

    // 0〜270度の角度を指定して、目標パルスをセットする関数
    void setTargetAngle(int angle) {
      // 指定範囲外の異常な数字弾く（安全対策）
      angle = constrain(angle, 0, 270);
      targetPulse = map(angle, 0, 270, 500, 2500);
    }

    // メインループで常に呼ぶ更新処理
    void update() {
      unsigned long currentMillis = millis();
      
      if (currentMillis - previousMillis >= updateInterval) {
        previousMillis = currentMillis;

        // 目標パルスに向かって少しずつ現在パルスを近づける（滑らか移動）
        if (currentPulse < targetPulse) {
          currentPulse += 5; // 1回の変化量。大きくすると速く、小さくすると遅くなる
          if(currentPulse > targetPulse) currentPulse = targetPulse;
          servo.writeMicroseconds(currentPulse);
        } else if (currentPulse > targetPulse) {
          currentPulse -= 5;
          if(currentPulse < targetPulse) currentPulse = targetPulse;
          servo.writeMicroseconds(currentPulse);
        }
      }
    }
};

// 【ここがクラスの魔法】
// ピン番号とスピードを指定して、2つのサーボを実体化するだけ！
SmoothServo joint1(2, 5); // 2番ピン、更新間隔5ms（速め）
SmoothServo joint2(3, 10); // 3番ピン、更新間隔10ms（少し遅め）

unsigned long seqPrevMillis = 0;
int seqStep = 0;

void setup() {
  joint1.begin();
  joint2.begin();
}

void loop() {
  // delayなしで常に2つのモーターの状態を更新し続ける
  joint1.update();
  joint2.update();

  // --- 以下、3秒ごとに別々の指示を出すシーケンサー ---
  unsigned long currentMillis = millis();
  if (currentMillis - seqPrevMillis >= 3000) {
    seqPrevMillis = currentMillis;

    // ステップごとに2つの関節に別々の角度を指示する
    if (seqStep == 0) {
      joint1.setTargetAngle(90);
      joint2.setTargetAngle(180);
      seqStep = 1;
    } else if (seqStep == 1) {
      joint1.setTargetAngle(180);
      joint2.setTargetAngle(90);
      seqStep = 2;
    } else if (seqStep == 2) {
      joint1.setTargetAngle(0);
      joint2.setTargetAngle(270);
      seqStep = 3;
    } else {
      joint1.setTargetAngle(90);
      joint2.setTargetAngle(90);
      seqStep = 0;
    }
  }
}