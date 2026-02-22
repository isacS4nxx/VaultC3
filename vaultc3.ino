#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define I2C_SDA 5
#define I2C_SCL 6

// Alinhamento ideal para 0.42" (72x40 útil dentro de 128x64)
#define OFF_X 28  
#define OFF_Y 24  

AsyncWebServer server(80);
Preferences prefs;

unsigned long lastScrollTime = 0;
const int scrollInterval = 5000; 
int currentSiteIndex = 0;

struct SiteEntry {
    String name;
    String url;
};
SiteEntry vaultSites[20]; 
int totalSites = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="pt-br"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
    :root { --p: #818cf8; --bg: #0f172a; --card: #1e293b; --red: #ef4444; }
    body { background: var(--bg); color: #f8fafc; font-family: 'Segoe UI', system-ui, sans-serif; display: flex; justify-content: center; align-items: center; min-height: 100vh; margin: 0; }
    .card { background: var(--card); padding: 2rem; border-radius: 1.5rem; width: 90%; max-width: 400px; border: 1px solid #334155; text-align: center; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }
    h2 { margin-top: 0; color: var(--p); }
    input { width: 100%; padding: 0.8rem; margin: 0.5rem 0; border-radius: 0.6rem; border: 1px solid #334155; background: #0f172a; color: white; box-sizing: border-box; }
    .pass-group { display: flex; gap: 5px; align-items: center; }
    button { width: 100%; padding: 0.8rem; background: var(--p); color: white; border: none; border-radius: 0.6rem; font-weight: bold; cursor: pointer; margin-top: 0.5rem; transition: 0.2s; }
    button:hover { opacity: 0.8; }
    .btn-gen { width: auto; background: #334155; padding: 0.8rem; white-space: nowrap; }
    .item { background: #1a2236; margin-top: 1rem; padding: 1rem; border-radius: 0.8rem; border: 1px solid #334155; text-align: left; position: relative; }
    .pass-val { font-family: monospace; color: #94a3b8; filter: blur(5px); cursor:pointer; transition: 0.3s; }
    .pass-val.visible { filter: blur(0); color: #34d399; }
    .btn-del { position: absolute; right: 10px; top: 10px; background: var(--red); width: 25px; height: 25px; padding: 0; border-radius: 5px; font-size: 12px; }
</style></head><body>
    <div id="login" class="card">
        <h2>🔐 Vault C3</h2>
        <input type="password" id="m" placeholder="Master Password">
        <button onclick="check()">Entrar</button>
    </div>
    <div id="main" class="card" style="display:none">
        <h2>📂 Meus Acessos</h2>
        <input id="n" placeholder="Nome do Site">
        <input id="u" placeholder="URL (ex: google.com)">
        <div class="pass-group">
            <input id="p" placeholder="Senha">
            <button class="btn-gen" onclick="genPass()" title="Gerar Senha Segura">🎲</button>
        </div>
        <button onclick="save()">Salvar no Vault</button>
        <div id="list" style="margin-top:20px"></div>
    </div>
<script>
    function genPass() {
        const chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+";
        let pass = "";
        const array = new Uint32Array(16);
        window.crypto.getRandomValues(array);
        for (let i = 0; i < 16; i++) {
            pass += chars[array[i] % chars.length];
        }
        document.getElementById('p').value = pass;
    }
    function check(){ if(document.getElementById('m').value==='123'){document.getElementById('login').style.display='none';document.getElementById('main').style.display='block';load();}else alert("Erro");}
    async function load(){
        const r = await fetch('/list'); const d = await r.json(); let h = '';
        d.forEach((i, idx)=>{
            h += `<div class='item'><b>${i.n}</b><br><small style='color:var(--p)'>${i.u}</small><br>
                <span class='pass-val' onclick='this.classList.toggle("visible")'>${i.p}</span>
                <button class='btn-del' onclick='del("${i.n}")'>X</button></div>`;
        });
        document.getElementById('list').innerHTML = h || '<p>Vazio</p>';
    }
    function save(){
        const n=document.getElementById('n').value, u=document.getElementById('u').value, p=document.getElementById('p').value;
        if(!n||!p) return; fetch(`/save?n=${encodeURIComponent(n)}&u=${encodeURIComponent(u)}&p=${encodeURIComponent(p)}`).then(load);
        document.getElementById('n').value=''; document.getElementById('u').value=''; document.getElementById('p').value='';
    }
    function del(n){ if(confirm('Apagar?')) fetch(`/del?n=${encodeURIComponent(n)}`).then(load); }
</script></body></html>
)rawliteral";

void syncVault() {
    totalSites = 0;
    String index = prefs.getString("index", "");
    int start = 0;
    int end = index.indexOf(',');
    while(end != -1 && totalSites < 20) {
        String key = index.substring(start, end);
        vaultSites[totalSites].name = key;
        vaultSites[totalSites].url = prefs.getString((key + "_u").c_str(), "");
        start = end + 1;
        end = index.indexOf(',', start);
        totalSites++;
    }
}

void drawOLED() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    if (totalSites > 0) {
        if (currentSiteIndex >= totalSites) currentSiteIndex = 0;
        display.setCursor(OFF_X, OFF_Y);
        String n = vaultSites[currentSiteIndex].name;
        if(n.length() > 10) n = n.substring(0, 8) + "..";
        display.print("SITE: "); display.println(n);

        display.setCursor(OFF_X, OFF_Y + 12);
        String u = vaultSites[currentSiteIndex].url;
        if(u.length() > 12) u = u.substring(0, 10) + "..";
        display.print("> "); display.println(u);
    } else {
        display.setCursor(OFF_X, OFF_Y + 5);
        display.print("VAULT VAZIO");
    }
    display.display();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();

    prefs.begin("vault", false);
    syncVault();

    WiFi.softAP("Vault-C3-Safe", "12345678");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        String index = prefs.getString("index", "");
        int start = 0, end = index.indexOf(',');
        while (end != -1) {
            String k = index.substring(start, end);
            String u = prefs.getString((k + "_u").c_str(), "");
            String p = prefs.getString(k.c_str(), "");
            json += "{\"n\":\"" + k + "\",\"u\":\"" + u + "\",\"p\":\"" + p + "\"},";
            start = end + 1; end = index.indexOf(',', start);
        }
        if(json.endsWith(",")) json.remove(json.length()-1);
        json += "]";
        request->send(200, "application/json", json);
    });

    server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("n") && request->hasParam("p")) {
            String n = request->getParam("n")->value();
            String u = request->hasParam("u") ? request->getParam("u")->value() : "";
            String p = request->getParam("p")->value();
            prefs.putString(n.c_str(), p);
            prefs.putString((n + "_u").c_str(), u);
            String index = prefs.getString("index", "");
            if(index.indexOf(n + ",") == -1) {
                index += n + ",";
                prefs.putString("index", index);
            }
            syncVault();
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/del", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->hasParam("n")) {
            String n = request->getParam("n")->value();
            prefs.remove(n.c_str());
            prefs.remove((n + "_u").c_str());
            String index = prefs.getString("index", "");
            index.replace(n + ",", "");
            prefs.putString("index", index);
            syncVault();
        }
        request->send(200, "text/plain", "OK");
    });

    server.begin();
    drawOLED();
}

void loop() {
    if (millis() - lastScrollTime >= scrollInterval) {
        lastScrollTime = millis();
        if (totalSites > 0) {
            currentSiteIndex = (currentSiteIndex + 1) % totalSites;
            drawOLED();
        }
    }
}