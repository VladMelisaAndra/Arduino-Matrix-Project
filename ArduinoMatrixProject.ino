#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "LedControl.h"

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

const int lcdNumberOfColumns = 16;
const int lcdNumberOfRows = 3;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 13;
const byte d7 = 4;

const byte lcdContrastPin = 5;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte lcdContrast;

const int joyXPin = A0;
const int joyYPin = A1;
const int joySWPin = 2;

const int joyLeftThreshold = 250;
const int joyRightThreshold = 750;

bool joyIsNeutral = true;
bool joySWState = HIGH;
bool joyPrevSWState = HIGH;

unsigned long lastjoySWPress = 0;
const int joySwPressThreshold = 800;

const int startGameMenuOption = 0;
const int highscoreMenuOption = 1;
const int settingsMenuOption = 2;
const int aboutMenuOption = 3;
const int howToPlayMenuOption = 4;

const int gameMenuIsLoading = -1;
const int gameMenuOptionOverflow = 5;
int currentMenuOption = gameMenuIsLoading;
bool inGame = false;

const int enterStartingDifficultySetting = 0;
const int enterLcdBrightnessSetting = 1;
const int enterMatrixBrightnessSetting = 2;
const int soundSetting = 3;
const int exitSetting = 4;
int currentSettingsOption = 0;

unsigned long gameOpenedTimer;
unsigned long gameStartedTimer;
const int welcomeMessageInterval = 3000;

int highscore = 0;

bool enteredTheOptionFeatures = false;

int player[2] = { 7, 7 };

unsigned long lastMatrixMove = 0;
const int matrixMoveInterval = 200;

const int initialWallLengh = 2;
int wallLength = initialWallLengh;
unsigned long lastWallLengthIncrese = 0;
const int wallLengthIncreseInterval = 4000;
bool generateWall = true;
int startWallPos, endWallPos;
int wallRow = 0;
unsigned long lastWallMove = 0;
int wallMoveInterval = 600;
const int easyDifficultyWallMoveInterval = 600;
const int mediumDifficultyWallMoveInterval = 500;
const int hardDifficultyWallMoveInterval = 300;

int score = 0;
int level = 1;
int difficulty = 1;
int lcdBrightness = 100;
int matrixBrightness = 10;
bool sound = 1;

const int up = 0;
const int right = 1;
const int down = 2;
const int left = 3;

const int lcdBrightnessAddress = 0;
const int matrixBrightnessAddress = 1;
const int soundAddress = 2;
const int highscoreAddress = 3;
const int highscorePlayerNameAddress = 4;

const int buzzPin = 3;

int nameIndex = 0;
const int playerNameLength = 8;
char playerName[playerNameLength] = "        ";
bool enterName = false;

long long lastScrollMove = 0;
const int scrollMovingTime = 500;

const int menuScrollingSoundFrequency = 200;
const int menuScrollingSoundDuration = 100;
const int levelUpSoundFrequency = 600;
const int levelUpSoundDuration = 300;
const int playerHitsWallSoundFrequency = 100;
const int playerHitsWallSoundDuration = 350;

byte upAndDownEmoji[] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100
};
const int upAndDownEmojiPosition = 0;

const int easyDifficulty = 1;
const int mediumDifficulty = 2;
const int hardDifficulty = 3;
const int brightnessSettingStep = 7;
const int maxBrightness = 15;
const int minBrightness = 1;
const int soundOn = 1;
const int soundOff = 0;

void setup() {
  lc.shutdown(0, false);
  lc.clearDisplay(0);

  pinMode(lcdContrastPin, OUTPUT);

  lcd.begin(lcdNumberOfColumns, lcdNumberOfRows);
  lcd.print("Welcome!");

  analogWrite(lcdContrastPin, lcdContrast);
  pinMode(joySWPin, INPUT_PULLUP);

  if (EEPROM.read(lcdBrightnessAddress) != 255)
    lcdBrightness = EEPROM.read(lcdBrightnessAddress);

  if (EEPROM.read(matrixBrightnessAddress) != 255)
    matrixBrightness = EEPROM.read(matrixBrightnessAddress);

  if (EEPROM.read(sound) != 255)
    if (EEPROM.read(sound) == soundOff)
      sound = false;

  if (EEPROM.read(highscoreAddress) != 255)
    highscore = EEPROM.read(highscoreAddress);

  for (int i = 0; i < playerNameLength; i++)
    playerName[i] = EEPROM.read(highscorePlayerNameAddress + i);

  lc.setIntensity(0, matrixBrightness);
  lcd.createChar(upAndDownEmojiPosition, upAndDownEmoji);

  gameOpenedTimer = millis();
}

void loop() {
  if (currentMenuOption == gameMenuIsLoading && millis() - gameOpenedTimer > welcomeMessageInterval) {
    currentMenuOption = startGameMenuOption;
    printMenuOptions();
  }

  if (inGame == true) {
    moveWall();
    printMatrix();
    displayScoreAndLevel();
  }

  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);

  checkJoyIsNeutral(joyX, joyY);

  if (moveDown(joyX)) {
    if (enteredTheOptionFeatures == false) {
      currentMenuOption--;
      if (currentMenuOption <= gameMenuIsLoading)
        currentMenuOption = howToPlayMenuOption;
      printMenuOptions();
      joyIsNeutral = false;
    } else if (enterName == true) {
      if (playerName[nameIndex] == ' ')
        playerName[nameIndex] = 'z';
      else if (playerName[nameIndex] == 'a')
        playerName[nameIndex] = ' ';
      else
        playerName[nameIndex] = playerName[nameIndex] - 1;
      showPlayerNameScreen();
      joyIsNeutral = false;
    } else if (inGame == true) {
      if (millis() - lastMatrixMove > matrixMoveInterval) {
        player[0] = player[0] + 1;
        if (player[0] >= matrixSize)
          player[0] = 0;
        lastMatrixMove = millis();
      }
    } else if (enteredTheOptionFeatures == true && currentMenuOption == settingsMenuOption) {
      modifySetting(down);
      joyIsNeutral = false;
    }
  }

  if (moveUp(joyX)) {
    if (enteredTheOptionFeatures == false) {
      currentMenuOption++;
      if (currentMenuOption >= gameMenuOptionOverflow)
        currentMenuOption = startGameMenuOption;
      printMenuOptions();
      joyIsNeutral = false;
    } else if (enterName == true) {
      if (playerName[nameIndex] == ' ')
        playerName[nameIndex] = 'a';
      else if (playerName[nameIndex] == 'z')
        playerName[nameIndex] = ' ';
      else
        playerName[nameIndex] = playerName[nameIndex] + 1;
      showPlayerNameScreen();
      joyIsNeutral = false;
    } else if (inGame == true) {
      if (millis() - lastMatrixMove > matrixMoveInterval) {
        player[0] = player[0] - 1;
        if (player[0] < 0)
          player[0] = matrixSize - 1;
        lastMatrixMove = millis();
      }
    } else if (enteredTheOptionFeatures == true && currentMenuOption == settingsMenuOption) {
      modifySetting(up);
      joyIsNeutral = false;
    }
  }

  if (moveRight(joyY)) {
    if (enterName == true) {
      nameIndex = nameIndex + 1;
      if (nameIndex >= playerNameLength)
        nameIndex = playerNameLength - 1;
      joyIsNeutral = false;
    } else if (inGame == true) {
      if (millis() - lastMatrixMove > matrixMoveInterval) {
        player[1] = player[1] + 1;
        if (player[1] >= matrixSize)
          player[1] = 0;
        lastMatrixMove = millis();
      }
    } else if (enteredTheOptionFeatures == true && currentMenuOption == settingsMenuOption) {
      modifySetting(right);
      joyIsNeutral = false;
    } else if (enteredTheOptionFeatures == true) {
      enteredTheOptionFeatures = false;
      printMenuOptions();
    }
  }

  if (moveLeft(joyY)) {
    if (enterName == true) {
      nameIndex = nameIndex - 1;
      if (nameIndex < 0)
        nameIndex = 0;
      joyIsNeutral = false;
    } else if (inGame == true) {
      if (millis() - lastMatrixMove > matrixMoveInterval) {
        player[1] = player[1] - 1;
        if (player[1] < 0)
          player[1] = matrixSize - 1;
        lastMatrixMove = millis();
      }
    } else if (enteredTheOptionFeatures == true && currentMenuOption == settingsMenuOption) {
      modifySetting(left);
      joyIsNeutral = false;
    } else if (enteredTheOptionFeatures == true) {
      enteredTheOptionFeatures = false;
      printMenuOptions();
    }
  }

  checkIfPlayerHitsTheWall();

  joySWState = digitalRead(joySWPin);
  if (joySWState != joyPrevSWState) {
    if (joySWState == LOW) {
      if (enterName == true) {
        for (int i = 0; i < playerNameLength; i++)
          EEPROM.update(highscorePlayerNameAddress + i, playerName[i]);
        enterName = false;
        enteredTheOptionFeatures = false;
        printMenuOptions();
      } else if (enteredTheOptionFeatures == true && currentMenuOption == settingsMenuOption && currentSettingsOption == exitSetting) {
        enteredTheOptionFeatures = false;
        printMenuOptions();
      } else
        enterTheOptionFeatures();
    }
    joyPrevSWState = joySWState;
  }

  if (enteredTheOptionFeatures == true && currentMenuOption == aboutMenuOption) {
    if (millis() - lastScrollMove >= scrollMovingTime) {
      lcd.scrollDisplayLeft();
      lastScrollMove = millis();
    }
  }
}

void printMenuOptions() {
  lcd.clear();
  if (sound == true)
    tone(buzzPin, menuScrollingSoundFrequency, menuScrollingSoundDuration);
  lcd.write((byte)upAndDownEmojiPosition);
  if (currentMenuOption == startGameMenuOption)
    lcd.print(" Start game");
  else if (currentMenuOption == highscoreMenuOption)
    lcd.print(" Highscore");
  else if (currentMenuOption == settingsMenuOption)
    lcd.print(" Settings");
  else if (currentMenuOption == aboutMenuOption)
    lcd.print(" About");
  else if (currentMenuOption == howToPlayMenuOption)
    lcd.print(" How to play");
}

void enterTheOptionFeatures() {
  lcd.clear();
  enteredTheOptionFeatures = true;
  if (currentMenuOption == startGameMenuOption) {
    gameStartedTimer = millis();
    lcd.print("Game started!");
    inGame = true;
    if (difficulty == easyDifficulty)
      wallMoveInterval = easyDifficultyWallMoveInterval;
    else if (difficulty == mediumDifficulty)
      wallMoveInterval = mediumDifficultyWallMoveInterval;
    else
      wallMoveInterval = hardDifficultyWallMoveInterval;
  } else if (currentMenuOption == highscoreMenuOption) {
    lcd.print("<> Highscore:");
    goToNextLcdLine();
    lcd.print(highscore);
    lcd.print(" by ");
    for (int i = 0; i < playerNameLength; i++)
      lcd.print(playerName[i]);
  } else if (currentMenuOption == settingsMenuOption) {
    settingsOptions();
  } else if (currentMenuOption == aboutMenuOption) {
    lcd.print("<> FunGame by Vlad Melisa");
    goToNextLcdLine();
    lcd.print("Github user: Vlad Melisa-Andra");
  } else if (currentMenuOption == howToPlayMenuOption) {
    lcd.print("<> Move in order");
    goToNextLcdLine();
    lcd.print("to avoid walls.");
  }
}

void goToNextLcdLine() {
  lcd.setCursor(0, 1);
}

void moveWall() {
  if (millis() - lastWallMove > wallMoveInterval) {
    wallRow = wallRow + 1;
    if (wallRow >= matrixSize) {
      generateWall = true;
      wallRow = 0;

      if (millis() - lastWallLengthIncrese > wallLengthIncreseInterval) {
        wallLength = wallLength + 1;
        if (wallLength >= matrixSize) {
          if (sound == true)
            tone(buzzPin, levelUpSoundFrequency, levelUpSoundDuration);
          level = level + 1;
          wallMoveInterval = wallMoveInterval - wallMoveInterval / 3;
          wallLength = initialWallLengh;
        }
        lastWallLengthIncrese = millis();
      }
    }

    lastWallMove = millis();
  }
  int randomNumber;
  if (generateWall == true) {
    randomNumber = random(0, matrixSize - wallLength);
    if (randomNumber % 2 == 0) {
      startWallPos = randomNumber;
      endWallPos = randomNumber + wallLength - 1;
    } else {
      startWallPos = randomNumber + 1;
      endWallPos = randomNumber + wallLength;
    }
    generateWall = false;
  }
}

void printMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (row == player[0] && col == player[1])
        lc.setLed(0, row, col, true);
      else if (row == wallRow && col >= startWallPos && col <= endWallPos)
        lc.setLed(0, row, col, true);
      else
        lc.setLed(0, row, col, false);
    }
  }
}

void changeMatrixLight(bool lightUp) {
  for (int row = 0; row < matrixSize; row++)
    for (int col = 0; col < matrixSize; col++)
      lc.setLed(0, row, col, lightUp);
}

void displayScoreAndLevel() {
  lcd.clear();
  lcd.print("Level:");
  lcd.print(level);
  goToNextLcdLine();
  lcd.print("Score:");
  score = (millis() - gameStartedTimer) / 1000 * level;
  lcd.print(score);
}

void checkJoyIsNeutral(int joyX, int joyY) {
  if (joyX >= joyLeftThreshold && joyX <= joyRightThreshold && joyY >= joyLeftThreshold && joyY <= joyRightThreshold)
    joyIsNeutral = true;
}

bool moveDown(int joyX) {
  if (joyX > joyRightThreshold && joyIsNeutral)
    return true;
  else
    return false;
}

bool moveUp(int joyX) {
  if (joyX < joyLeftThreshold && joyIsNeutral)
    return true;
  else
    return false;
}

bool moveRight(int joyY) {
  if (joyY < joyLeftThreshold && joyIsNeutral)
    return true;
  else
    return false;
}

bool moveLeft(int joyY) {
  if (joyY > joyRightThreshold && joyIsNeutral)
    return true;
  else
    return false;
}

void checkIfPlayerHitsTheWall() {
  if (inGame == true && player[0] == wallRow && player[1] >= startWallPos && player[1] <= endWallPos) {
    if (sound == true)
      tone(buzzPin, playerHitsWallSoundFrequency, playerHitsWallSoundDuration);
    inGame = false;
    score = (millis() - gameStartedTimer) / 1000 * level;
    wallRow = 0;
    for (int row = 0; row < matrixSize; row++)
      for (int col = 0; col < matrixSize; col++)
        lc.setLed(0, row, col, false);

    lcd.clear();
    lcd.print("Congrats on");
    goToNextLcdLine();
    lcd.print("reaching lvl ");
    lcd.print(level);

    delay(2000);

    lcd.clear();
    lcd.print("Your score is");
    goToNextLcdLine();
    lcd.print(score);

    highscore = 0;
    if (score > highscore) {
      highscore = score;
      EEPROM.write(highscoreAddress, highscore);
      delay(2000);
      lcd.clear();
      lcd.print("Congrats! You");
      goToNextLcdLine();
      lcd.print("beated highscore!");
      delay(2000);
      showPlayerNameScreen();
      enterName = true;
    } else {
      delay(2000);
      lcd.clear();
      lcd.print("<> to go to Menu");
    }

    level = 1;
    score = 0;
  }
}

void showPlayerNameScreen() {
  lcd.clear();
  lcd.print("Enter name:");
  goToNextLcdLine();
  for (int i = 0; i < playerNameLength; i++)
    lcd.print(playerName[i]);
}

void settingsOptions() {
  lcd.clear();
  if (sound == true)
    tone(buzzPin, menuScrollingSoundFrequency, menuScrollingSoundDuration);
  if (currentSettingsOption == enterStartingDifficultySetting) {
    lcd.write((byte)upAndDownEmojiPosition);
    lcd.print(" <> Set");
    goToNextLcdLine();
    lcd.print("difficulty: ");
    lcd.print(difficulty);
  } else if (currentSettingsOption == enterLcdBrightnessSetting) {
    lcd.write((byte)upAndDownEmojiPosition);
    lcd.print(" <> LCD");
    goToNextLcdLine();
    lcd.print("brightness: ");
    lcd.print(lcdBrightness);
    changeMatrixLight(false);
  } else if (currentSettingsOption == enterMatrixBrightnessSetting) {
    lcd.write((byte)upAndDownEmojiPosition);
    lcd.print(" <> Matrix:");
    goToNextLcdLine();
    lcd.print("brightness: ");
    lcd.print(matrixBrightness);
    changeMatrixLight(true);
  } else if (currentSettingsOption == soundSetting) {
    lcd.write((byte)upAndDownEmojiPosition);
    lcd.print(" <> Sounds:");
    goToNextLcdLine();
    if (sound == true)
      lcd.print("on");
    else
      lcd.print("off");
    changeMatrixLight(false);
  } else if (currentSettingsOption == exitSetting) {
    lcd.write((byte)upAndDownEmojiPosition);
    lcd.print(" Click to exit");
    goToNextLcdLine();
    lcd.print("the Settings");
  }
}

void modifySetting(int joyMoved) {
  lcd.clear();
  if (joyMoved == up) {
    currentSettingsOption = currentSettingsOption + 1;
    if (currentSettingsOption > exitSetting)
      currentSettingsOption = enterStartingDifficultySetting;
  } else if (joyMoved == down) {
    currentSettingsOption = currentSettingsOption - 1;
    if (currentSettingsOption < enterStartingDifficultySetting)
      currentSettingsOption = exitSetting;
  }

  if (currentSettingsOption == enterStartingDifficultySetting) {
    if (joyMoved == right) {
      difficulty = difficulty + 1;
      if (difficulty > hardDifficulty)
        difficulty = hardDifficulty;
    } else if (joyMoved == left) {
      difficulty = difficulty - 1;
      if (difficulty < easyDifficulty)
        difficulty = easyDifficulty;
    }
  } else if (currentSettingsOption == enterLcdBrightnessSetting) {
    if (joyMoved == right) {
      lcdBrightness = lcdBrightness + brightnessSettingStep;
    } else if (joyMoved == left) {
      lcdBrightness = lcdBrightness - brightnessSettingStep;
    }
    EEPROM.write(lcdBrightnessAddress, lcdBrightness);
  } else if (currentSettingsOption == enterMatrixBrightnessSetting) {
    if (joyMoved == right) {
      matrixBrightness = matrixBrightness + brightnessSettingStep;
      if (matrixBrightness > maxBrightness)
        matrixBrightness = maxBrightness;
    } else if (joyMoved == left) {
      matrixBrightness = matrixBrightness - brightnessSettingStep;
      if (matrixBrightness < minBrightness)
        matrixBrightness = minBrightness;
    }
    EEPROM.write(matrixBrightnessAddress, matrixBrightness);
    lc.setIntensity(0, matrixBrightness);
  } else if (currentSettingsOption == soundSetting) {
    if (joyMoved == right || joyMoved == left)
      sound = !sound;
    if (sound == true)
      EEPROM.write(soundAddress, soundOn);
    else
      EEPROM.write(soundAddress, soundOff);
  }

  settingsOptions();
}