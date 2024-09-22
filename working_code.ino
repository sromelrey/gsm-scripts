#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

SoftwareSerial sim(10, 11);  // RX, TX
TinyGsm modem(sim);
TinyGsmClient client;

bool sendATCommand(const char* command) {
  sim.println(command);
  delay(500);
  while (sim.available()) {
    String response = sim.readString();
    Serial.println(response);
    if (response.indexOf("OK") != -1) {
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(9600);  // Changed to 9600 baud
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  Serial.println("Initializing GSM module...");
  sim.begin(9600);
  delay(1000);

  Serial.println("Initializing modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem, attempting to continue without restart");
  }

  Serial.println("Setting SMS mode...");
  if (!sendATCommand("AT+CMGF=1")) {
    Serial.println("Error setting SMS mode");
  }

  Serial.println("Configuring SMS notifications...");
  if (!sendATCommand("AT+CNMI=2,2,0,0,0")) {
    Serial.println("Error configuring SMS notifications");
  }

  Serial.println("GSM module initialized");
  Serial.println("System Ready and Waiting for New SMS...");
}

void loop() {
  if (sim.available()) {
    delay(500); // Allow time for the full message to arrive
    String data = readFullMessage();
    
    Serial.println("Raw Data:");
    Serial.println(data);

    processMessage(data);
  }
}

String readFullMessage() {
  String fullMessage = "";
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) { // 5-second timeout
    if (sim.available()) {
      char c = sim.read();
      fullMessage += c;
      if (fullMessage.endsWith("\n") && sim.available() == 0) {
        break; // Message complete
      }
    }
  }
  return fullMessage;
}

void processMessage(String data) {
  data.trim();
  
  if (data.startsWith("+CMT:")) {
    // This is an SMS message
    String messageContent = extractSMSContent(data);
    if (messageContent.length() > 0) {
      Serial.print("SMS Content: ");
      Serial.println(messageContent);
      // Process the SMS content here
    } else {
      Serial.println("ERROR: No valid SMS content found.");
    }
  } else if (data.startsWith("+CTZV:") || data.startsWith("*PSUTTZ:") || data.startsWith("DST:")) {
    // This is a network time/date update
    Serial.print("Network Time Update: ");
    Serial.println(data);
  } else {
    // Unknown message type
    Serial.print("Unknown message type: ");
    Serial.println(data);
  }
}

String extractSMSContent(String smsData) {
  int contentStart = smsData.indexOf('\n');
  if (contentStart != -1) {
    String content = smsData.substring(contentStart + 1);
    content.trim();
    return content;
  }
  return "";
}