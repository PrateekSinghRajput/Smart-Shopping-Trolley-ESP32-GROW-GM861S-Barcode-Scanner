#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>

// --- WIFI CREDENTIALS ---
const char* ssid = "Prateek";
const char* password = "justdoelectronics@#12345";

WebServer server(80);

// --- PIN DEFINITIONS (EXPLICITLY DEFINED HERE) ---
#define BUZZER 14
#define GSM_RX_PIN 4  // Connects to GSM Module TX Pin
#define GSM_TX_PIN 5  // Connects to GSM Module RX Pin

// --- HARDWARE SERIAL CONFIGURATION ---
HardwareSerial ScannerSerial(2);  // Barcode Scanner: Default RX=16, TX=17
HardwareSerial GsmSerial(1);      // GSM Module: Customized to use Pin 4 and Pin 5

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust address if needed

// --- TARGET PHONE NUMBER FOR RECEIVING SMS ---
const char* target_phone = "+919975617490"; // Replace with your target mobile number

String barcode = "";
unsigned long current = 0;
unsigned long time_interval = 2000;
char c = ' ';

// --- DATABASE REFRACTORED FOR MATH & INVENTORY TRACKING ---
const int TOTAL_PRODUCTS = 9;
String Item_Code[]  = { "F-01", "F-02", "F-03", "F-04", "F-05", "F-06", "F-07", "F-08", "F-09" };
String Item_Name[]  = { "CHERRY", "PEAR", "AVOCADO", "APPLE", "GUAVA", "KIWI", "PAPAYA", "LITCHI", "LEMON" };
int Item_Price[]    = { 48, 16, 20, 22, 13, 27, 10, 32, 14 };
int Item_Quant[]    = { 0,     0,     0,     0,     0,     0,     0,     0,     0 }; 

// --- RUNNING TOTALS & HISTORY ---
int totalItems = 0;
int totalPrice = 0;

struct ScanRecord {
  String text;
};
ScanRecord history[5]; 
int historyCount = 0;

// Forward Declarations
void adjustProductQuantity(String code, int amount, String source);
void recalculateTotals();
void addLog(String message);
void LCD_Printing(String AA, String BB);
void UpdateIdleLCD();
void sendSMSReceipt();

// ==========================================
// HTML & CSS FOR THE DESIRABLE DASHBOARD
// ==========================================
const char* html_page PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart POS Billing Terminal</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap');
        body { font-family: 'Poppins', sans-serif; background-color: #f0f4f8; margin: 0; padding: 15px; color: #2c3e50; }
        .container { max-width: 900px; margin: 0 auto; }
        .header { text-align: center; margin-bottom: 25px; background: linear-gradient(135deg, #1e3c72, #2a5298); color: white; padding: 20px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
        .header h1 { margin: 0; font-size: 26px; font-weight: 600; letter-spacing: 0.5px; }
        .header p { margin: 5px 0 0 0; opacity: 0.8; font-size: 14px; }
        .cards { display: flex; gap: 15px; justify-content: center; margin-bottom: 25px; }
        .card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 4px 10px rgba(0,0,0,0.03); text-align: center; flex: 1; }
        .card h3 { margin: 0 0 5px 0; color: #7f8c8d; font-weight: 400; font-size: 14px; text-transform: uppercase; letter-spacing: 0.5px; }
        .card .value { font-size: 32px; font-weight: 600; color: #2ecc71; }
        .card.items .value { color: #3498db; }
        .section-title { font-size: 18px; font-weight: 600; margin: 20px 0 10px 0; color: #34495e; }
        .table-container { background: white; border-radius: 15px; box-shadow: 0 4px 10px rgba(0,0,0,0.03); padding: 15px; overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; text-align: left; }
        th { padding: 12px 10px; border-bottom: 2px solid #ecf0f1; color: #7f8c8d; font-size: 13px; text-transform: uppercase; }
        td { padding: 12px 10px; border-bottom: 1px solid #f1f2f6; font-size: 15px; vertical-align: middle; }
        tr:last-child td { border-bottom: none; }
        .btn { border: none; width: 32px; height: 32px; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; transition: all 0.2s ease; margin: 0 4px; display: inline-flex; align-items: center; justify-content: center; }
        .btn-add { background-color: #e8f8f5; color: #2ecc71; }
        .btn-add:hover { background-color: #2ecc71; color: white; }
        .btn-sub { background-color: #fdedec; color: #e74c3c; }
        .btn-sub:hover { background-color: #e74c3c; color: white; }
        .qty-badge { background-color: #f1f2f6; padding: 4px 10px; border-radius: 20px; font-weight: 600; min-width: 20px; display: inline-block; text-align: center; }
        .qty-active { background-color: #3498db; color: white; }
        .pay-container { text-align: center; margin: 25px 0; }
        .btn-pay { background: linear-gradient(135deg, #2ecc71, #27ae60); color: white; border: none; padding: 14px 40px; font-size: 18px; font-weight: 600; border-radius: 30px; cursor: pointer; box-shadow: 0 5px 15px rgba(46, 204, 113, 0.4); transition: all 0.2s ease; width: 100%; max-width: 300px; }
        .btn-pay:hover { transform: translateY(-2px); box-shadow: 0 7px 20px rgba(46, 204, 113, 0.5); background: linear-gradient(135deg, #27ae60, #219653); }
        .btn-pay:active { transform: translateY(0); }
        .log-list { background: #2c3e50; color: #ecf0f1; padding: 15px; border-radius: 15px; font-family: monospace; font-size: 13px; list-style: none; margin: 0; }
        .log-list li { margin-bottom: 6px; padding-bottom: 6px; border-bottom: 1px solid #34495e; }
        .log-list li:last-child { margin-bottom: 0; padding-bottom: 0; border-bottom: none; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Smart POS Billing Terminal</h1>
            <p>Interactive Control Dashboard</p>
        </div>
        
        <div class="cards">
            <div class="card items">
                <h3>Total Quantities</h3>
                <div class="value" id="val-items">0</div>
            </div>
            <div class="card">
                <h3>Grand Total</h3>
                <div class="value" id="val-total">Rs. 0</div>
            </div>
        </div>

        <div class="section-title">Product Counter Sheet</div>
        <div class="table-container">
            <table>
                <thead>
                    <tr>
                        <th>Code</th>
                        <th>Product Name</th>
                        <th>Price</th>
                        <th style="text-align: center; width: 140px;">Quantity</th>
                        <th>Subtotal</th>
                    </tr>
                </thead>
                <tbody id="inventory-body">
                    <!-- Dynamic Rows Insertion -->
                </tbody>
            </table>
        </div>

        <div class="pay-container">
            <button class="btn-pay" onclick="payNow()">Pay Now</button>
        </div>

        <div class="section-title">Live Terminal Status Logs</div>
        <ul class="log-list" id="log-box">
            <li>[SYSTEM] System initialized. Ready to receive commands...</li>
        </ul>
    </div>

    <script>
        function updateDashboard() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('val-items').innerText = data.total_items;
                    document.getElementById('val-total').innerText = "Rs. " + data.grand_total;
                    
                    let tbody = document.getElementById('inventory-body');
                    tbody.innerHTML = '';
                    
                    data.products.forEach(p => {
                        let badgeClass = p.qty > 0 ? "qty-badge qty-active" : "qty-badge";
                        let row = `<tr>
                            <td><strong>${p.code}</strong></td>
                            <td>${p.name}</td>
                            <td>Rs. ${p.price}</td>
                            <td style="text-align: center;">
                                <button class="btn btn-sub" onclick="adjustQty('${p.code}', 'remove')">-</button>
                                <span class="${badgeClass}">${p.qty}</span>
                                <button class="btn btn-add" onclick="adjustQty('${p.code}', 'add')">+</button>
                            </td>
                            <td><strong>Rs. ${p.qty * p.price}</strong></td>
                        </tr>`;
                        tbody.innerHTML += row;
                    });

                    let logBox = document.getElementById('log-box');
                    logBox.innerHTML = '';
                    if (data.logs.length === 0) {
                        logBox.innerHTML = '<li>Waiting for activity...</li>';
                    } else {
                        data.logs.forEach(log => {
                            logBox.innerHTML += `<li>${log}</li>`;
                        });
                    }
                });
        }

        function adjustQty(code, action) {
            fetch(`/${action}?code=${code}`)
                .then(res => updateDashboard());
        }

        function payNow() {
            if (confirm("Confirm payment processing and dispatch SMS invoice?")) {
                fetch('/pay')
                    .then(res => res.text())
                    .then(text => {
                        alert("Checkout Status: " + text);
                        updateDashboard();
                    });
            }
        }

        setInterval(updateDashboard, 1000); 
        updateDashboard();
    </script>
</body>
</html>
)rawliteral";

// ==========================================
// SERVER REQUEST HANDLERS
// ==========================================
void handleRoot() {
  server.send(200, "text/html", html_page);
}

void handleData() {
  String json = "{";
  json += "\"total_items\":" + String(totalItems) + ",";
  json += "\"grand_total\":" + String(totalPrice) + ",";
  json += "\"products\":[";
  for(int i = 0; i < TOTAL_PRODUCTS; i++) {
    json += "{";
    json += "\"code\":\"" + Item_Code[i] + "\",";
    json += "\"name\":\"" + Item_Name[i] + "\",";
    json += "\"price\":" + String(Item_Price[i]) + ",";
    json += "\"qty\":" + String(Item_Quant[i]);
    json += "}";
    if(i < TOTAL_PRODUCTS - 1) json += ",";
  }
  json += "],";
  json += "\"logs\":[";
  for(int i = historyCount - 1; i >= 0; i--) { 
    json += "\"" + history[i].text + "\"";
    if(i > 0) json += ",";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleAdd() {
  if (server.hasArg("code")) {
    String code = server.arg("code");
    adjustProductQuantity(code, 1, "WEB");
  }
  server.send(200, "text/plain", "OK");
}

void handleRemove() {
  if (server.hasArg("code")) {
    String code = server.arg("code");
    adjustProductQuantity(code, -1, "WEB");
  }
  server.send(200, "text/plain", "OK");
}

void handlePay() {
  if (totalItems == 0) {
    server.send(400, "text/plain", "Error: Cart Empty!");
    return;
  }
  server.send(200, "text/plain", "SMS Dispatched");
  sendSMSReceipt();
}

// ==========================================
// CORE CONTROL LOGIC
// ==========================================
void adjustProductQuantity(String code, int amount, String source) {
  for (int i = 0; i < TOTAL_PRODUCTS; i++) {
    if (Item_Code[i] == code) {
      if (amount < 0 && Item_Quant[i] <= 0) return; 
      
      Item_Quant[i] += amount;
      recalculateTotals();
      
      String direction = (amount > 0) ? "Added" : "Removed";
      addLog("[" + source + "] " + direction + " 1x " + Item_Name[i]);
      
      if(amount > 0) {
        LCD_Printing(Item_Name[i] + " ADDED", "Qty: " + String(Item_Quant[i]));
      } else {
        LCD_Printing(Item_Name[i] + " REMOVED", "Qty: " + String(Item_Quant[i]));
      }
      return;
    }
  }
}

void recalculateTotals() {
  totalItems = 0;
  totalPrice = 0;
  for(int i = 0; i < TOTAL_PRODUCTS; i++) {
    totalItems += Item_Quant[i];
    totalPrice += (Item_Quant[i] * Item_Price[i]);
  }
}

void addLog(String message) {
  if(historyCount < 5) {
    history[historyCount].text = message;
    historyCount++;
  } else {
    for(int i = 1; i < 5; i++) {
      history[i-1] = history[i];
    }
    history[4].text = message;
  }
}

void UpdateIdleLCD() {
  LCD_Printing("Items: " + String(totalItems), "Total: Rs." + String(totalPrice));
}

// ==========================================
// GSM SMS SENDING LOGIC
// ==========================================
void sendSMSReceipt() {
  LCD_Printing("PROCESSING PAY", "SENDING SMS...");
  addLog("[GSM] Sending SMS...");
  
  // Construct the SMS layout dynamically 
  String sms_body = "--- Smart POS Receipt ---\n";
  for (int i = 0; i < TOTAL_PRODUCTS; i++) {
    if (Item_Quant[i] > 0) {
      sms_body += Item_Name[i] + " x" + String(Item_Quant[i]) + "\n";
    }
  }
  sms_body += "---------------------\n";
  sms_body += "Total Items: " + String(totalItems) + "\n";
  sms_body += "Grand Total: Rs. " + String(totalPrice);

  // Send physical AT commands to the GSM hardware
  GsmSerial.println("AT+CMGF=1"); // Mode text configuration
  delay(300);
  
  GsmSerial.print("AT+CMGS=\"");
  GsmSerial.print(target_phone);
  GsmSerial.println("\"");
  delay(300);
  
  GsmSerial.print(sms_body);
  delay(300);
  
  GsmSerial.write(26); // Sends Ctrl+Z character to dispatch message
  delay(3000);        
  
  addLog("[SYSTEM] Pay processed. Cart Reset.");
  LCD_Printing("PAYMENT SUCCESS", "THANK YOU!");
  
  // Wipe item tracking structures back to initial conditions
  for (int i = 0; i < TOTAL_PRODUCTS; i++) {
    Item_Quant[i] = 0;
  }
  recalculateTotals();
  
  delay(2000);
  UpdateIdleLCD();
}

// ==========================================
// MAIN SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  
  // Initialize Scanner Hardware UART2
  ScannerSerial.begin(9600, SERIAL_8N1, 16, 17);
  
  // Initialize GSM Module Hardware UART1 dynamically routing it over the specified pins 
  GsmSerial.begin(9600, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  
  pinMode(BUZZER, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  LCD_Printing("BAR CODE SCANNER", "INITIALIZING...");
  tone(BUZZER, 1000, 100);
  delay(1000);

  Serial.print("Connecting to Wi-Fi");
  LCD_Printing("CONNECTING WIFI:", String(ssid).substring(0,16));
  WiFi.begin(ssid, password);
  
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 20) {
    delay(500);
    Serial.print(".");
    wifi_attempts++;
  }
  
  Serial.println("");
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    LCD_Printing("WIFI CONNECTED!", "IP:" + WiFi.localIP().toString());
    
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/add", handleAdd);
    server.on("/remove", handleRemove);
    server.on("/pay", handlePay); 
    server.begin();
    
    delay(4000);
  } else {
    Serial.println("Wi-Fi Failed. Running Offline.");
    LCD_Printing("WIFI FAILED", "OFFLINE MODE");
    delay(3000);
  }

  UpdateIdleLCD();
  Serial.println("\nSystem Ready. Waiting for scans...\n");
}

void loop() {
  server.handleClient(); 
  
  if (ScannerSerial.available() && (millis() - current) >= time_interval) {
    barcode = "";
    
    while (ScannerSerial.available()) {
      c = ScannerSerial.read();
      if (isPrintable(c)) {
        barcode += c;
      }
      delay(3);
    }
    
    barcode.trim();
    
    if (barcode.length() > 0) {
      tone(BUZZER, 1000, 100);
      Serial.println("\nScanned Code : " + barcode);
      
      bool itemFound = false;
      for (int i = 0; i < TOTAL_PRODUCTS; i++) {
        if (Item_Code[i] == barcode) {
          adjustProductQuantity(barcode, 1, "SCAN");
          itemFound = true;
          break;
        }
      }
      
      if (!itemFound) {
        Serial.println("Result: UNKNOWN PRODUCT");
        addLog("[SCAN] Unknown Item: " + barcode.substring(0,6));
        LCD_Printing("UNKNOWN PRODUCT", barcode.substring(0, 16));
      }
      
      for(int wait = 0; wait < 200; wait++) {
        delay(10);
        server.handleClient();
      }
      
      UpdateIdleLCD();
      current = millis();
    }
  } 
  else if (ScannerSerial.available()) {
    while (ScannerSerial.available()) {
      ScannerSerial.read();
      delay(3);
    }
  }
}

// ==========================================
// STRING ASSISTANTS
// ==========================================
String AutoText(String text, int width = 16) {
  if (text.length() > width) return text.substring(0, width);  
  while (text.length() < width) text += " ";                   
  return text;
}

void LCD_Printing(String AA, String BB) {
  lcd.setCursor(0, 0);
  lcd.print(AutoText(AA));  
  lcd.setCursor(0, 1);
  lcd.print(AutoText(BB));
}