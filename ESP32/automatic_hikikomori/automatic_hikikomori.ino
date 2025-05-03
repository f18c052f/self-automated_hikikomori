#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <esp_bt.h>
#include <esp_wifi.h>

// ピン定義
const int LED_PIN = 12;       // 外付けLEDのピン
const int SERVO_PIN = 5;    // サーボモータのピン
const int TOGGLE_PIN = 22;   // トグルスイッチのピン

// サーボの角度設定
const int SERVO_INIT_ANGLE = 0;    // 初期角度
const int SERVO_TARGET_ANGLE = 180; // 目標角度

// オブジェクト初期化
Servo myServo;

// 割り込み処理のための変数
volatile bool toggleState = false;
volatile bool toggleChanged = false;

// 割り込み処理：状態変化検知
void IRAM_ATTR toggleInterrupt() {
  toggleState = digitalRead(TOGGLE_PIN);
  toggleChanged = true;
}

void setup() {
  // ピン設定
  pinMode(LED_PIN, OUTPUT);
  pinMode(TOGGLE_PIN, INPUT_PULLUP);
  
  // 省電力設定
  // WiFiとBluetoothを完全に無効化
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  
  btStop();
  esp_bt_controller_disable();
  
  // 初期化時の動作
  // LEDを点灯
  digitalWrite(LED_PIN, HIGH);
  
  // サーボ初期化
  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_INIT_ANGLE);
  
  // 数秒間待機
  delay(3000);
  
  // LEDを消灯
  digitalWrite(LED_PIN, LOW);
  
  // 割り込み設定 - 初期化が終わった後に設定
  attachInterrupt(digitalPinToInterrupt(TOGGLE_PIN), toggleInterrupt, CHANGE);
  
  // 初期状態の読み込み
  toggleState = digitalRead(TOGGLE_PIN);
}

void loop() {
  // トグルスイッチの状態が変化した場合の処理
  if (toggleChanged) {
    // デバウンス処理のための短い遅延
    delay(50);
    // 状態を再確認（デバウンス対策）
    bool currentState = digitalRead(TOGGLE_PIN);
    if (currentState == toggleState) {  // 状態が安定している場合
      if (toggleState) {
        // スイッチがON
        digitalWrite(LED_PIN, HIGH);
        delay(1000);
        myServo.write(SERVO_TARGET_ANGLE);
      } else {
        // スイッチがOFF
        delay(50);
        myServo.write(SERVO_INIT_ANGLE);
        digitalWrite(LED_PIN, LOW);
      }
    }
    toggleChanged = false;
  }
  
  // 省電力化のためのライトスリープ
  // ESP32のライトスリープモードを使用
  delay(100);
}