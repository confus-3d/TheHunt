/*
    The Hunt
    Lead development by FJ Rios
    original game by FJ Rios

    12+ Blinks (18 Recomended)          
    2 Players (MONSTER=RED and CITIZEN=GREEN)

    Rules:
    1st- Leave a blink alone and double click, this will be KEY blink. Click or doubleclick once to chose first player.
    2nd- Player attach KEY to the Blinks Board, and press an adjacent Blink to move to starting position.
    3rd- Put KEY alone to change player and do the same for second player 
    GAME START!
    MONSTER Player (connect KEY to board and Pick one action):
    -Move to an adjacent Blink to catch the CITIZEN or
    -Press your actual position to SMELL in the surroundings and check if CITIZEN is adjacent to your tile.
    If you SMELL the CITIZEN adjacent tiles turn YELLOW
    If you catch the CITIZEN, you win and the board turns RED.
    CITIZEN Player (connect KEY to board Pick one action):
    -Move to an adjacent Blink to avoid the MONSTER or
    -Press your actual position to HEAR in the surroundings and check if MONSTER is adjacent to your tile.
    If you HEAR the MONSTER adjacent tiles turn YELLOW
    If you avoid the MONSTER for several rounds, you win and the board turns GREEN.
    ADVANCED MODE: (18 Blinks Recomended)
    You can, instead using your action, take a tile from the board and put it into another place.
    This count as your action.

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018

    @madewithblinks
    www.move38.com
    --------------------
*/


enum gameStates {SLAVE, MONSTER, CITIZEN, RESET, CLEARM, CLEARC, DETECTED, UNDETECTED, WIN, LOSE, KM, KC, KE, KWM, KWC, SWAP};  //16 tile states sended in BYTE CDEF
byte gameState = SLAVE; //Starting state (Cornfield)

enum Hear {NH, NOISE}; //Define MONSTER position sended in BYTE B
byte Hear = NH;

enum Smell {NS, STINK}; //Define CITIZEN position sended in BYTE A
byte Smell = NS;

byte coinflip; //For first player
int randomizer = random(5); //For random face colors at the end of the game

#define STEP_SIZE 10  //Fading animations
#define STEP_TIME_MS 30
int brightness = 1; 
int step = STEP_SIZE;
Timer HeartBeat;
int resetPressed;
Timer Wheel;
#define LOOP 200
int numNeighbors;
int numDetected;

#define ROUND_TIME 2000 //Time showing board after any action
Timer roundTimer;

int CWIN = 50; //Rounds to CITIZEN win. Each player count as 1 round. (25 CITIZEN rounds to win)

void setup() {  //Setup code not used
}

void loop() {   //Main Loop of the tile
  switch (gameState) { //Detects gamestate
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

//RESET GAME Reset all tiles conected
if (resetPressed == 0) {
  if (buttonLongPressed()) {
    resetPressed = 1;
    gameState = RESET;
    roundTimer.set(ROUND_TIME);
  }

  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        resetPressed = 1;
        gameState = RESET;
        roundTimer.set(ROUND_TIME);
      }
    }
  }
} 
//Button imput used
  buttonSingleClicked();
  buttonDoubleClicked();
  buttonPressed();
  
//Tile comunication
  byte sendData;
  switch (gameState) {
    case RESET: 
    case SLAVE:
    case CLEARM:
    case CLEARC:
    case KM:
    case KC:
    case KE:
    case KWM:
    case KWC:
    case DETECTED:
    case UNDETECTED:
    case WIN:
    case LOSE:
    case SWAP:
      sendData = (gameState);
      break;
    case MONSTER:
      sendData = (gameState) + (Hear << 4) + (Smell << 5);
      break;
    case CITIZEN:
      sendData = (gameState) + (Hear << 4) + (Smell << 5);
      break;
  }

  setValueSentOnAllFaces(sendData);
}

void slaveLoop() { //Starting gamestate (Cornfield)
  if (roundTimer.isExpired()){
      resetPressed = 0;
  }
  
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor?? Change ALL Cornfield if KEY attached.
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
      if (neighborGameState == WIN){
        gameState = WIN;
      } 
      if (neighborGameState == LOSE){
        gameState = LOSE;
      }
    }
  }

  if (buttonDoubleClicked()){ //Turn a Blink into KEY blink
    if (isAlone()){
      gameState = KE;
    }
  }
}

void keyLoop() { //KEY change during gameplay and alternate players

if (gameState == KE){ //Initial coinflip to determine starting player
  if (buttonSingleClicked() || buttonDoubleClicked()){
      Hear = NOISE;
      Smell = STINK; 
      coinflip = random(1);
      if (coinflip == 0) {
        gameState = KM; 
      } else {
         gameState = KC;
      }
  }
}

  FOREACH_FACE(f) { //If action is performed on the board, change to next player
    if ( !isValueReceivedOnFaceExpired( f ) ) {
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == CLEARM) {
          Hear = NH;
          gameState = KWC;
      }
      if (neighborGameState == CLEARC){
          Smell = NS; 
          gameState = KWM;
      }
      if (gameState == KE && neighborGameState == SLAVE){ //Let you change the key if is not activated yet
          gameState = SLAVE;
          Hear = NH;
          Smell = NS; 
      }
    }
  }
  if (isAlone()){ //Wait on blue until KEY is removed from Cornfield
        if (gameState == KWC){
          gameState = KC;
        }
        if (gameState == KWM){
          gameState = KM;
        } 
  }
}

void monsterLoop() {

FOREACH_FACE(f) { //If action is performed on the board, change to next player
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARM) {
                Hear = NH;
                gameState = CLEARM;
                roundTimer.set(ROUND_TIME);
              }
              if (neighborGameState == DETECTED) {
                gameState = CLEARM;
                roundTimer.set(ROUND_TIME);
              }
              if (neighborGameState == UNDETECTED) {
                gameState = CLEARM;
                roundTimer.set(ROUND_TIME);
              }
          }
  }

if (Hear == NOISE && buttonSingleClicked()){ //If you are in the tile and click it, detection attemp
       FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborSmell = getSmell(getLastValueReceivedOnFace(f));
              if (neighborSmell == STINK){
                  gameState = DETECTED;
                  roundTimer.set(ROUND_TIME);
              }
          }
        }
        if (gameState != DETECTED){
                gameState = UNDETECTED;
                roundTimer.set(ROUND_TIME);
        }
}
if (Hear == NH && buttonSingleClicked()) { //If you are in a neighbor tile and click it, perform move
      FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborHear == NOISE){
                  Hear = NOISE;
                  gameState = CLEARM;
                  roundTimer.set(ROUND_TIME);
                   
              }
            }
        }
    }

//Extra feature, change a tile
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == SWAP){
                  gameState = CLEARM;
                  roundTimer.set(ROUND_TIME);
                   
              }
            }
        }
        if (isAlone() && Hear == NH){ //Prevent cheating avoiding to move tile if you are there
                CWIN--;
                gameState = SWAP;
        }
}

void citizenLoop() {

FOREACH_FACE(f) { //If action is performed on the board, change to next player
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARC) {
                Smell = NS;
                gameState = CLEARC;
                roundTimer.set(ROUND_TIME);
              }
              if (neighborGameState == DETECTED) {
                gameState = CLEARC;
                roundTimer.set(ROUND_TIME);
              }
              if (neighborGameState == UNDETECTED) {
                gameState = CLEARC;
                roundTimer.set(ROUND_TIME);
              }
          }
  }

if (Smell == STINK && buttonSingleClicked()){ //If you are in the tile and click it, detection attemp
       FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborHear == NOISE){
                  gameState = DETECTED;
                  roundTimer.set(ROUND_TIME);
              }
          }
        }
        if (gameState != DETECTED){
                gameState = UNDETECTED;
                roundTimer.set(ROUND_TIME);
        }
}

if (Smell == NS && buttonSingleClicked()) { //If you are in a neighbor tile and click it, perform move
  FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborHear == STINK){
                  Smell = STINK;
                  gameState = CLEARC;
                  roundTimer.set(ROUND_TIME);
                }   
             }
         }
     }

//Extra feature, change a tile
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == SWAP){
                  gameState = CLEARC;
                  roundTimer.set(ROUND_TIME);
              }
          }
        }
        if (isAlone() && Smell == NS){ //Prevent cheating avoiding to move tile if you are there
                CWIN--;
                gameState = SWAP;
      
        }
}

void swapLoop() { //Extra feature, change a tile
  numNeighbors = 0;
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            numNeighbors++;
              if (numNeighbors >= 2){ //Prevent cheating
                  gameState = SLAVE;
              }
          }
        }
}

void resetLoop() { //Timer to reset the game
  if (roundTimer.isExpired()){
        gameState = SLAVE;  
        roundTimer.set(ROUND_TIME);
        Hear = NH;
        Smell = NS;  
  }
}

void clearLoop() { //Wait to timer and clean the board for next player

  if (CWIN <= 0 && Hear == NH && Smell == STINK){ //Detect WIN condition
    gameState = WIN;
  }
  if (Smell == STINK && Hear == NOISE){ //Detect LOSE condition
    gameState = LOSE;
  }

  FOREACH_FACE(f) { // Detect WIN or LOSE condition on the board
          if ( !isValueReceivedOnFaceExpired( f ) ) {
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

void detectedLoop() { // Detecting state timer
  if (roundTimer.isExpired()){
        CWIN--;
        gameState = SLAVE;
  }
}

void endingLoop() { //WIN and LOSE loop

}


/////////////////
//   GRAPHICS  //
/////////////////

void slaveDisplayLoop() { //Fade random faces on Cornfield
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


void keyDisplayLoop() { //Show KEY status
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
void monsterDisplayLoop() { //Fade MONSTER position while all board is OFF

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
   
   numDetected = 0; //Turn YELOW if detection worked
      if (Hear == NH) {
          FOREACH_FACE(f) {
              if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
                byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
                  if (neighborGameState == DETECTED){
                    numDetected++;
              }
          }
      }
      if (numDetected == 1){ //If detected action is performed and achieved, turn YELLOW
        setColor(YELLOW);  
      }
      if (numDetected == 0){
        setColor(OFF);  
      }
   }
}

void citizenDisplayLoop() {//Fade CITIZEN position while all board is OFF
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
   
   numDetected = 0; //Turn YELOW if detection worked
      if (Smell == NS) {
          FOREACH_FACE(f) {
              if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
                byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
                  if (neighborGameState == DETECTED){
                    numDetected++;
              }
          }
      }
      if (numDetected == 1){//If detected action is performed and achieved, turn YELLOW
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
int x = 0;
Wheel.set(LOOP);
  while (x < 6){
    setColorOnFace(YELLOW, x); 
    if (Wheel.isExpired()) {
      setColor(OFF);  
      Wheel.set(LOOP);
      x++;
    }                        
  }
}


void winDisplayLoop() {//Fade random faces on GREEN

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

void loseDisplayLoop() {//Fade random faces on RED
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

/////////////////
//COMMUNICATION//
/////////////////

byte getGameState(byte data) { //BYTE CDEF
  return (data & 15);
}

byte getHear(byte data) { //BYTE B
  return ((data >> 4) & 1);
}

byte getSmell(byte data) { //BYTE A
  return ((data >> 5) & 1);
}
