5. 넘어짐 감지 신발
#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU6050.h>

#define BT_RXD 3
#define BT_TXD 4
SoftwareSerial hc06(BT_RXD, BT_TXD);

MPU6050 mpu;

const int buzzerPin = 8; // 부저 핀
long lastTime = 0; // 시간 체크를 위한 변수
bool buzzerState = false; // 부저 상태

int FSRsensor = A0;                           // 센서값을 아나로그 A0핀 설정
int value = 0;          

void setup() {
  // 시리얼 모니터 시작
  Serial.begin(9600);
  pinMode(buzzerPin,OUTPUT);
  
  //hc06
  hc06.begin(9600);

  // MPU6050 초기화
  Wire.begin();
  mpu.initialize();
  
  // 센서 캘리브레이션
  Serial.println("MPU6050 캘리브레이션 중...");
  mpu.CalibrateGyro(6); // 자이로 캘리브레이션
  mpu.CalibrateAccel(6); // 가속도 캘리브레이션
  Serial.println("캘리브레이션 완료.");
  tone(buzzerPin, 523,300);
}

void loop() {
  value = analogRead(FSRsensor);     // 아날로그 센서값을 value 변수에 저장
  value = map(value, 0, 1023, 0, 255); // value에 아날로그 센서값(0~1023)을 PWM 조절을위해 값 0~255로 변환
  Serial.println(value);
  // MPU6050에서 가속도 데이터를 읽음
  
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  
  // 가속도 값을 각도로 변환 (tilt angle 계산)
  float angle = atan2(ay, az) * 180.0 / PI;

  // 각도 출력 (디버깅용)
  Serial.print("Angle: ");
  Serial.println(angle);

  // 자이로 센서에서 구해진 각도가 15도 이상이거나, -15도 이하일 때 
  if (angle >= 15 || angle <= -15) {
    if (millis() - lastTime >= 3000 && value >=128) { // 3초간 유지되었을 때
      if (!buzzerState) {
        tone(buzzerPin, 700,1000);
        delay(500);
        // noTone(buzzerState);
        buzzerState = true;
        Serial.println("부저 울림");
        hc06.write("1"); // 넘어짐 감지되어 상황을 휴대폰으로 전송
        lastTime = millis(); // 3초 유지 여부 파악
      }
    }
  } else {
    digitalWrite(buzzerPin, LOW); // 부저 끄기
    buzzerState = false;
    lastTime = millis(); // 90도 상태를 벗어나면 시간 초기화
  }


  // 블루투스가 전송된 값이 정상적으로 받아왔을 때
  if (hc06.available()) {
    Serial.write(hc06.read()); // 시리얼 모니터에 전송 받은 값 출력
  }
  // 시리얼 모니터에 입력한 값이 전송된 경우
  if (Serial.available()) {
    hc06.write(Serial.read()); // 블루투스에게 입력한 값을 전송함.
  }

  delay(100); // 루프 속도 제어
}