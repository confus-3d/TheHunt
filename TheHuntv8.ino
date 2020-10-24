/*
    The Hunt 
    Lead development by FJ Rios
    original game by FJ Rios
    12+ Blinks          
    2 Players (MONSTER=RED and CITIZEN=GREEN)
    Rules:
    1st- Leave a blink alone, this will be KEY blink. Double click to chose first player.
    2nd- Player attach KEY to the Blinks Board, and press a Blink adjacent to te KEY to move to starting position.
    3rd- Put KEY alone to change player and do the same for second player 
    GAME START!
    MONSTER Player (connect KEY to board and Pick one action):
    -Move to an adjacent Blink to catch the CITIZEN or
    -Press your actual position to SMELL in the surroundings and check if CITIZEN is adjacent to your tile.
    If you SMELL the CITIZEN 3 adjacent tiles turn YELLOW, the citizen is on one of them.
    If you catch the CITIZEN, you win and the board turns RED.
    CITIZEN Player (connect KEY to board Pick one action):
    -Move to an adjacent Blink to avoid the MONSTER or
    -Press your actual position to HEAR in the surroundings and check if MONSTER is adjacent to your tile.
    If you HEAR the MONSTER all adjacent tiles turn YELLOW
    If you avoid the MONSTER for several rounds, you win and the board turns GREEN.
    ADVANCED MODE: (18 Blinks Recomended)
    You can, instead using your action, take a tile from the board and put it into another place.
    This count as your action. The new position must touch 2 blinks to be valid and will flash yellow
    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018
    @madewithblinks
    www.move38.com
    --------------------
*/


enum gameState {SLAVE, MONSTER, CITIZEN, RESET, CLEARM, CLEARC, DETECTED, UNDETECTED, WIN, LOSE, KM, KC, KE, KWM, KWC, SWAP};  //16 tile states sended in BYTE CDEF
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
byte wheelFace = 0;

int numNeighbors;
int numDetected;
byte faceDetected;

#define ROUND_TIME 3000 //Time showing board after any action
Timer roundTimer;

#define MAX_ROUNDS 40 //Rounds to CITIZEN win. Each player count as 1 round. (25 CITIZEN rounds to win)
int CWIN = MAX_ROUNDS; 

void setup() {  //Setup code
  randomize(); //Get random string
}

void loop() {   //Main Loop
  setColor(OFF); //Restart graphics

  byte sendData; //Tile comunication
  switch (gameState) {
    case RESET: case SLAVE: 
    case KE: case KWM: case KWC: case WIN: case LOSE:
      sendData = (gameState);
      break;
    case KM: case KC: case MONSTER: case CITIZEN: case CLEARM: case CLEARC:
      sendData = (gameState) + (Hear << 4) + (Smell << 5);
      break;
  }
  if (gameState != DETECTED){setValueSentOnAllFaces(sendData);}
  
  switch (gameState) { //Detects gamestate
    case SLAVE: slaveLoop(); slaveDisplayLoop(); break;
    case KM: case KC: case KE: case KWM: case KWC: keyLoop(); keyDisplayLoop(); break;
    case MONSTER: monsterLoop(); monsterDisplayLoop(); break;
    case CITIZEN: citizenLoop(); citizenDisplayLoop(); break;
    case RESET: resetLoop(); break;
    case CLEARM: clearLoop(); monsterDisplayLoop(); break;
    case CLEARC: clearLoop(); citizenDisplayLoop(); break;
    case DETECTED: detectedLoop(); detectedDisplayLoop(); break;
    case UNDETECTED: detectedLoop(); undetectedDisplayLoop(); break;
    case WIN: winDisplayLoop(); winLoop(); break;
    case LOSE: loseDisplayLoop(); winLoop(); break;
    case SWAP: swapLoop(); swapDisplayLoop(); break;
  }

  if (resetPressed == 0) { //RESET GAME Reset all tiles conected
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

}

void slaveLoop() { //Starting gamestate (Cornfield)
  numDetected = 0;
  if (roundTimer.isExpired()){resetPressed = 0;}
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen an neighbor?? Change ALL Cornfield if KEY attached.
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == KM) {gameState = MONSTER;}
      if (neighborGameState == KC) {gameState = CITIZEN;}
      if (neighborGameState == MONSTER) {gameState = MONSTER;}
      if (neighborGameState == CITIZEN) {gameState = CITIZEN;}
      if (neighborGameState == WIN){gameState = WIN;} 
      if (neighborGameState == LOSE){gameState = LOSE;}
    }
  }
  if (isAlone()){gameState = KE;} //Turn a solo Blink into KEY blink
}

void keyLoop() { //KEY change during gameplay and alternate players
  winLoop();
  if (gameState == KE){ //Initial coinflip to determine starting player
    if (buttonDoubleClicked()){
        Hear = NOISE;
        Smell = STINK; 
        coinflip = random(1);
        if (coinflip == 0) {gameState = KM;} else {gameState = KC;}
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
        if (gameState == KWC){gameState = KC;}
        if (gameState == KWM){gameState = KM;} 
  }
}

void monsterLoop() {
  winLoop();
  FOREACH_FACE(f) { //If action is performed on the board, change to next player
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborHear = getHear(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARM) {
                if (neighborHear == NOISE){Hear = NH;}
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
                  faceDetected = f;
                  int randomdetect = random(2);
                  if (randomdetect == 1) faceDetected = (faceDetected+1)%6;
                  if (randomdetect == 2) faceDetected = (faceDetected+5)%6;
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
    
  if (isAlone() && Hear == NH){gameState = SWAP;}//Extra feature, change a tile. Prevent cheating avoiding to move tile if you are there
}

void citizenLoop() {
  winLoop();
  FOREACH_FACE(f) { //If action is performed on the board, change to next player
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
            byte neighborSmell = getSmell(getLastValueReceivedOnFace(f));
              if (neighborGameState == CLEARC) {
                if (neighborSmell == STINK){Smell = NS;}
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
              byte neighborSmell = getSmell(getLastValueReceivedOnFace(f));
                if (neighborSmell == STINK){
                    Smell = STINK;
                    gameState = CLEARC;
                    roundTimer.set(ROUND_TIME);
                }   
            }
    }
  }
  
  if (isAlone() && Smell == NS){gameState = SWAP;} //Extra feature, change a tile. Prevent cheating avoiding to move tile if you are there
}

void swapLoop() { //Extra feature, change a tile
  numNeighbors = 0;
        FOREACH_FACE(f) {
          if ( !isValueReceivedOnFaceExpired( f ) ) { 
            numNeighbors++;
              if (numNeighbors >= 2){ //Prevent cheating
                  gameState = UNDETECTED;
                  roundTimer.set(ROUND_TIME);
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
        CWIN = MAX_ROUNDS; 
  }
}

void clearLoop() { //Wait to timer and clean the board for next player

  if (CWIN <= 0 && Hear == NH && Smell == STINK){gameState = WIN;}//Detect WIN condition
  if (Smell == STINK && Hear == NOISE){gameState = LOSE;} //Detect LOSE condition

  winLoop();

  if (roundTimer.isExpired()){
        CWIN--;
        gameState = SLAVE;
  }
}

void detectedLoop() { // Detecting state timer
if (Hear != NOISE) {
  setValueSentOnAllFaces(gameState);
}
if (Hear == NOISE){
  setValueSentOnFace(gameState, faceDetected);
  setValueSentOnFace(gameState, (faceDetected+1)%6);
  setValueSentOnFace(gameState, (faceDetected+5)%6);
  setValueSentOnFace(UNDETECTED, (faceDetected+2)%6);
  setValueSentOnFace(UNDETECTED, (faceDetected+3)%6);
  setValueSentOnFace(UNDETECTED, (faceDetected+4)%6);
  

}
  if (roundTimer.isExpired()){
        CWIN--;
        gameState = SLAVE;
  }
}

void winLoop(){
  resetPressed = 0;
  FOREACH_FACE(f) { // Detect WIN or LOSE condition on the board
          if ( !isValueReceivedOnFaceExpired( f ) ) {
            byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
              if (neighborGameState == WIN){gameState = WIN;} 
              if (neighborGameState == LOSE){gameState = LOSE;}   
          }
  }
}


/////////////////
//   GRAPHICS  //
/////////////////

void slaveDisplayLoop() { //Fade random faces on Cornfield
      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {step = -step;}
        if (brightness + step < 0 ) {step = -step; randomizer = random(5);}
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
    case KWC:
      setColor(BLUE);
      break;
    case KE:
      setColor(RED);
      setColorOnFace(GREEN, 3);
      setColorOnFace(GREEN, 4);
      setColorOnFace(GREEN, 5);
      break;
  }
}

void monsterDisplayLoop() { //Fade MONSTER position while all board is OFF
    if (Hear == NOISE) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {step = -step;}
        brightness += step; 
        HeartBeat.set( STEP_TIME_MS );
      }
      setColor( dim( RED ,  brightness  ) );
   } 
      if (numDetected == 0) {//Turn YELOW if detection worked
          FOREACH_FACE(f) {
              if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
                byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
                  if (neighborGameState == DETECTED){
                    numDetected = 1;
                    brightness = random (255);
              }
          }
        }
      }
      if (numDetected == 1){ //If detected action is performed and achieved, turn YELLOW
        if (HeartBeat.isExpired()) { 
          if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {step = -step;}
          brightness += step; 
          HeartBeat.set( STEP_TIME_MS );
        }
        setColor( dim( YELLOW ,  brightness  ) );
      }
}

void citizenDisplayLoop() {//Fade CITIZEN position while all board is OFF
    if (Smell == STINK) {
      if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {step = -step;}
        brightness += step; 
        HeartBeat.set( STEP_TIME_MS );
      }
      setColor( dim( GREEN ,  brightness  ) );
   } 
      if (numDetected == 0) {//Turn YELOW if detection worked
          FOREACH_FACE(f) {
              if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
                byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
                  if (neighborGameState == DETECTED){
                    numDetected = 1;
                    brightness = random (255);
              }
          }
        }
      }
      if (numDetected == 1){//If detected action is performed and achieved, turn YELLOW
        if (HeartBeat.isExpired()) { 
          if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {step = -step;}
          brightness += step; 
          HeartBeat.set( STEP_TIME_MS );
        }
        setColor( dim( YELLOW ,  brightness  ) ); 
      }
}

void detectedDisplayLoop() {
    if (Wheel.isExpired()) {
      wheelFace = (wheelFace+1)%6;
      Wheel.set(LOOP);
    }
    setColorOnFace(YELLOW, wheelFace);                        
}

void swapDisplayLoop() {
    if (Wheel.isExpired()) {
      wheelFace = (wheelFace+1)%6;
      Wheel.set(LOOP);
    }
    setColorOnFace(BLUE, wheelFace);                        
}

void undetectedDisplayLoop() {
    if (Wheel.isExpired()) {
      Wheel.set(LOOP);
      wheelFace = (wheelFace+1)%2;
    }  
    if (wheelFace == 1) {setColor(YELLOW);}                             
}


void winDisplayLoop() {//Fade random faces on GREEN
      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {step = -step;}
        if (brightness + step < 0 ) {step = -step; randomizer = random(5);}
        brightness += step;  
        HeartBeat.set( STEP_TIME_MS );
      }
      setColorOnFace( dim( GREEN,  brightness  ), randomizer);              
}

void loseDisplayLoop() {//Fade random faces on RED
      if (HeartBeat.isExpired()) { 
        if (brightness + step > MAX_BRIGHTNESS ) {step = -step;}
        if (brightness + step < 0 ) {step = -step; randomizer = random(5);}
        brightness += step;  
        HeartBeat.set( STEP_TIME_MS );
      }
      setColorOnFace( dim( RED,  brightness  ), randomizer);                       
}

/////////////////
//COMMUNICATION//
/////////////////
byte getGameState(byte data) {return (data & 15);} //BYTE CDEF
byte getHear(byte data) {return ((data >> 4) & 1);} //BYTE B
byte getSmell(byte data) {return ((data >> 5) & 1);} //BYTE A
