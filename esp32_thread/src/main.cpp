#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ESP32-C3的MAC地址（需要替换为实际的MAC地址）
uint8_t receiverAddress[] = {0xEC, 0xDA, 0x3B, 0xBE, 0xDB, 0x8C};

void printMacAddress(const char* prefix, const uint8_t* mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.printf("%s %s\n", prefix, macStr);
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // printMacAddress("Last Packet Send Status to:", mac_addr);
  Serial.printf("Send Status: %s\n", status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  // printMacAddress("Received data from:", mac_addr);
  Serial.write(data, data_len);
  Serial.println(); // Add a newline for better readability
}

bool addPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  // 让ESP-NOW自动选择通道
  peerInfo.encrypt = false;
  
  esp_err_t addStatus = esp_now_add_peer(&peerInfo);
  if (addStatus == ESP_OK) {
    Serial.println("Peer added successfully");
    return true;
  } else {
    Serial.printf("Failed to add peer. Error: 0x%X\n", addStatus);
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(100); }  // 等待串口准备就绪
  
  Serial.println("\nESP32 ESP-NOW Point-to-Point");
  
  // 初始化 WiFi
  WiFi.mode(WIFI_STA);
  
  uint8_t originalMac[6];
  esp_read_mac(originalMac, ESP_MAC_WIFI_STA);
  printMacAddress("ESP32 MAC address:", originalMac);
  
  // 初始化 ESP-NOW
  esp_err_t initStatus = esp_now_init();
  if (initStatus != ESP_OK) {
    Serial.printf("Error initializing ESP-NOW. Error: 0x%X\n", initStatus);
    return;
  }
  Serial.println("ESP-NOW initialized successfully");
  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  if (!addPeer()) {
    return;
  }
  
  // printMacAddress("ESP32-C3 MAC address:", receiverAddress);
  
  // 获取当前WiFi通道
  uint8_t currentChannel;
  esp_wifi_get_channel(&currentChannel, NULL);
  // Serial.printf("Current WiFi Channel: %d\n", currentChannel);
  
  // Serial.println("ESP32 initialized and ready");
}

void loop() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)message.c_str(), message.length());
    
    if (result == ESP_OK) {
      // Serial.println("Send request success");
    } else {
      Serial.printf("Send failed. Error: 0x%X\n", result);
      
      // 如果发送失败，尝试重新添加对等设备
      Serial.println("Attempting to re-add peer...");
      esp_now_del_peer(receiverAddress);
      if (addPeer()) {
        Serial.println("Peer re-added. Retrying send...");
        result = esp_now_send(receiverAddress, (uint8_t *)message.c_str(), message.length());
        if (result == ESP_OK) {
          // Serial.println("Retry send request success");
        } else {
          Serial.printf("Retry send failed. Error: 0x%X\n", result);
        }
      }
    }
  }
}