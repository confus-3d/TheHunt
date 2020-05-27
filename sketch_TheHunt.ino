/*
    The Hunt
    by Move38, Inc. 2020
    Lead development by FJ Rios
    original game by FJ Rios

    Rules: 

    --------------------
    Blinks by Move38
    Brought to life via Kickstarter 2018

    @madewithblinks
    www.move38.com
    --------------------
*/


enum gameStates {SLAVE, MONSTER, CITIZEN, RESET, MONSTERSHOW, CITIZENSHOW, CLEAR, DELETE, WIN, LOSE};  //tile states
enum Nearby {NO, SMELL, HEAR};
byte gameState = SLAVE;
byte Nearby = NO;
int Stink = 0;
int Noise = 0;
Timer nearbyTimer;
#define STEP_SIZE 10  
#define STEP_TIME_MS 30
int brightness = 1; 
int step = STEP_SIZE;
Timer HeartBeat;
int numNeighbors = 0;
Timer roundTimer;
int MSHOW = 0;
int CSHOW = 0;
int CWIN = 0;


void setup() {
  // put your setup code here, to run once:

}
void loop() {   // main loop
  switch (gameState) {
    case SLAVE:
      slaveLoop();
      slaveDisplayLoop();
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
    case MONSTERSHOW:
      mshowLoop();
      mshowDisplayLoop();
      break;
    case CITIZENSHOW:
      cshowLoop();
      cshowDisplayLoop();
      break;
    case CLEAR:
      clearLoop();
      resetDisplayLoop();
      break;
    case WIN:
      winLoop();
      citizenDisplayLoop();
      break;
    case LOSE:
      winLoop();
      monsterDisplayLoop();
      break;
  }

  if (buttonLongPressed()) {
    gameState = RESET;
    roundTimer.set(2000);
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
    case CLEAR:
      sendData = (gameState << 4);
      break;
    case MONSTERSHOW:
      sendData = (gameState << 4) + (Nearby << 2);
      break;
    case CITIZENSHOW:
      sendData = (gameState << 4) + (Nearby << 2);
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
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
      }
      if (neighborGameState == MONSTERSHOW) {
        gameState = MONSTERSHOW;
      }
      if (neighborGameState == CITIZENSHOW) {
        gameState = CITIZENSHOW;
      }
    }
  }

  if (buttonDoubleClicked()){
    if (numNeighbors == 0){
      gameState = MONSTER;  
    }
  }
}

void monsterLoop() {

  numNeighbors = 0;
  
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      numNeighbors++;
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
    }
    }
  }
  
    if (numNeighbors == 0){
      setValueSentOnAllFaces(MONSTERSHOW);
    }

    if (buttonSingleClicked()){
      setValueSentOnAllFaces(CLEAR);
    }

    
    if (buttonDoubleClicked()){
      if (numNeighbors == 0){
        gameState = CITIZEN;  
        
      }
    }
}

void citizenLoop() {

  numNeighbors = 0;
  
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      numNeighbors++;
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
    }
    }
  }
  
    if (numNeighbors == 0){
      setValueSentOnAllFaces(CITIZENSHOW);
    }

    if (buttonSingleClicked()) {
      setValueSentOnAllFaces(CLEAR);
    }

    
    if (buttonDoubleClicked()){
      if (numNeighbors == 0){
        gameState = MONSTER;  
        
      }
    }
}

void resetLoop() {
  if (roundTimer.isExpired()){
        gameState = SLAVE;  
        Stink = 0;
        Noise = 0;  
  }
}

void clearLoop() {
  if (roundTimer.isExpired()){
        gameState = SLAVE;
        Stink = 0;
        Noise = 0;  
  }
}

void mshowLoop() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == DELETE) {
        MSHOW = 0;
      }
      if (neighborGameState == MONSTERSHOW) {
        setValueSentOnAllFaces(MONSTERSHOW);
      }
      if (neighborGameState == CLEAR) {
      gameState = CLEAR;
      roundTimer.set(2000);
      }
      if (neighborGameState == RESET) {
      gameState = RESET;
      roundTimer.set(2000);
      }
      if (neighborGameState == LOSE) {
      gameState = LOSE;
      }
    }
  }

      if (buttonSingleClicked() && MSHOW == 0){
          MSHOW = 1;
          setValueSentOnAllFaces(DELETE);
      }
      if (buttonSingleClicked() && MSHOW == 1){ //SMELL
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              byte neighborNearby = getNearby(getLastValueReceivedOnFace(f));
              if (neighborNearby == SMELL) {
                Stink++;
              }
            }
          }  
      }
      
      if (MSHOW == 1 && CSHOW == 1){
        gameState = LOSE;
      }
}

void cshowLoop() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == DELETE) {
        CSHOW = 0;
      }
      if (neighborGameState == CITIZENSHOW) {
        setValueSentOnAllFaces(CITIZENSHOW);
      }
      if (neighborGameState == CLEAR) {
      gameState = CLEAR;
      roundTimer.set(2000);
      }
      if (neighborGameState == RESET) {
      gameState = RESET;
      roundTimer.set(2000);
    }
      if (neighborGameState == WIN) {
      gameState = WIN;
      }
    }
  }

          if (buttonSingleClicked() && CSHOW == 0){
          CSHOW = 1;
          CWIN++;
          setValueSentOnAllFaces(DELETE);
          }
          if (CWIN == 25){
            gameState = WIN;
          }
          if (MSHOW == 1 && CSHOW == 1){
          gameState = LOSE;
          }
                if (buttonSingleClicked() && CSHOW == 1){ //HEAR
          FOREACH_FACE(f) {
            if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
              byte neighborNearby = getNearby(getLastValueReceivedOnFace(f));
              if (neighborNearby == HEAR) {
                Noise++;
              }
            }
          }  
      }
}


void winLoop() {
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
    }
    }
  }
}

void loseLoop() {
  FOREACH_FACE(f) {
    if ( !isValueReceivedOnFaceExpired( f ) ) { // Have we seen a neighbor
      byte neighborGameState = getGameState(getLastValueReceivedOnFace(f));
      if (neighborGameState == RESET) {
        gameState = RESET;
        roundTimer.set(2000);
    }
    }
  }
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

void monsterDisplayLoop() {
  setColor(RED);
}

void citizenDisplayLoop() {
  setColor(GREEN);                         
}

void resetDisplayLoop() {
  setColor(OFF);                         
}

void mshowDisplayLoop() {
  if (MSHOW == 1){
     if (Stink >= 0){
            setColor(YELLOW); 
     }else if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( RED ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      } 
  } else{
      setColor(OFF); 
  }
  
}

void cshowDisplayLoop() {
  if (CSHOW == 1){
         if (Noise >= 0){
            setColor(YELLOW); 
     }else if (HeartBeat.isExpired()) { 
        if ( (brightness + step > MAX_BRIGHTNESS ) || (brightness + step < 0 ) ) {
          step = -step;
        }
    
      brightness += step; 
      setColor( dim( GREEN ,  brightness  ) );
      HeartBeat.set( STEP_TIME_MS );
      }                         
  } else{
      setColor(OFF); 
  }
}

//COMMUNICATION

byte getGameState(byte data) {
  return (data >> 4);
}

byte getNearby(byte data) {
  return (data >> 2);
}
