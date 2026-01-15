#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>

#define LED_PIN A0
#define NUM_LEDS 139

const char* ssid = "wifi_ssid";
const char* password = "wifi_password";

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// State of the LED's
bool isOn = true;
uint8_t red = 255, green = 50, blue = 0;
uint8_t brightness = 255;
bool rainbowMode = false;
uint8_t hue = 0; // wheel effect

void setAllLEDs(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// Wheel function - HUE (0–255)
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en-EN">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: sans-serif; 
            text-align: center; 
            background: #111; 
            color: white; 
            padding: 20px;
        }
        
        .slider-container {
            margin: 20px auto;
            max-width: 500px;
        }
        
        .slider-group {
            margin: 15px 0;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        
        .slider-label {
            font-weight: bold;
            width: 80px;
            text-align: left;
        }
        
        .red-label { color: #ff4444; }
        .green-label { color: #44ff44; }
        .blue-label { color: #4444ff; }
        
        input[type="range"] {
            flex: 1;
            margin: 0 15px;
            height: 8px;
            border-radius: 4px;
            outline: none;
            -webkit-appearance: none;
        }
        
        .red-slider::-webkit-slider-track {
            background: linear-gradient(to right, #000, #ff0000);
        }
        
        .green-slider::-webkit-slider-track {
            background: linear-gradient(to right, #000, #00ff00);
        }
        
        .blue-slider::-webkit-slider-track {
            background: linear-gradient(to right, #000, #0000ff);
        }
        
        .brightness-slider::-webkit-slider-track {
            background: linear-gradient(to right, #000, #fff);
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: white;
            cursor: pointer;
            border: 2px solid #333;
        }
        
        .value-display {
            width: 40px;
            text-align: right;
            font-family: monospace;
            font-size: 14px;
        }
        
        .color-preview {
            width: 100px;
            height: 50px;
            border: 2px solid #555;
            border-radius: 8px;
            margin: 20px auto;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 12px;
            background: #000;
        }
        
        button { 
            margin: 10px; 
            padding: 12px 20px; 
            font-size: 16px; 
            border: none;
            border-radius: 6px;
            background: #333;
            color: white;
            cursor: pointer;
            transition: background 0.2s;
        }
        
        button:hover {
            background: #555;
        }
        
        .rainbow-btn {
            background: linear-gradient(45deg, #ff0000, #ff7700, #ffff00, #00ff00, #0077ff, #0000ff, #7700ff);
            color: white;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.8);
        }
    </style>
</head>
<body>
    <h1>RGB LED lamp</h1>
    
    <div class="slider-container">
        <div class="slider-group">
            <span class="slider-label red-label">Red:</span>
            <input type="range" id="redSlider" class="red-slider" min="0" max="255" value="255">
            <span class="value-display" id="redValue">255</span>
        </div>
        
        <div class="slider-group">
            <span class="slider-label green-label">Green:</span>
            <input type="range" id="greenSlider" class="green-slider" min="0" max="255" value="50">
            <span class="value-display" id="greenValue">50</span>
        </div>
        
        <div class="slider-group">
            <span class="slider-label blue-label">Blue:</span>
            <input type="range" id="blueSlider" class="blue-slider" min="0" max="255" value="0">
            <span class="value-display" id="blueValue">0</span>
        </div>
        
        <div class="color-preview" id="colorPreview">
            Color Preview
        </div>
        
        <div class="slider-group">
            <span class="slider-label">Brightness:</span>
            <input type="range" id="brightnessSlider" class="brightness-slider" min="0" max="255" value="255">
            <span class="value-display" id="brightnessValue">255</span>
        </div>
    </div>
    
    <button onclick="togglePower()">Turn ON / OFF LED lamp</button>
    <br>
    <button onclick="rainbow()" class="rainbow-btn">Turn ON Rainbow</button>

    <script>
        const send = (url) => fetch(url).catch(e => console.error(e));
        
        let redValue = 255;
        let greenValue = 50;
        let blueValue = 0;
        let brightnessValue = 255;
        
        function updateColor() {
            // Update display values
            document.getElementById('redValue').textContent = redValue;
            document.getElementById('greenValue').textContent = greenValue;
            document.getElementById('blueValue').textContent = blueValue;
            document.getElementById('brightnessValue').textContent = brightnessValue;
            
            // Update color preview
            const preview = document.getElementById('colorPreview');
            preview.style.background = `rgb(${redValue}, ${greenValue}, ${blueValue})`;
            
            // Send color to LED
            send(`/color?r=${redValue}&g=${greenValue}&b=${blueValue}`);
        }
        
        function updateBrightness() {
            document.getElementById('brightnessValue').textContent = brightnessValue;
            send(`/brightness?val=${brightnessValue}`);
        }
        
        // Red slider
        document.getElementById("redSlider").addEventListener("input", e => {
            redValue = parseInt(e.target.value);
            updateColor();
        });
        
        // Green slider
        document.getElementById("greenSlider").addEventListener("input", e => {
            greenValue = parseInt(e.target.value);
            updateColor();
        });
        
        // Blue slider
        document.getElementById("blueSlider").addEventListener("input", e => {
            blueValue = parseInt(e.target.value);
            updateColor();
        });
        
        // Brightness slider
        document.getElementById("brightnessSlider").addEventListener("input", e => {
            brightnessValue = parseInt(e.target.value);
            updateBrightness();
        });
        
        function togglePower() { 
            send("/toggle"); 
        }
        
        function rainbow() { 
            send("/rainbow"); 
        }
        
        // Initialize display
        updateColor();
        updateBrightness();
    </script>
</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    red = server.arg("r").toInt();
    green = server.arg("g").toInt();
    blue = server.arg("b").toInt();
    rainbowMode = false;
    applyColor();
  }
  server.send(200, "text/plain", "OK");
}

void handleBrightness() {
  if (server.hasArg("val")) {
    brightness = server.arg("val").toInt();
    strip.setBrightness(brightness);
    rainbowMode = false;
    applyColor();
  }
  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  isOn = !isOn;
  if (isOn) {
    applyColor();
  } else {
    setAllLEDs(0, 0, 0);
  }
  server.send(200, "text/plain", "OK");
}

void handleRainbow() {
  rainbowMode = true;
  server.send(200, "text/plain", "OK");
}

void applyColor() {
  if (isOn) {
    setAllLEDs(red, green, blue);
  } else {
    setAllLEDs(0, 0, 0);
  }
}

void wheelEffect() {
  // Pobierz kolor z funkcji Wheel dla aktualnego HUE
  uint32_t color = Wheel(hue);
  
  // Zastosuj jasność
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  // Skaluj kolory według jasności
  r = (r * brightness) / 255;
  g = (g * brightness) / 255;
  b = (b * brightness) / 255;
  
  // Ustaw ten sam kolor dla każdej diody
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
  
  // Zwiększ HUE, płynnie przechodząc przez barwy
  hue = (hue + 1) % 256;
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  // Turn ON LED lamp - white warm
  setAllLEDs(red, green, blue);


  IPAddress local_IP(192, 168, 0, 151);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\nConnected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot); // local web page
  server.on("/color", handleColor);
  server.on("/brightness", handleBrightness);
  server.on("/toggle", handleToggle);
  server.on("/rainbow", handleRainbow);

  server.begin();
}

void loop() {
  server.handleClient();

  if (rainbowMode && isOn) {
    wheelEffect();
    delay(40); // rainbow speed
  }

}