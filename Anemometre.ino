/**
 * Capteur Générique avec une carte Arduino
 */
#include "capteur_generique_esp_now.h"

#define BOARD_ID 2 // Mettre l'identidiant du capteur ici

int getBoardId(){
  return BOARD_ID;
}

/* constantes pour la broche de mesure */
const byte PIN_SIGNAL = 2;
unsigned long debutObservation = millis();   // Spour stocker le moment de la première  mesure d'une période d'obervation

#define DUREE_OBSERVATION 60*1000

bool envoiMessage=false;
bool demarrage=true;

/* Variables pour la mesure */
volatile unsigned long periode = 0;
volatile unsigned long rpm = 0;
volatile unsigned long vitesse = 0;
float frequence=0;
unsigned long previousMicros = 0;

/** Fonction d'interruption pour la mesure entre deux fronts */
void tick() {
  unsigned long currentMicros = millis();
  /* Calcule le temps écoulé depuis le précédent front */
  periode= currentMicros - previousMicros;
  /* Met à jour la variable pour la prochaine interruption */
  previousMicros = currentMicros;

}

/** Fonction setup() */
void setup() {
  /* Initialise le port série */
  Serial.begin(115200);
  /* Initialise le réseau ESP-NOW pour échanger avec le serveur */
  setup_ESP_NOW();
  /* Met la broche en entrée */  
  pinMode(PIN_SIGNAL, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SIGNAL), tick, RISING);
  /* Permet une première màj de "periode" */
  delay(3000); // On attend 3 secondes avant de commencer les calculs 
  
}
/*
Léger (0 à 9 km/h)
Modéré (10 à 40 km/h)
Fort/venteux (41 à 60 km/h)
Très fort/coups de vent (61 à 90 km/h)
Très fort/force de tempête (plus de 91 km/h)
Force d'ouragan (plus de 115 km/h)
*/
String getStringNiveauVent(){
  String _s="";
  if(vitesse>0&&vitesse<=10){
    _s="Vent leger";
  } else if(vitesse>10&&vitesse<=40){
    _s="vent modere";
  } else if(vitesse>40&&vitesse<=60){
    _s="Vent Fort";
  } else if(vitesse>40&&vitesse<=90){
    _s="Vent très fort";
  } else if(vitesse>90&&vitesse<=115){
    _s="Tempête";
  } else if(vitesse>115){
    _s="Ouragan";
  } else if(vitesse==0){
    _s="Aucun vent";
  }
  return _s;
}
/** Fonction loop() */
void loop() {
  if (autoPairing() == PAIR_PAIRED&&envoiMessage) {
    // prépare le message à envoyer puis y ajoute toutes les mesures recceuillies par le capteur
    prepareMessage();
    addMesure(vitesse,getStringNiveauVent().c_str(),0);
    // Envoi le message via ESP-NOW
    sendMessage();
    envoiMessage=false;
  } else {
    if(millis()-debutObservation>DUREE_OBSERVATION){
      if(periode!=0){frequence=1000000/periode;}else{frequence=0;};
      rpm=frequence * 60/1000 ;
      vitesse=2.4*rpm/60;
      envoiMessage=true;
      debutObservation=millis();
    } else if(demarrage){
      demarrage=false;
      envoiMessage=true;
    }
    delay(1000*60); // un envoi toutes les minutes
  } 
  
}
 
