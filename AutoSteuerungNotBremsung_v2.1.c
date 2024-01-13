#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>    //https://www.sbarjatiya.com/notes_wiki/index.php/Using_ncurses_library_with_C
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

//gcc -o name name.c -lwiringPi -lpthread -lncurses

#define H1Input1 13 // VLinks: 0 = Forward, 1 = Backward // Vorne Links Vorwaerts
#define H1Input2 15 // VLinks: 1 = Forward, 0 = Backward // Vorne Links Rueckwaerts 

#define H1Input3 18 // VRechts: 0 = Forward, 1 = Backward // Vorne Rechts Vsorwaerts
#define H1Input4 22 // VRechts: 1 = Forward, 0 = Backward // Vorne Rechts Rueckwaerts

#define H1ENA 19 // VLinks: Drehzahl
#define H1ENB 24 // VRechts: Drehzahl


#define H2Input1 29 // HLinks: 0 = Forward, 1 = Backward // Hinten Links Vorwaerts
#define H2Input2 31 // HLinks: 1 = Forward, 0 = Backward // Hinten Links Rueckwaerts

#define H2Input3 36 // HRechts: 0 = Forward, 1 = Backward //Hinten Rechts Vorwaerts
#define H2Input4 38 // HRechts: 1 = Forward, 0 = Backward //Hinten Rechts Rueckwaerts

#define H2ENA 26 // HLinks: Drehzahl
#define H2ENB 40 // HRechts: Drehzahl

#define TRIGGER 16 //GPIO: 23  //Phys: 16
#define ECHO 11 //GPIO: 17  //Phys: 11

#define Periodendauer 100 // PWM-Periodendauer (Wertebereich)

double distanz;
int notStop = 0;

void reset(){
        digitalWrite(H1Input1, LOW);
        digitalWrite(H1Input2, LOW);
        digitalWrite(H1Input3, LOW);
        digitalWrite(H1Input4, LOW);
        digitalWrite(H2Input1, LOW);
        digitalWrite(H2Input2, LOW);
        digitalWrite(H2Input3, LOW);
        digitalWrite(H2Input4, LOW);
}

// Funktion, um die Distanz in cm zu berechnen
double distanz() {
    // Variablen für die Zeitmessung
    struct startZeit, stopZeit;
    long start, stop;
    double total;

    // setze TRIGGER auf HIGH
    digitalWrite(TRIGGER, HIGH);

    // setze TRIGGER nach 10us aus LOW
    delay(0.01);
    digitalWrite(TRIGGER, LOW);

    // speichere Startzeit
    while (digitalRead(ECHO) == LOW) {
        clock_gettime(CLOCK_REALTIME, &startZeit);
    }

    // speichere Ankunftszeit
    while (digitalRead(ECHO) == HIGH) {
        clock_gettime(CLOCK_REALTIME, &stopZeit);
    }

    // berechne die Differenz zwischen Start und Ankunft in Mikrosekunden
    start = startZeit.tv_sec * 1000000 + startZeit.tv_usec;
    stop = stopZeit.tv_sec * 1000000 + stopZeit.tv_usec;
    total = (double) (stop - start);

    // mit der Schallgeschwindigkeit (34300 cm/s) multiplizieren
    // und durch 2 teilen, da hin und zurueck
    double distanz = (total * 34300) / 2 / 1000000;

    if(distanz > 10.0) {
        notStop = 0;
    }

    return distanz;
}

int Eingabe; 

int main() {

    int i = 0;

    int H1ENAImpulsdauer = 100;
    int H1ENBImpulsdauer = 100;
    int H2ENAImpulsdauer = 100;
    int H2ENBImpulsdauer = 100;

    wiringPiSetupPhys();

    pinMode(H1Input1, OUTPUT);
    pinMode(H1Input2, OUTPUT);
    pinMode(H1Input3, OUTPUT);
    pinMode(H1Input4, OUTPUT);
    pinMode(H1ENA, OUTPUT);
    pinMode(H1ENB, OUTPUT);

    pinMode(H2Input1, OUTPUT);
    pinMode(H2Input2, OUTPUT);
    pinMode(H2Input3, OUTPUT);
    pinMode(H2Input4, OUTPUT);
    pinMode(H2ENA, OUTPUT);
    pinMode(H2ENB, OUTPUT);

    pinMode(TRIGGER, OUTPUT);
    pinMode(ECHO, INPUT);

    softPwmCreate (H1ENA, 0, Periodendauer);
    softPwmCreate (H1ENB, 0, Periodendauer);
    softPwmCreate (H2ENA, 0, Periodendauer);
    softPwmCreate (H2ENB, 0, Periodendauer);

    initscr();  //initisalisierung der N-curses funktionen
    raw();      //Man muss kein ENter mehr drücken
    noecho();   //Gibt die Eingabe NICHT im terminal aus

    wiringPiISR(InterruptTrigger,INT_EDGE_FALLING,&NotBremsung);

    while(i < 1) {

        softPwmWrite(H1ENA, H1ENAImpulsdauer);
        softPwmWrite(H1ENB, H1ENBImpulsdauer);

        softPwmWrite(H2ENA, H2ENAImpulsdauer);
        softPwmWrite(H2ENB, H2ENBImpulsdauer);

        distanz();

        printf("Abstand %lf cm",distanz);

        // Auto-Stopp bei einem Abstand unter 10 cm
        if (distanz < 10.0 && notStop == 0) {
            notStop = 1;
            printf("Auto gestoppt! Abstand zu gering.\n");
            reset();
        }

        Eingabe = getch();

        switch(Eingabe) {
             
             case 'w':
            digitalWrite(H1Input1, HIGH);       //Vorne Links fährt nach vorne
            digitalWrite(H1Input2, LOW);
            digitalWrite(H1Input3, HIGH);       //Vorne Rechts fährt nach vorne
            digitalWrite(H1Input4, LOW);

            digitalWrite(H2Input1, HIGH);       //Hinten Links fährt nach vorne
            digitalWrite(H2Input2, LOW);
            digitalWrite(H2Input3, HIGH);       //Hinten Rechts fährt nach vorne
            digitalWrite(H2Input4, LOW);
            Eingabe = 0;
            break;


             case 's':


            digitalWrite(H1Input1, LOW);        //Vorne Links fährt nach hinten
            digitalWrite(H1Input2, HIGH);
            digitalWrite(H1Input3, LOW);        //Vorne Rechts fährt nach hinten
            digitalWrite(H1Input4, HIGH);

            digitalWrite(H2Input1, LOW);        //Hinten Links fährt nach hinten
            digitalWrite(H2Input2, HIGH);
            digitalWrite(H2Input3, LOW);        //Hinten Rechts fährt nach hinten
            digitalWrite(H2Input4, HIGH);

            break;


             case 'a':

            digitalWrite(H1Input1, LOW);        //Vorne Links fährt nach hinten
            digitalWrite(H1Input2, HIGH);
            digitalWrite(H1Input3, HIGH);       //Vorne Rechts fährt nach vorne
            digitalWrite(H1Input4, LOW);

            digitalWrite(H2Input1, LOW);        //Hinten Links fährt nach hinten
            digitalWrite(H2Input2, HIGH);
            digitalWrite(H2Input3, HIGH);       //Hinten Rechts fährt nach vorne
            digitalWrite(H2Input4, LOW);


            break;


             case 'd':

            digitalWrite(H1Input1, HIGH);       //Vorne Links fährt nach vorne
            digitalWrite(H1Input2, LOW); 
            digitalWrite(H1Input3, LOW);        //Vorne Rechts fährt nach hinten
            digitalWrite(H1Input4, HIGH);

            digitalWrite(H2Input1, HIGH);       //Hinten Links fährt nach vorne
            digitalWrite(H2Input2, LOW);
            digitalWrite(H2Input3, LOW);        //Hinten Rechts fährt nach hinten
            digitalWrite(H2Input4, HIGH);

            break;


             case 'r':
            i++; 


            break;

             case 'q':
            digitalWrite(H1Input1, LOW);        //Vorne Links fährt nach hinten
            digitalWrite(H1Input2, HIGH);
            digitalWrite(H1Input3, HIGH);       //Vorne Rechts fährt nach vorne
            digitalWrite(H1Input4, LOW);

            digitalWrite(H2Input1, HIGH);        //Hinten Links fährt nach vorne 
            digitalWrite(H2Input2, LOW);
            digitalWrite(H2Input3, HIGH);        //Hinten Rechts fährt nach vorne
            digitalWrite(H2Input4, LOW);


            break;

             case 'e':

            digitalWrite(H1Input1, HIGH);       //Vorne Links fährt nach vorne
            digitalWrite(H1Input2, LOW); 
            digitalWrite(H1Input3, LOW);        //Vorne Rechts fährt nach hinten
            digitalWrite(H1Input4, HIGH);

            digitalWrite(H2Input1, HIGH);       //Hinten Links fährt nach vorne 
            digitalWrite(H2Input2, LOW);
            digitalWrite(H2Input3, HIGH);       //Hinten Rechts fährt nach vorne
            digitalWrite(H2Input4, LOW);


            break;

            case'+':

            if(H1ENAImpulsdauer == 100 || H1ENBImpulsdauer == 100 || H2ENAImpulsdauer == 100 || H2ENBImpulsdauer == 100){
                printw("High Engine Power. Can't go Higher.\n");
            }

            else{
                printw("Increased Power by 10\n");
                H1ENAImpulsdauer = H1ENAImpulsdauer + 10;
                H1ENBImpulsdauer = H1ENBImpulsdauer + 10;
                H2ENAImpulsdauer = H2ENAImpulsdauer + 10;
                H2ENBImpulsdauer = H2ENBImpulsdauer + 10;
            }


            break;

            case'-':

            if(H1ENAImpulsdauer == 0 || H1ENBImpulsdauer == 0 || H2ENAImpulsdauer == 0 || H2ENBImpulsdauer == 0){
                printw("Low Engine Power. Can't go Lower.\n");
            }

            else{
                printw("Lowered Power by 10\n");
                H1ENAImpulsdauer = H1ENAImpulsdauer - 10;
                H1ENBImpulsdauer = H1ENBImpulsdauer - 10;
                H2ENAImpulsdauer = H2ENAImpulsdauer - 10;
                H2ENBImpulsdauer = H2ENBImpulsdauer - 10;
            }


            break;

            default:

            reset();

            break;
            }

    }

    endwin();   //Schließt das Terminalfenster (Ist dafür da damit das Terminal nicht abstürzt)
    return 0;
}