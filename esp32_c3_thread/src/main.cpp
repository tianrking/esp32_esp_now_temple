#include <esp_now.h>
#include <WiFi.h>

// MAC address of the ESP32 board (ABCD)
uint8_t receiverAddress[] = {0xC8, 0xF0, 0x9E, 0xA1, 0xCE, 0x58};;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.printf("Last Packet Send Status to: %s | %s\n", macStr, status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // Serial.printf("Received data from: %s\n", macStr);
  Serial.write(data, data_len);
  Serial.println(); // Add a newline for better readability
}

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  // 发送初始化时的两条广播消息
  esp_now_send(receiverAddress, (uint8_t *)"ESP32-C3 initialized", strlen("ESP32-C3 initialized"));
  esp_now_send(receiverAddress, (uint8_t *)"ESP-NOW ready", strlen("ESP-NOW ready"));
}

void loop() {
  // Check for incoming serial data
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)message.c_str(), message.length());
    
    if (result == ESP_OK) {
      // Serial.println("Send success");
    } else {
      Serial.println("Send failed");
    }
  }
}