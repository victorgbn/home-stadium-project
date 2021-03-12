#include <Adafruit_NeoPixel.h> // Charge la librairie Neo Pixel d'Adafruit utilisé pour piloter le ruban de LED

#define PIXEL_PIN 6 // On définit le pin où est connecté la patte DATA du bandeau
#define PIXEL_COUNT 2 // On définit le nombre de LED compris sur le Ruban de LED soit 150 pour le ruban de 5m50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800); // Paramètre l'objet strip qui correspond à toute les LED du ruban

void setup() {
strip.begin(); // Lance la connection
strip.show(); // Initialise toute les led à 'off'
}

/* Définition des couleurs */
int VIOLET[3] = {60, 41, 85};
int BLANC[3] = {255, 255, 255};
int VERT_N[3] = {0, 97, 47};
int VERT_S[3] = {0, 106, 50};
int JAUNE[3] = {255, 220, 0};
int BLEU[3] = {23, 45, 92};
int ROUGE[3] = {241, 73, 75};
int OFF[3] = {0, 0, 0}; // Éteint

void loop() {
strip.setBrightness(100); // Règle la luminosité à 100 % de la luminosité maximale
allBicolor(VERT_N,JAUNE);
}

void allBicolor(int COLOR_LEFT[], int COLOR_RIGHT[])
{
for(int i = 0 ; i < PIXEL_COUNT/2 ; i++)
{
strip.setPixelColor(i, COLOR_LEFT[0], COLOR_LEFT[1], COLOR_LEFT[2]);
}

for(int i = PIXEL_COUNT/2 ; i < PIXEL_COUNT ; i++)
{
strip.setPixelColor(i, COLOR_RIGHT[0], COLOR_RIGHT[1], COLOR_RIGHT[2]);
}
strip.show();
}
