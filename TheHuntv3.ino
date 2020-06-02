/*
    The Hunt
    Lead development by FJ Rios
    original game by FJ Rios

    Rules:
    12+ Blinks (18 Recomended)
    2 Players (MONSTER=RED and CITIZEN=GREEN)
    1st- Leave a blink alone and double click, this will be KEY blink. Click once to chose first player.
    2nd- Player attach KEY to the Blinks Board, and press an adjacent Blink to move to starting position.
    3rd- Put KEY alone to change player and do the same for second player 
    GAME START!
    MONSTER Player (connect KEY to board and Pick one action):
    -Move to an adjacent Blink to catch the CITIZEN or
    -Press your actual position to SMELL in the surroundings and check if CITIZEN is adjacent to your tile.
    If you SMELL the CITIZEN the board turns YELLOW
    If you catch the CITIZEN, you win and the board turns RED.
    CITIZEN Player (connect KEY to board Pick one action):
    -Move to an adjacent Blink to avoid the MONSTER or
    -Press your actual position to HEAR in the surroundings and check if MONSTER is adjacent to your tile.
    If you HEAR the MONSTER the board turns YELLOW
    If you avoid the MONSTER for several rounds, you win and the board turns GREEN.
    ADVANCED MODE:
    You can, instead using your action, take a tile from the board and put it into another place.

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018

    @madewithblinks
    www.move38.com
    --------------------
*/


enum gameStates {SLAVE, MONSTER, CITIZEN, RESET, CLEARM, CLEARC, DETECTED, UNDETECTED, WIN, LOSE, KM, KC, KE, KWM, KWC, SWAP};  //tile states
byte gameState = SLAVE;

enum Hear {NH, NOISE};
byte Hear = NH;

enum Smell {NS, STINK};
byte Smell = NS;

byte coinflip; //For first player
int randomizer = random(5); //For random face colors at the end of the game

#define STEP_SIZE 10  
#define STEP_TIME_MS 30
int brightness = 1; 
int step = STEP_SIZE;
Timer HeartBeat;
int numNeighbors;
int numDetected;
Timer roundTimer;
int CWIN = 50; //Rounds to CITIZEN win. Each player count as 1 round.


void setup() {
  // put your setup code here, to run once:

}
void loop() {   // main loop
  switch (gameState) {
    case SLAVE:
      slaveLoop();
      slaveDisplayLoop();
      break;
    case KM:
    case KC:
    case KE:
    case KWM:
    case KWC:
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
    case UNDETECTED:
      detectedLoop();
      detectedDisplayLoop();
      break;
    case WIN:
      endingLoop();
      winDisplayLoop();
      break;
    case LOSE:
      endingLoop();
      loseDisplayLoop();
      break;
    case SWAP:
      swapLoop();
      detectedDisplayLoop();
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
      sendData = (gameState);
      break;    
    case SLAVE:
      sendData = (gameState);
      break; 
    case CLEARM:
      sendData = (gameState);
      break;
    case CLEARC:
      sendData = (gameState);
      break;
    case KM:
    case KC:
    case KE:
    case KWM:
    case KWC:
      sendData = (gameState);
      break;
    case MONSTER:
      sendData = (gameState) + (Hear << 4) + (Smell << 5);
      break;
    case CITIZEN:
      sendData = (gameState) + (Hear << 4) + (Smell << 5);
      break;
    case DETECTED:
      sendData = (gameState);
      break;
    case UNDETECTED:
      sendData = (gameState);
      break;
    case WIN:
      sendData = (gameState);
      break;
    case LOSE:
      sendData = (gameState);
      break;
    case SWAP:
      sendData = (gameState);
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
      if (neighborGameState == KM) {
        gameState = MONSTER;
      }
      if (neighborGameState == KC) {
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
      gameState = KE;
    }
  }
}

void keyLoop() {

if (gameState == KE){
  if (buttonSingleClicked() || buttonDoubleClicked()){
      coinflip = random(1);
      if (coinflip == 0) {
        gameState = KM; 
      } else {
         gameState = KC;
      }
  }
}

numNeighbors = 0;

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      numNeighbors++;
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == CLEARM) {
          gameState = KWC;
      }
      if (neighborGameState == CLEARC){
          gameState = KWM;
      }
    }
  }
  if (numNeighbors == 0){
        if (gameState == KWC){
          gameState = KC;
        }
        if (gameState == KWM){
          gameState = KM;
        } 
  }
}

void monsterLoop() {

FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            numNeighbors++;
              if (neighborGameState == CLEARM) {
                Hear = NH;
                gameState = CLEARM;
                roundTimer.set(2000);
              }
              if (neighborGameState == DETECTED) {
                gameState = CLEARM;
                roundTimer.set(2000);
              }
              if (neighborGameState == UNDETECTED) {
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
                gameState = UNDETECTED;
                roundTimer.set(2000);
        }
  }

} else if (buttonSingleClicked()) {
      FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborGameState == KM || neighborHear == NOISE){
                  Hear = NOISE;
                  gameState = CLEARM;
                  roundTimer.set(2000);
                   
              }
            }
        }
    }

numNeighbors = 0;  //Extra feature, change a tiles

          FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              numNeighbors++;
              if (neighborGameState == SWAP){
                  gameState = CLEARM;
                  roundTimer.set(2000);
                   
              }
            }
        }
            if (numNeighbors == 0 && Hear == NH){
                CWIN--;
                gameState = SWAP;
      
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
              if (neighborGameState == DETECTED) {
                gameState = CLEARC;
                roundTimer.set(2000);
              }
              if (neighborGameState == UNDETECTED) {
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
                gameState = UNDETECTED;
                roundTimer.set(2000);
        }
  }
} else if (buttonSingleClicked()) {
  FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborGameState == KC || neighborHear == STINK){
                  Smell = STINK;
                  gameState = CLEARC;
                  roundTimer.set(2000);
                }   
             }
         }
     }


numNeighbors = 0; //Extra feature, change a tiles

          FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              numNeighbors++;
              if (neighborGameState == SWAP){
                  gameState = CLEARC;
                  roundTimer.set(2000);
                   
              }
            }
        }
            if (numNeighbors == 0 && Smell == NS){
                CWIN--;
                gameState = SWAP;
      
    }

}

void swapLoop() {
  numNeighbors = 0;
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (numNeighbors >= 2){
                  gameState = SLAVE;
              }
            }
        }
}

void resetLoop() {
  if (roundTimer.isExpired()){
        gameState = SLAVE;  
        Hear = NH;
        Smell = NS;  
  }
}

void clearLoop() {

  if (CWIN <= 0 && Hear == NH && Smell == STINK){
    gameState = WIN;
  }
  if (Smell == STINK && Hear == NOISE){
    gameState = LOSE;
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
    if (roundTimer.isExpired()){
        CWIN--;
        gameState = SLAVE;
  }
}

void detectedLoop() {
  if (roundTimer.isExpired()){
        CWIN--;
        gameState = SLAVE;
  }
}

void endingLoop() {

}


// GRAPHICS 

void slaveDisplayLoop() {
      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {
          step = -step;
        }
        if (brightness + step < 0 ) {
          step = -step;
          randomizer = random(5);
        }
      brightness += step;  
      HeartBeat.set( STEP_TIME_MS );
      }
      
      setColorOnFace( dim( YELLOW,  brightness  ), randomizer);                       
}


void keyDisplayLoop() {
  switch (gameState) {
    case KM:
      setColor(RED);
      break;
    case KC:
      setColor(GREEN); 
      break;
    case KWM:
      setColor(BLUE);
      break;
    case KWC:
      setColor(BLUE);
      break;
    case KE:
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
  numDetected = 0;
  
    if (Hear == NOISE) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( RED ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }
   } 
   if (Hear == NH) {
          FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == DETECTED){
                numDetected++;
              }
            }
        }
    
      if (numDetected == 1){
        setColor(YELLOW);  
      }
      if (numDetected == 0){
        setColor(OFF);  
      }
   }
}

void citizenDisplayLoop() {
    numDetected = 0;
    
    if (Smell == STINK) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( GREEN ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }
   } 
   if (Smell == NS) {
          FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == DETECTED){
                numDetected++;
              }
            }
        }
    
      if (numDetected == 1){
        setColor(YELLOW);  
      }
      if (numDetected == 0){
        setColor(OFF);  
      }
   }
}

void resetDisplayLoop() {
  setColor(OFF);                         
}


void detectedDisplayLoop() {
  setColor(YELLOW);                         
}

void winDisplayLoop() {

      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {
          step = -step;
        }
        if (brightness + step < 0 ) {
          step = -step;
          randomizer = random(5);
        }
      brightness += step;  
      HeartBeat.set( STEP_TIME_MS );
      }
      
      setColorOnFace( dim( GREEN,  brightness  ), randomizer);              
}

void loseDisplayLoop() {
      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {
          step = -step;
        }
        if (brightness + step < 0 ) {
          step = -step;
          randomizer = random(5);
        }
      brightness += step;  
      HeartBeat.set( STEP_TIME_MS );
      }
      
      setColorOnFace( dim( RED,  brightness  ), randomizer);                       
}

//COMMUNICATION

byte getGameState(byte data) { //BYTE CDEF
  return (data & 15);
}

byte getHear(byte data) { //BYTE B
  return ((data >> 4) & 1);
}

byte getSmell(byte data) { //BYTE A
  return ((data >> 5) & 1);
}
