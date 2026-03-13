import serial
import time

# port path: pioHome-device
PORT = '/dev/cu.usbserial-1130' 
BAUD_RATE = 115200

try:
    # 開啟Serial
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"嘗試連線至 {PORT}...")
    time.sleep(2) # 重要：等待 Arduino 重啟

    # 給一個停止指令封包： header, 127, 127, 127, 127, footer
    # 127對應到公式:(127-127)*2=0
    packet = bytearray([0xAA, 127, 127, 127, 127, 0x55])

    while True:
        print(f"發送封包: {list(packet)}")
        ser.write(packet)
        
        # 讀取 Arduino 的回報
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(f"Arduino :{line}")
        
        time.sleep(1)

except KeyboardInterrupt:
    print("\n測試停止")

    # 停止傳輸-輸入停止
    ser.write(bytearray([0xAA, 127, 127, 127, 0x55]))
    time.sleep(0.1)
finally:
    if 'ser' in locals():
        ser.close()