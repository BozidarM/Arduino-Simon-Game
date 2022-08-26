//Ubacivanje TimerOne biblioteke u projekat
#include <TimerOne.h>

//Nizovi u koje čuvamo pinove led dioda i pinove tastera
const int leds[] = {9, 10, 11, 12};
const int buttons[] = {4, 5, 6, 7};

//Definišemo konstantu koja će biti korišćena za pin 13 (dioda za greške)
#define LED_WRONG 13

//Definišemo konstantu koja će biti korišćena za pin 2 (dioda za startovanje sekvenci)
#define BUTTON_START 2
//Definišemo konstantu koja će biti korišćena za pin 3 (dioda za prekid igre)
#define BUTTON_END 3

//Definisanje konstanti za stanja radi čitljivijeg koda
#define START_SEQUENCE    0
#define USER_GUESSING     1
#define PROGRAM_CHECKING  2
#define SHOW_NEW_SEQUENCE 3
#define GAME_OVER         4

//Definisanje state varijablje
int state;

//Definisanje nizova za sekvencu igre i za sekvencu korisnika koje se kasnije upoređuju
int game_sequence[100] = {0};
int user_sequence[100] = {0};

//Definisanje početnog nivoa, da u prvoj sekvenci zasvetle 2 diode
int sequence_length = 2;

//Definisanje varijable za greške korisnika
int mistakes = 0;
//Definisanje varijable za broj mogućih grešaka korisnika
int available_mistakes = 3;
//Definisanje varijable za osvojene poene korisnika
int score = 0;
//Postavljamo pocetnu vrednost crvene lampice na LOW odnosno 0 odnosno iskljuceno 
int red_led = LOW;
//Pokazivač na trenutni element igračeve sekvence
int seq_pointer = 0;

//Definisanje i setovanje varijable na false, služi sa pokretanje igre klikom na dugme
bool start_game = false;

//Funkcija za eksterni prekid koji služi za startovanje igre
void button_interrupt_start(){
  start_game = true;
}

//Druga funkcija za eksterni prekid koja sluzi da prekine igru
//(menja stanje igre u kraj igre)
void button_interrupt_end(){
  state = GAME_OVER;
}

//Funkcija koja se poziva pri vremenskom prekidu koji je definisan TimerOne bibliotekom
//sluzi za paljenje crvene led diode
void timeInterrupt(){
  //ako ima gresaka onda da ce crvena dioda treperiti
  if(mistakes > 0){
    red_led = !red_led;
  }else{
    red_led = LOW;
  }
}

//Funkcija koja sluzi za kreiranje sekvence to jest
void create_new_sequence(){
  for(int i=0; i < sequence_length; i++){
    //u game_sequence ubacije random indekse dioda
    game_sequence[i] = random(0, 4);
  }
}

//Funkcija koja sluzi da pokrene treptanje diode koja joj je prosledjena
void blink_led(int led) {
  digitalWrite(led, HIGH);
  delay(250);
  digitalWrite(led, LOW);
  delay(250);
}

//Funkcija koja sluzi za pokretanje gore napravljene sekvence
void start_sequence(){
  delay(500);
  for (int i = 0; i < sequence_length; i++) {
    //za svaki diodu iz niza pokrece funkciju blink_led koja sluzi za treptanje diode
    blink_led(leds[game_sequence[i]]);
  }
}

//Funkcija koja sluzi 
void user_guessing_function(){
  //Ako je korisnik kliknuo taster 1
  if(digitalRead(buttons[0]) == HIGH){
    //U njegovu sekvencu upisati broj diode
    user_sequence[seq_pointer] = 0;
    //Ovo sluzi da didoa trepne da bi korisnik znao koju je izabrao
    blink_led(leds[0]);
    //pointer se povecava za 1
    seq_pointer++;
  }
  if(digitalRead(buttons[1]) == HIGH){
    user_sequence[seq_pointer] = 1;
    blink_led(leds[1]);
    seq_pointer++; 
  }
  if(digitalRead(buttons[2]) == HIGH){
    user_sequence[seq_pointer] = 2;
    blink_led(leds[2]);
    seq_pointer++; 
  }
  if(digitalRead(buttons[3]) == HIGH){
    user_sequence[seq_pointer] = 3; 
    blink_led(leds[3]);
    seq_pointer++;
  }
}

//Funkcija koja se koristi za poredjenje dva niza
boolean array_cmp(int *a, int *b){
      
      for (int n=0;n<sequence_length;n++) if (a[n]!=b[n]) return false;

      return true;
}

//Funkcija koja se pokrece kada je igra zavsena bilo da je korisnik pogresio 3 puta,
//presao igru ili iskoristion dugme za prekid igre
void game_over(){
  Serial.print("Games is Over. Your Score was: ");
  Serial.println(score);

  //Vracamo varijable na pocetno stanje
  sequence_length = 2;
  seq_pointer = 0;
  mistakes = 0;
  score = 0;
  red_led = LOW;
  start_game = false;

  //Vraca se stanje na start_sequence da bi mogli ponovo da pokrenemo igru
  state = START_SEQUENCE;
}

void setup() {

  //Omogucava ispis na konzoli 
  Serial.begin(9600);

  //Omogucava arduinu da koristi nasumicno biranje elemenata u nizu 
  randomSeed(analogRead(A0));

  //Za svaki broj u leds i buttons nizu postavlja se mod
  for (int i = 0; i < 4; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT);
  }

  //Postavlja se mod Izlaz za crvenu diodu
  pinMode(LED_WRONG, OUTPUT);
  //Inicializuje se tajmer na 0.8 sekundi
  Timer1.initialize(800000);
  //Timeru se povezuje prekid koji ce da pozove funkciju za vremenski prekid
	Timer1.attachInterrupt(timeInterrupt);

  //Postavlja se mod ulaz za dugme koje pokrece igru
  pinMode(BUTTON_START, INPUT);
  //Tasteru za pokretanje igre se povezuje prekid koji ce da pozove funkciju za
  //spoljasnji prekid
  attachInterrupt(digitalPinToInterrupt(BUTTON_START), button_interrupt_start, RISING);

   //Postavlja se mod ulaz za dugme koje zaustavlja igru
  pinMode(BUTTON_END, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_END), button_interrupt_end, RISING);

  //Na pocetku je stanje inicijalzovano na START_SEQUENCE
  state = START_SEQUENCE;
}

void loop() {
  //Masina stanja koja kontrolise stanja programa
  switch(state){
    //Pocetno stanje za pokretanje sekvence
    case START_SEQUENCE:

      //Ako je uz pomoc spoljasnjeg prekide klikom na dugme start, setovano 
      //start_game na true onda pokrenuti igru
      if(start_game == true){

        //Iskljucuje crvenu diodu
        digitalWrite(LED_WRONG, LOW);

        //Kada je igra pokrenuta vracamo stanje ove varijable na false
        start_game = false;

        //Kreira se nova sekvenca brojeva, u ovom slucaju od dva broja
        create_new_sequence();
      
        //Pokrece se sekvenca
        start_sequence();

        //Setuje se stanje za pogadjanje sekvence za korisnika
        state = USER_GUESSING;

      }
      break;
    //U ovom stanju masina ceka input korisnika
    case USER_GUESSING:

      //Stanje crvene diode u odnosu na broj gresaka
      digitalWrite(LED_WRONG, red_led);

      //Pozivanje funkcije za pogadjanje sekvence
      user_guessing_function();

      //Program ceka sve dok korisnik ne klikne onoliko tastera
      //kolika je dužina sekvence programa
      if(seq_pointer == sequence_length){
        //Kada korisnik zavrsi za unosom prelazi se u stanje provere sekvence
        state = PROGRAM_CHECKING;
      }

      break;
    //Stanje programa u kojem program proverava sekvencu koja je trebalo da se pogodi
    // i sekvencu koju je uneo korisnik
    case PROGRAM_CHECKING:
  
      //Ako je duzina sekvence manja ili jednaka od 100 pokrenuti proveru,
      //ako je duza zavrsiti igru.
      if(sequence_length <= 100){
       
        //Ako su sekvenca koju je napravio program i sekvenca koju je uneo korisnik iste
        if (array_cmp(game_sequence, user_sequence) == true){

          //Povecati duzinu sekvence za 1, nesto kao predji na sledeci nivo
          sequence_length++;

          //Povecavanje poena korisnika za 1
          score++;

          //Setovanje stanja koje sluzi za kreiranje nove sekvence
          state = SHOW_NEW_SEQUENCE;

        }else{

          //Ukoliko se sekvenca korisnka razlikiuje od sekvence programa,
          //Povecati broj gresaka za 1
          mistakes++;

          //Ako korisnik ima manje gresaka od 3 moze da nastavi 
          if(mistakes < available_mistakes){

            Serial.print("Wrong Sequence! Mistake: ");
            Serial.print(mistakes);
            Serial.print(" of ");
            Serial.println(available_mistakes);

            //Ponovo se prebacuje u stanje za generisanje nove sekvence
            state = SHOW_NEW_SEQUENCE;

          }else{

            Serial.print("Wrong Sequence! Mistake: ");
            Serial.print(mistakes);
            Serial.print(" of ");
            Serial.println(available_mistakes);

            //Ukoliko korisnik ima 3 greske prebacuje se u stanje za kraj igre
            state = GAME_OVER;

          }
        }
      }else{
        state = GAME_OVER;
      }
      break;
    //Stanje koje kreira nove sekvence
    case SHOW_NEW_SEQUENCE:

        create_new_sequence();
      
        start_sequence();

        //Postavljanje pokazivaca na 0
        seq_pointer = 0;

        state = USER_GUESSING;

      break;
    case GAME_OVER:

      //Ukljucuje crvenu diodu
      digitalWrite(LED_WRONG, HIGH);

      //Pozivanje funkcije za kraj igre
      game_over();

      break;
    
  }
}
