#include <Arduino.h>

/*
e.g. 前後代表了(+100)or(-100) ...
左前輪 = 前後 + 左右 + 自轉
右前輪 = 前後 - 左右 - 自轉
左後輪 = 前後 - 左右 + 自轉
右後輪 = 前後 + 左右 - 自轉

順序:左前->右前->左後->右後

BLDC必需接的pin:
2(F/R) DigitalPin, 正轉反轉 -Gray
3(PWM) AnalogPin -Purple
6(on/off) DigitalPin, 馬達開關 -Yellow
8(GND) 地線 -Black
9(VM) 電源線 -Red
*/

/*
與主控raspberry pi的通訊, 直接使用arduino mega封裝好的UART即可
Serial1 對應 RX1與TX1
*/

// LF = LeftForward
// A0 = D54
const int LF_FR = A0;
const int LF_PWM = 2;
const int LF_OF = A1;

const int RF_FR = A2;
const int RF_PWM = 3;
const int RF_OF = A3;

const int LB_FR = A4;
const int LB_PWM = 4;
const int LB_OF = A5;

const int RB_FR = A6;
const int RB_PWM = 5;
const int RB_OF = A7;

// uint(unsigned integer 8-bit type) 0~255
enum State: uint8_t {WAIT_HEADER, READ_vx, READ_vy, READ_rotate, WAIT_FOOTER};
State currentState = WAIT_HEADER;
/*
速度計算公式：(x - 127) * 2
analogWrite(0~255)
127-127*2=0
256-126*2=260
0-127*2=-254
*/
uint8_t wheelSpeeds[3] = {0, 0, 0};

// 強制停止計時器
unsigned long lastReceiveTime = 0;

int vx = 0; int vy = 0; int rotate = 0;

void setup() {
    // 此Serial = Serial0 = debugSerial 默認連電腦USB
    // 與電腦通訊
    Serial.begin(115200);
    Serial.println("Inital text!");

    // 與Raspberry Pi通訊
    Serial1.begin(115200);
    Serial1.println("Hello, raspberry pi!");

    pinMode(LF_FR, OUTPUT);
    pinMode(LF_PWM, OUTPUT);
    pinMode(LF_OF, OUTPUT);

    pinMode(RF_FR, OUTPUT);
    pinMode(RF_PWM, OUTPUT);
    pinMode(RF_OF, OUTPUT);

    pinMode(LB_FR, OUTPUT);
    pinMode(LB_PWM, OUTPUT);
    pinMode(LB_OF, OUTPUT);

    pinMode(RB_FR, OUTPUT);
    pinMode(RB_PWM, OUTPUT);
    pinMode(RB_OF, OUTPUT);
    
    digitalWrite(LF_OF, HIGH);
    digitalWrite(RF_OF, HIGH);
    digitalWrite(LB_OF, HIGH);
    digitalWrite(RB_OF, HIGH);
}


// motor control function
void motorSpeeds(int speedLF, int speedRF, int speedLB, int speedRB) {
    /*
    FR, PWM, OF
    */
    // 左前輪
    if (speedLF > 0) {
        digitalWrite(LF_FR, HIGH);
        analogWrite(LF_PWM, speedLF);
    } else {
        digitalWrite(LF_FR, LOW);
        analogWrite(LF_PWM, abs(speedLF));
    }
    //右前輪
    if (speedRF > 0) {
        digitalWrite(RF_FR, HIGH);
        analogWrite(RF_PWM, speedRF);
    } else {
        digitalWrite(RF_FR, LOW);
        analogWrite(RF_PWM, abs(speedRF));
    }
    //左後輪
    if (speedLB > 0) {
        digitalWrite(LB_FR, HIGH);
        analogWrite(LB_PWM, speedLB);
    } else {
        digitalWrite(LB_FR, LOW);
        analogWrite(LB_PWM, abs(speedLB));
    }
    //右後輪
    if (speedRB > 0) {
        digitalWrite(RB_FR, HIGH);
        analogWrite(RB_PWM, speedRB);
    } else {
        digitalWrite(RB_FR, LOW);
        analogWrite(RB_PWM, abs(speedRB));
    }
}

/*
e.g. 前後代表了(+100)or(-100) ...
左前輪 = 前後 + 左右 + 自轉
右前輪 = 前後 - 左右 - 自轉
左後輪 = 前後 - 左右 + 自轉
右後輪 = 前後 + 左右 - 自轉

順序:左前->右前->左後->右後
*/
void drive(int vx, int vy, int rotate) {
    int speedLf = vx + vy + rotate;
    int speedLb = vx - vy - rotate;
    int speedRf = vx - vy + rotate;
    int speedRb = vx + vy - rotate;

    motorSpeeds(speedLf, speedRf, speedLb, speedRb);
}

void loop() {

    if (Serial.available()) {
        // 超過500ms沒接收新的指令，強制停止
        unsigned currentTime = millis();
        if (currentTime - lastReceiveTime > 500) {
            motorSpeeds(0, 0, 0, 0);
        }
        // 0xAA代表從開頭進入數據監聽 0x55代表結束監聽回到WAIT_HEADER
        // 需要確人header, footer皆是正確的才會執行這一段馬達控制
        while(Serial.available()) {
            uint8_t c = Serial.read();
            switch(currentState){
                case(WAIT_HEADER):
                    if (c == 0xAA) currentState = READ_vx; 
                    break;
                case(READ_vx): 
                    vx = c; 
                    currentState = READ_vy;
                    break;
                case(READ_vy): 
                    vy = c; 
                    currentState = READ_rotate; 
                    break;
                case(READ_rotate): 
                    rotate = c; 
                    currentState = WAIT_FOOTER; 
                    break;
                case(WAIT_FOOTER):
                    if (c == 0x55) {
                        lastReceiveTime = millis();
                        drive(vx, vy, rotate);
    
                        Serial.println("Package Received");
                        Serial.print("校驗：");
                        Serial.print(vx);
                        Serial.print(vy);
                        Serial.println(rotate);
                    }
                    currentState = WAIT_HEADER;
                    break;
                }
        }       
    }
    else {

    }
}