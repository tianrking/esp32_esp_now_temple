#include <ESP8266WiFi.h>
#include <espnow.h>

// 接收设备的MAC地址（需要替换为实际的接收设备MAC地址）
uint8_t receiverAddress[] = {0x5C, 0xCF, 0x7F, 0xF0, 0x06, 0x10};

// 打印MAC地址的辅助函数
void printMacAddress(const char* prefix, const uint8_t* mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("%s %s\n", prefix, macStr);
}

// 数据发送回调函数
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  printMacAddress("Last Packet Sent to:", mac_addr);
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

// 数据接收回调函数
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  printMacAddress("Received data from:", mac);
  Serial.write(incomingData, len);
  Serial.println(); // 为了更好的可读性添加换行
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }  // 等待串口准备就绪
  
  Serial.println("\nESP8266 ESP-NOW Transparent Bridge");
  
  // 设置设备为 Station 模式
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // 获取并打印ESP8266的MAC地址
  uint8_t macAddr[6];
  WiFi.macAddress(macAddr);
  printMacAddress("ESP8266 MAC Address:", macAddr);

  // 初始化 ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // 设置 ESP-NOW 角色
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  
  // 注册发送和接收回调
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // 添加对等设备
  esp_now_add_peer(receiverAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  printMacAddress("Receiver MAC Address:", receiverAddress);
  
  Serial.println("ESP8266 initialized and ready for ESP-NOW communication");
}

void loop() {
  // 检查串口是否有数据可读
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    
    // 通过 ESP-NOW 发送数据
    uint8_t result = esp_now_send(receiverAddress, (uint8_t *)message.c_str(), message.length());
    
    if (result == 0) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }
}