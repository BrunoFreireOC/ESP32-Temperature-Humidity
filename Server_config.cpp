#include "User_lib.hpp"
#include <WebServer.h>
#include <WiFi.h>
#include <HTTPUpdateServer.h>


WebServer server(80);
HTTPUpdateServer httpUpdater;


const char* AP_SSID = "ESP32_Config";  // Nome do hotspot
const char* AP_PASS = "config123";     // Senha do hotspot (opcional)

String savedSSID = "";
String savedPassword = "";

bool loggingActive = false;
String csvBuffer;
String currentCSVFileName;
unsigned long lastLogTime = 0;

String generateCSVHeader() {
  return "timestamp,ntc0_T,ntc1_T,ntc2_T,ntc3_T,ntc4_T,ntc5_T,dht1_T,dht1_H,dht2_T,dht2_H\n";
}

void Server_Init(){
  server.on("/", handleRoot);
  server.on("/data", handleData); // Adicione esta linha

  server.on("/start_log", []() {
    loggingActive = true;
    csvBuffer = generateCSVHeader();

    time_t now = millis() / 1000;
    currentCSVFileName = "log_" + String(now) + ".csv";

    server.send(200, "text/plain", "Gravação iniciada!");
  });

  server.on("/stop_log", []() {
    loggingActive = false;
    server.send(200, "text/plain", currentCSVFileName);
  });

  server.on("/download.csv", []() {
    if (server.hasArg("file")) {
      String filename = server.arg("file");
      server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
    }
    server.send(200, "text/csv", csvBuffer);
  });


  
  httpUpdater.setup(&server, "/update"); // rota OTA

  // (Opcional) com autenticação:
  // httpUpdater.setup(&server, "/update", "admin", "admin");
  httpUpdater.setup(&server, "/update", "admin", "senha123"); // Senha para update do codigo

  server.begin();
  Serial.println("WebServer iniciado.");
  Serial.println("OTA disponível em /update");
}

void Server_HandleClient(){
  server.handleClient();
}

void handleRoot() {
  String html = R"rawliteral(
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Monitoramento</title>
    <script>
      function atualizarDados() {
        fetch('/data')
          .then(response => response.json())
          .then(data => {
            let table = "";
            data.ntcs.forEach((ntc, i) => {
              table += "<tr>";
              table += "<td>NTC " + (i+1) + "</td>";
              table += "<td>" + ntc.resistencia + " Ω</td>";
              table += "<td>" + ntc.medias_resi + " Ω</td>";
              table += "<td>" + ntc.temperatura + " °C</td>";
              table += "<td>" + ntc.medias_temp + " °C</td>";
              table += "</tr>";
            });
            document.getElementById("ntc-table").innerHTML = table;

            let dhtTable = "";
            dhtTable += "<tr><td>DHT1</td><td>" + data.dht1.temp + " °C</td><td>" + data.dht1.humi + " %</td></tr>";
            dhtTable += "<tr><td>DHT2</td><td>" + data.dht2.temp + " °C</td><td>" + data.dht2.humi + " %</td></tr>";
            document.getElementById("dht-table").innerHTML = dhtTable;
          });
      }
      setInterval(atualizarDados, 1000);
      window.onload = atualizarDados;
    </script>
  </head>
  <body>
    <h2>Monitoramento dos NTCs</h2>
    <table border="1">
      <tr> <th>NTC</th> <th>Resistência (Ω)</th> <th>Média Resist. (Ω)</th> <th>Temperatura (°C)</th> <th>Média Temp. (°C)</th> </tr>
      <tbody id="ntc-table"></tbody>
    </table>
    <h2>Sensores DHT</h2>
    <table border="1">
      <tr><th>Sensor</th><th>Temperatura (°C)</th><th>Umidade (%)</th></tr>
      <tbody id="dht-table"></tbody>
    </table>
    <h2>Atualização OTA</h2>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="update" accept=".bin" required>
      <input type="submit" value="Enviar Atualização">
    </form>

    <h2>Registro CSV</h2>
    <p id="logStatus">Status: <strong>Parado</strong></p>
    <button onclick="startLogging()">Começar Gravação</button>
    <button onclick="stopLogging()">Parar e Baixar CSV</button>

    <script>
      function startLogging() {
        fetch('/start_log')
          .then(response => response.text())
          .then(msg => {
            document.getElementById("logStatus").innerHTML = "Status: <strong>Gravando...</strong>";
            alert(msg);
          });
      }

      function stopLogging() {
        fetch('/stop_log')
          .then(response => response.text())
          .then(filename => {
            document.getElementById("logStatus").innerHTML = "Status: <strong>Parado</strong>";
            window.location.href = `/download.csv?file=${filename}`;
          });
      }
    </script>


  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html; charset=UTF-8", html);
}


void handleData() {
  String json = "[";
  for (int i = 0; i < NUM_NTC; i++) {
    json += "{";
    json += "\"id\":" + String(i) + ",";
    json += "\"resistencia\":" + String(ntc[i].Rcalc, 1) + ",";
    json += "\"medias_resi\":" + String(ntc[i].Rcalc, 1) + ",";
    json += "\"temperatura\":" + String(ntc[i].Temp, 1) + ",";
    json += "\"medias_temp\":" + String(getNTCAverage(i), 1);
    json += "}";
    if (i < NUM_NTC - 1) json += ",";
  }
  json += "]";

  String finalJSON = "{";
  finalJSON += "\"ntcs\":" + json + ",";
  finalJSON += "\"dht1\":{\"temp\":" + String(dht1_temp, 1) + ",\"humi\":" + String(dht1_humi, 1) + "},";
  finalJSON += "\"dht2\":{\"temp\":" + String(dht2_temp, 1) + ",\"humi\":" + String(dht2_humi, 1) + "}";
  finalJSON += "}";

  server.send(200, "application/json", finalJSON);
}

void startAPMode() {
  //WiFi.disconnect(true); // Desconecta completamente do WiFi anterior
  delay(300);

  WiFi.softAP(AP_SSID, AP_PASS);
  delay(500);

  Serial.println("\nModo AP Ativado!");
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());

  //server.stop(); // Garante que o servidor está parado antes de reconfigurar
  //server.close(); // Recria o servidor
  
  // server.on("/", HTTP_GET, handleRootAP);
  // server.on("/config", HTTP_POST, handleConfigAP);
  server.on("/", handleRootAP);
  server.on("/config", handleConfigAP);
  server.onNotFound(handleNotFound);

  server.begin();
}

void handleRootAP() {
  Serial.println("Root AP requested");

  String html = R"=====(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Configuração WiFi</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; }
      form { max-width: 400px; margin: 0 auto; }
      input { width: 100%; padding: 8px; margin: 5px 0 15px; box-sizing: border-box; }
      input[type="submit"] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }
    </style>
  </head>
  <body>
    <h1>Configurar WiFi</h1>
    <form action="/config" method="POST">
      <label for="ssid">SSID da rede:</label>
      <input type="text" id="ssid" name="ssid" required>
      
      <label for="password">Senha:</label>
      <input type="password" id="password" name="password">
      
      <input type="submit" value="Conectar">
    </form>
    <p>Tempo restante para configuração: <span id="timeout">03:00</span></p>
    
    <script>
      // Contador regressivo
      let time = 180;
      const timer = setInterval(() => {
        time--;
        const minutes = Math.floor(time / 60).toString().padStart(2, '0');
        const seconds = (time % 60).toString().padStart(2, '0');
        document.getElementById('timeout').textContent = `${minutes}:${seconds}`;
        
        if (time <= 0) {
          clearInterval(timer);
          alert('Tempo esgotado! A página será atualizada.');
          location.reload();
        }
      }, 1000);
    </script>
  </body>
  </html>
  )=====";

  server.send(200, "text/html", html);
}

void handleConfigAP() {
  Serial.println("AP Password received");
  String newSSID = server.arg("ssid");
  String newPassword = server.arg("password");

  saveCredentials(newSSID, newPassword);  // Salva na EEPROM
  server.send(200, "text/html", "<h1>Credenciais salvas! Reiniciando...</h1>");
  delay(1000);
  ESP.restart();  // Reinicia para tentar conectar com as novas credenciais
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}