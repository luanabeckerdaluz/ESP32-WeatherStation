#include "IOXhop_FirebaseESP32.h"

// SSID do WiFi
#define WIFI_SSID "Estrela-32"
// PASSWORD do WiFi
#define WIFI_PASSWORD "AdC111729"
// Firebase Database URL
#define databaseURL "engenharia-de-software-fa3bf.firebaseio.com"
// Definição do caminho onde os dados do sensor serão armazenados
#define LOGSPATH "logs/"


// Pino onde o Sensor está conectado
#define SENSORPIN 33
// Pino onde o Atuador está conectado
#define ACTUATORPIN 23
// Pino onde o LED de status está conectado
#define STATUSPIN 2
// Definição de quantas vezes o valor do sensor será lido até que haja o tratamento (Máx: 255)
#define TIMESTOUPLOADVALUE 10
// Tempo de intervalo entre cada leitura
#define TIMESENSORREAD 1000


// Define os títulos a serem printados na Serial
#define VALUEUPLOAD "------------------Value Upload----------------"
#define VECTOR 		"---------------------Vector-------------------"
#define SORT 		"---------------------Sort---------------------"
#define ENDVECTOR 	"-------------------End Vector-----------------"
#define UPDATE 		"---------------------Update-------------------"
#define EMPTYSPACE 	"----------------------------------------------"

//------------------------------------------------------
//------------------------------------------------------

void setTempOnFirebase(String stringDatetime, float tempCelcius){
	// Converte a temperatura para uma string com precisão de 1 casa decimal somente.
	char temp[10];
	dtostrf(tempCelcius, 2, 1, temp); // min length=2 / setprecision=1

	// Temperatura correspondente é colocada no database
	Firebase.setString(stringDatetime, temp);
	// Lida com algum possível erro no database
	if (Firebase.failed()) {
		Serial.println((String)"setting /number failed: " + Firebase.error());
		return;
	}
	else{
		Serial.println(VALUEUPLOAD);
		Serial.println((String)"value on database >> " + stringDatetime + (String)": " + temp);
		Serial.println(EMPTYSPACE);
	}
}


//------------------------------------------------------
//------------------------------------------------------

float getTemperature(){
	// Resolução: 1.220703125 mV
	// Logo, 8mv equivale a 1ºC

	// 5v 1024
	// V   read
	// V(volts) = 5*read/1024

	// Se 10mv equivale a 1ºC, então: 
	// T(ºC) = V*1000/10
	// Logo, T(ºC) = (5*read/4096) * 1000/10
	// Logo, T(ºC) = read * 500.0  / 4096.0

	int ADCread = analogRead(SENSORPIN);
    // Serial.print("TempADC:");
    // Serial.println(ADCread);

	float temperature = 12;//(ADCread * 500.0) / 4096.0;
 	return temperature;
}

//------------------------------------------------------
//------------------------------------------------------

float getArrayMedian(float array[TIMESTOUPLOADVALUE]){
	// Printa o vetor
	// Serial.println(VECTOR);
	// for (int i=0; i<TIMESTOUPLOADVALUE; i++){
	// 	Serial.print("  ");
	// 	Serial.print(i);
	// 	Serial.print("- ");
	// 	Serial.println(array[i]);
	// }

	// Ordena o vetor
	for (int i = 0; i<TIMESTOUPLOADVALUE; i++){
		for (int j = 0; j<TIMESTOUPLOADVALUE; j++){
			if (array[j] > array[i]){
				float tmp = array[i];
				array[i] = array[j];
				array[j] = tmp;
			}
		}
	}

	// Printa o vetor depois do sort
	// Serial.println(SORT);
	// for (int i=0; i<TIMESTOUPLOADVALUE; i++){
	// 	Serial.print("  ");
	// 	Serial.print(i);
	// 	Serial.print("- ");
	// 	Serial.println(array[i]);
	// }
	// Serial.println(ENDVECTOR);

	// Define a posição do vetor a ser retornada e retorna
	byte position = (TIMESTOUPLOADVALUE/2);
	return array[position];
}

//------------------------------------------------------
//------------------------------------------------------

String getDatetime(){
	// Obtém a data atual
	struct tm timeinfo;
	while(!getLocalTime(&timeinfo));
	
	// Nome da variável é formatado para o formato "data/horário"
	String stringDatetime = "";

	if(timeinfo.tm_mday < 10) 	stringDatetime += "0";
	stringDatetime += timeinfo.tm_mday;

	stringDatetime += "-";

	if(timeinfo.tm_mon+1 < 10) 	stringDatetime += "0";
	stringDatetime += timeinfo.tm_mon+1;

	stringDatetime += "-";

	stringDatetime += timeinfo.tm_year+1900;
	stringDatetime += "/";

	if(timeinfo.tm_hour < 10) 	stringDatetime += "0";
	stringDatetime += timeinfo.tm_hour;

	stringDatetime += ":";

	if(timeinfo.tm_min < 10) 	stringDatetime += "0";
	stringDatetime += timeinfo.tm_min;

	stringDatetime += ":";

	if(timeinfo.tm_sec < 10) 	stringDatetime += "0";
	stringDatetime += timeinfo.tm_sec;

	// Serial.print("stringDatetime: ");
	// Serial.println(stringDatetime);

	return stringDatetime;
}
