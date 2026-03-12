import serial
import time

# 1. 設定你的串口名字 (請根據 ls /dev/cu.* 的結果修改)
# 例如: PORT = '/dev/cu.usbmodem14101'
PORT = '你的路徑' 
BAUD_RATE = 115200

try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"成功連線至 {PORT}")
    time.sleep(2) # 等待 Arduino 重啟

    while True:
        print("\n--- 機器人測試控制 ---")
        print("請輸入 0-254 (127 為停止，127 以上前進，以下後退)")
        try:
            val = int(input("輸入速度值: "))
            if 0 <= val <= 254:
                # 2. 打包封包 [Header, W1, W2, W3, W4, Footer]
                # 我們先讓四個輪子跑一樣的速度
                packet = bytearray([0xAA, val, val, val, val, 0x55])
                
                # 3. 傳送二進位序列
                ser.write(packet)
                print(f"已傳送封包: {list(packet)}")
            else:
                print("錯誤: 請輸入 0-254 之間的數字")
        except ValueError:
            print("請輸入有效的數字")

except Exception as e:
    print(f"發生錯誤: {e}")
finally:
    if 'ser' in locals():
        ser.close()
        print("串口已關閉")