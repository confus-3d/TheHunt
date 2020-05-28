/*
    The Hunt
    by Move38, Inc. 2020
    Lead development by FJ Rios
    original game by FJ Rios

    Rules:
    12+ Blinks (18 Recomended)
    2 Players (MONSTER=RED and CITIZEN=GREEN)
    1st- Leave a blink alone, this will be KEY blink, and press to chose first player
    2nd- Player attach KEY to the Blinks Board, and press an adjacent Blink to move to starting position.
    3rd- Put KEY alone to change player and do the same for second player 
    GAME START!
    MONSTER Player:
    Move to an adjacent Blink to catch the CITIZEN or
    Press your actual position to SMELL in the surroundings and check if CITIZEN is adjacent to your tile.
    If you SMELL the CITIZEN the board turns YELLOW
    If you catch the CITIZEN, you win and the board turns RED.
    CITIZEN Player:
    Move to an adjacent Blink to avoid the MONSTER or
    Press your actual position to HEAR in the surroundings and check if MONSTER is adjacent to your tile.
    If you HEAR the MONSTER the board turns YELLOW
    If you avoid the MONSTER for several rounds, you win and the board turns GREEN.
    

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018
    @madewithblinks
    www.move38.com
    --------------------
*/


enum gameStates {SLAVE, KEY, MONSTER, CITIZEN, RESET, CLEARM, CLEARC, DETECTED, WIN, LOSE};  //tile states
byte gameState = SLAVE;

enum Hear {NH, NOISE};
byte Hear = NH;

enum Smell {NS, STINK};
byte Smell = NS;

enum keyStates {M, C, E, WM, WC}; //M=MONSTER C= CITIZEN E=Empty WM=Waiting Monster WC=Waiting Citizen
byte keyState = E;

byte coinflip; //For first player

#define STEP_SIZE 10  
#define STEP_TIME_MS 30
int brightness = 1; 
int step = STEP_SIZE;
Timer HeartBeat;
int numNeighbors;
Timer roundTimer;
int CWIN = 25; //Rounds to CITIZEN win


void setup() {
  // put your setup code here, to run once:

}
void loop() {   // main loop
  switch (gameState) {
    case SLAVE:
      slaveLoop();
      slaveDisplayLoop();
      break;
    case KEY:
      keyLoop();
      keyDisplayLoop();
      break;
    case MONSTER:
      monsterLoop();
      monsterDisplayLoop();
      break;
    case CITIZEN:
      citizenLoop();
      citizenDisplayLoop();
      break;
    case RESET:
      resetLoop();
      resetDisplayLoop();
      break;
    case CLEARM:
      clearLoop();
      monsterDisplayLoop();
      break;
    case CLEARC:
      clearLoop();
      citizenDisplayLoop();
      break;
    case DETECTED:
      detectedLoop();
      detectedDisplayLoop();
      break;
    case WIN:
      winLoop();
      winDisplayLoop();
      break;
    case LOSE:
      loseLoop();
      loseDisplayLoop();
      break;
  }

//RESET GAME

  if (buttonLongPressed()) {
    gameState = RESET;
    roundTimer.set(2000);
  }

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
      }
    }
  }
  
  //dump button data
  buttonSingleClicked();
  buttonDoubleClicked();
  buttonPressed();
  
  //comunication
  byte sendData;
  switch (gameState) {
    case RESET:
      sendData = (gameState << 4);
      break;    
    case CLEARM:
      sendData = (gameState << 4);
      break;
    case CLEARC:
      sendData = (gameState << 4);
      break;
    case KEY:
      sendData = (keyState << 3);
      break;
    case MONSTER:
      sendData = (gameState << 4) + (Hear << 2) + (Smell << 1);
      break;
    case CITIZEN:
      sendData = (gameState << 4) + (Hear << 2) + (Smell << 1);
      break;
    case WIN:
      sendData = (gameState << 4);
      break;
    case LOSE:
      sendData = (gameState << 4);
      break;
  }

  setValueSentOnAllFaces(sendData);
}

void slaveLoop() {

numNeighbors = 0;
  
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
      numNeighbors++;
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      byte neighborKeyState = getKeyState(getLastValueReceivedOnFace(f));
      if (neighborKeyState == M) {
        gameState = MONSTER;
      }
      if (neighborKeyState == C) {
        gameState = CITIZEN;
      }
      if (neighborGameState == MONSTER) {
        gameState = MONSTER;
      }
      if (neighborGameState == CITIZEN) {
        gameState = CITIZEN;
      }
    }
  }

  if (buttonDoubleClicked()){
    if (numNeighbors == 0){
      gameState = KEY;
    }
  }
}

void keyLoop() {

if (keyState == E){
  if (buttonSingleClicked() || buttonDoubleClicked()){
      coinflip = random(1);
      if (coinflip == 0) {
        keyState = M; 
      } else {
         keyState = C;
      }
  }
}

numNeighbors = 0;

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      numNeighbors++;
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == CLEARM) {
          keyState = WC;
      }
      if (neighborGameState == CLEARC){
          keyState = WM;
      }
    }
  }
  if (numNeighbors == 0){
        if (keyState == WC){
          keyState = C;
        }
        if (keyState == WM){
          keyState = M;
        } 
  }
}

void monsterLoop() {

FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARM) {
                Hear = NH;
                gameState = CLEARM;
                roundTimer.set(2000);
              }
          }
  }

if (Hear == NOISE){
  if (buttonSingleClicked()){
       FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborSmell = getSmell(getLastValueReceivedOnFace(f));
              if (neighborSmell == STINK){
                  gameState = DETECTED;
                  roundTimer.set(2000);
              }
          }
        }
        if (gameState != DETECTED){
                gameState = CLEARM;
                roundTimer.set(2000);
        }
  }

} else if (buttonSingleClicked()) {
      FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborKeyState = getKeyState(getLastValueReceivedOnFace(f));
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborKeyState == M || neighborHear == NOISE){
                  Hear = NOISE;
                  gameState = CLEARM;
                  roundTimer.set(2000);
                   
              }
            }
        }
    }
}
void citizenLoop() {

FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARC) {
                Smell = NS;
                gameState = CLEARC;
                roundTimer.set(2000);
              }
          }
  }


if (Smell == STINK){
  if (buttonSingleClicked()){
       FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborHear == NOISE){
                  gameState = DETECTED;
                  roundTimer.set(2000);
              }
          }
        }
        if (gameState != DETECTED){
                gameState = CLEARC;
                roundTimer.set(2000);
        }
  }
} else if (buttonSingleClicked()) {
  FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborKeyState = getKeyState(getLastValueReceivedOnFace(f));
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborKeyState == C || neighborHear == STINK){
                  Smell = STINK;
                  CWIN--;
                  gameState = CLEARC;
                  roundTimer.set(2000);
                }   
             }
         }
     }
    
}

void resetLoop() {
  if (roundTimer.isExpired()){
        gameState = SLAVE;  
        Hear = NH;
        Smell = NS;
        keyState = E;  
  }
}

void clearLoop() {
  if (CWIN <= 0 && Hear == NH){
    gameState = WIN;
  }
  if (Smell == STINK && Hear == NOISE){
    gameState = LOSE;
  }
  if (roundTimer.isExpired()){
        gameState = SLAVE;
  }
  FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == WIN){
                  gameState = WIN;
                } 
              if (neighborGameState == LOSE){
                  gameState = LOSE;
                }   
             }
         }
}

void detectedLoop() {
  if (roundTimer.isExpired()){
        gameState = SLAVE;
  }
}

void winLoop() {

}

void loseLoop() {

}

// GRAPHICS 

void slaveDisplayLoop() {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( WHITE ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }
}

void keyDisplayLoop() {
  switch (keyState) {
    case M:
      setColor(RED);
      break;
    case C:
      setColor(GREEN); 
      break;
    case WM:
      setColor(YELLOW);
      break;
    case WC:
      setColor(YELLOW);
      break;
    case E:
      setColorOnFace(RED, 0);
      setColorOnFace(RED, 1);
      setColorOnFace(RED, 2);
      setColorOnFace(GREEN, 3);
      setColorOnFace(GREEN, 4);
      setColorOnFace(GREEN, 5);
      break;
  }
}
void monsterDisplayLoop() {
    if (Hear == NOISE) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( RED ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }
   } else {
      setColor(OFF);  
   }
}

void citizenDisplayLoop() {
    if (Smell == STINK) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( GREEN ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }
   } else {
      setColor(OFF);  
   }  
}

void resetDisplayLoop() {
  setColor(OFF);                         
}


void detectedDisplayLoop() {
  setColor(YELLOW);                         
}

void winDisplayLoop() {
  setColor(GREEN);                         
}

void loseDisplayLoop() {
  setColor(RED);                         
}

//COMMUNICATION

byte getGameState(byte data) {
  return (data >> 4);
}

byte getHear(byte data) {
  return (data >> 2);
}

byte getSmell(byte data) {
  return (data >> 1);
}

byte getKeyState(byte data) {
  return (data >> 3);
}
