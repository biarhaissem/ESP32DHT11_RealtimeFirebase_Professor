// Bibliotecas ------------------------------------------
#include <WiFiServer.h>
#include <DHT.h>
#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <ArduinoJson.h>
//#include <TimeLib.h>

// DHT --------------------------------------------------
#define DHTPIN 4  // Pino de dados do sensor DHT
#define DHTTYPE DHT11  // Tipo do sensor (DHT11)

// Firebase -- ------------------------------------------
#define FIREBASE_HOST "novo-7c929-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "PqHvc5PAQx0fHodq2gOHwDj8UfktXTEXQEqmJTih"

// Wi-Fi ------------------------------------------------
#define ssid     "BIA_2G"
#define password "Sapo-666"

// Constantes -------------------------------------------
const char*   ntpServer           = "pool.ntp.org";
const long    gmtOffset_sec       = -3 * 60 * 60;   // -3h*60min*60s = -10800s
const int     daylightOffset_sec  = 0;              // Fuso em horário de verão

// Variáveis globais ------------------------------------
time_t        nextNTPSync         = 0;


DHT dht(DHTPIN, DHTTYPE);


// Funções auxiliares -----------------------------------

String dateTimeStr(time_t t, int8_t tz = 0) {
  // Formata time_t como "aaaa-mm-dd hh:mm:ss"
  if (t == 0) {
    return "N/D";
  } else {
    t += tz * 3600;                               // Ajusta fuso horário
    struct tm *ptm;
    ptm = gmtime(&t);
    String s;
    s = ptm->tm_year + 1900;
    s += "-";
    if (ptm->tm_mon < 9) {
      s += "0";
    }
    s += ptm->tm_mon + 1;
    s += "-";
    if (ptm->tm_mday < 10) {
      s += "0";
    }
    s += ptm->tm_mday;
    s += " ";
    if (ptm->tm_hour < 10) {
      s += "0";
    }
    s += ptm->tm_hour;
    s += ":";
    if (ptm->tm_min < 10) {
      s += "0";
    }
    s += ptm->tm_min;
    s += ":";
    if (ptm->tm_sec < 10) {
      s += "0";
    }
    s += ptm->tm_sec;
    return s;
  }
}

String timeStatus() {
  // Obtém o status da sinronização
  if (nextNTPSync == 0) {
    return "não definida";
  } else if (time(NULL) < nextNTPSync) {
    return "atualizada";
  } else {
    return "atualização pendente";
  }
}

// Callback de sincronização
void ntpSync_cb(struct timeval *tv) {
  time_t t;
  t = time(NULL);
  // Data/Hora da próxima atualização
  nextNTPSync = t + (SNTP_UPDATE_DELAY / 1000) + 60;

  Serial.println("Sincronizou com NTP em " + dateTimeStr(t));
  Serial.println("Limite para próxima sincronização é " +
                  dateTimeStr(nextNTPSync));
}

// Setup ------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  dht.begin();
  delay(2000);

  Serial.begin(115200);
  //sntp_set_time_sync_notification_cb(ntpSync_cb);

  // Intervalo de sincronização - definido pela bibioteca lwIP
  Serial.printf("\n\nNTP sincroniza a cada %d segundos\n",
                SNTP_UPDATE_DELAY / 1000);

  // Função para inicializar o cliente NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.println("--------------------------- WIFI CONNECTION ----------------------------");
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //server.begin();

  Serial.println("-------------------------- FIREBASE CONNECTION -------------------------");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    
  if (Firebase.failed()) {
    Serial.print("Falha na conexão com o Firebase. Código de erro: ");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Conexão com o Firebase bem-sucedida...");
    Serial.println();
  }

  Serial.println("----------------- TEMPERATURE AMD HUMIDITY INFORMATION -----------------");
  Serial.println();

}

int n = 0;

void loop() {

  // Realizar a leitura do sensor
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    Serial.println("Falha na leitura do sensor DHT");
    delay(2000);
    return;
  }
  
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C | ");
  Serial.print("Humidity: ");
  Serial.print(humid);
  Serial.print(" % ");

  Serial.println();

  String data = "{\"temperature\": " + String(temp) + ", \"humidity\": " + 
                                       String(humid) + ", \"timestamp\": \"" "\"}";

  Firebase.pushString("/data", data);

  delay(15000);

}
