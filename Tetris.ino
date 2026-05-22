#include <LovyanGFX.hpp>
#include <Preferences.h>

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796 _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX() {
    auto bcfg = _bus.config();
    bcfg.spi_host = VSPI_HOST;
    bcfg.spi_mode = 0;
    bcfg.freq_write = 10000000;
    bcfg.freq_read = 8000000;
    bcfg.pin_sclk = 18;
    bcfg.pin_mosi = 23;
    bcfg.pin_miso = -1;
    bcfg.pin_dc = 2;
    _bus.config(bcfg);
    _panel.setBus(&_bus);

    auto pcfg = _panel.config();
    pcfg.pin_cs = 5;
    pcfg.pin_rst = 4;
    pcfg.pin_busy = -1;
    pcfg.panel_width = 320;
    pcfg.panel_height = 480;
    pcfg.invert = true;
    pcfg.rgb_order = false;
    _panel.config(pcfg);

    setPanel(&_panel);
  }
};

LGFX tft;
Preferences prefs;

#define HOLD_BTN    19
#define LEFT_BTN    32
#define RIGHT_BTN   33
#define DROP_BTN    26
#define ROTATE_BTN  27
#define START_BTN   14

const int COLS = 10;
const int ROWS = 20;
const int BLOCK = 20;
const int BOARD_X = 20;
const int BOARD_Y = 40;

enum ScreenState { MENU, PLAYING, GAMEOVER };
enum GameMode { CLASSIC, ZEN, LEVEL };

ScreenState screenState = MENU;
GameMode gameMode = CLASSIC;

int board[ROWS][COLS];

int pieceType, pieceRot, pieceX, pieceY;
int oldPieceType, oldPieceRot, oldPieceX, oldPieceY;

int holdType = -1;
int nextType;
bool canHold = true;

int score = 0;
int highScore = 0;
int linesCleared = 0;
int linesThisLevel = 0;
int level = 1;

int normalFallDelay = 600;
int softFallDelay = 70;
int fallDelay = 600;

unsigned long lastFall = 0;

bool lastLeft = HIGH;
bool lastRight = HIGH;
bool lastDrop = HIGH;
bool lastHold = HIGH;
bool lastRotate = HIGH;
bool lastStart = HIGH;

uint16_t colors[8] = {
  TFT_BLACK,
  TFT_CYAN,      // I
  TFT_BLUE,      // J
  TFT_ORANGE,    // L
  TFT_YELLOW,    // O
  TFT_GREEN,     // S
  TFT_PURPLE,    // T
  TFT_RED        // Z
};

const byte shapes[7][4][4] = {
  {
    {0b0000, 0b1111, 0b0000, 0b0000},
    {0b0010, 0b0010, 0b0010, 0b0010},
    {0b0000, 0b1111, 0b0000, 0b0000},
    {0b0010, 0b0010, 0b0010, 0b0010}
  },
  {
    {0b1000, 0b1110, 0b0000, 0b0000},
    {0b0110, 0b0100, 0b0100, 0b0000},
    {0b0000, 0b1110, 0b0010, 0b0000},
    {0b0100, 0b0100, 0b1100, 0b0000}
  },
  {
    {0b0010, 0b1110, 0b0000, 0b0000},
    {0b0100, 0b0100, 0b0110, 0b0000},
    {0b0000, 0b1110, 0b1000, 0b0000},
    {0b1100, 0b0100, 0b0100, 0b0000}
  },
  {
    {0b0110, 0b0110, 0b0000, 0b0000},
    {0b0110, 0b0110, 0b0000, 0b0000},
    {0b0110, 0b0110, 0b0000, 0b0000},
    {0b0110, 0b0110, 0b0000, 0b0000}
  },
  {
    {0b0110, 0b1100, 0b0000, 0b0000},
    {0b0100, 0b0110, 0b0010, 0b0000},
    {0b0110, 0b1100, 0b0000, 0b0000},
    {0b0100, 0b0110, 0b0010, 0b0000}
  },
  {
    {0b0100, 0b1110, 0b0000, 0b0000},
    {0b0100, 0b0110, 0b0100, 0b0000},
    {0b0000, 0b1110, 0b0100, 0b0000},
    {0b0100, 0b1100, 0b0100, 0b0000}
  },
  {
    {0b1100, 0b0110, 0b0000, 0b0000},
    {0b0010, 0b0110, 0b0100, 0b0000},
    {0b1100, 0b0110, 0b0000, 0b0000},
    {0b0010, 0b0110, 0b0100, 0b0000}
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(HOLD_BTN, INPUT_PULLUP);
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(DROP_BTN, INPUT_PULLUP);
  pinMode(ROTATE_BTN, INPUT_PULLUP);
  pinMode(START_BTN, INPUT_PULLUP);

  prefs.begin("tetris", false);
  highScore = prefs.getInt("high", 0);

  randomSeed(analogRead(34));

  tft.init();
  tft.setRotation(0);

  drawMenu();
}

void loop() {
  readButtons();

  if (screenState == PLAYING) {
    fallDelay = digitalRead(DROP_BTN) == LOW ? softFallDelay : normalFallDelay;

    if (millis() - lastFall > fallDelay) {
      saveOldPiece();

      if (!collides(pieceX, pieceY + 1, pieceRot)) {
        pieceY++;
        if (fallDelay == softFallDelay) score++;
        redrawMovingPiece();
        drawSidebar();
      } else {
        lockPiece();
        clearLines();
        spawnPiece(true);
        drawBoardArea();
        drawSidebar();
      }

      lastFall = millis();
    }
  }
}

void readButtons() {
  bool leftNow = digitalRead(LEFT_BTN);
  bool rightNow = digitalRead(RIGHT_BTN);
  bool dropNow = digitalRead(DROP_BTN);
  bool holdNow = digitalRead(HOLD_BTN);
  bool rotateNow = digitalRead(ROTATE_BTN);
  bool startNow = digitalRead(START_BTN);

  if (screenState == MENU) {
    if (leftNow == LOW && lastLeft == HIGH) {
      gameMode = (GameMode)((gameMode + 2) % 3);
      drawMenu();
    }

    if (rightNow == LOW && lastRight == HIGH) {
      gameMode = (GameMode)((gameMode + 1) % 3);
      drawMenu();
    }

    if (startNow == LOW && lastStart == HIGH) {
      resetGame();
    }
  }

  else if (screenState == PLAYING) {
    if (leftNow == LOW && lastLeft == HIGH) {
      saveOldPiece();
      if (!collides(pieceX - 1, pieceY, pieceRot)) pieceX--;
      redrawMovingPiece();
    }

    if (rightNow == LOW && lastRight == HIGH) {
      saveOldPiece();
      if (!collides(pieceX + 1, pieceY, pieceRot)) pieceX++;
      redrawMovingPiece();
    }

    if (rotateNow == LOW && lastRotate == HIGH) {
      saveOldPiece();
      rotatePiece();
      redrawMovingPiece();
    }

    if (holdNow == LOW && lastHold == HIGH) {
      Serial.println("HOLD BUTTON PRESSED");
      saveOldPiece();
      holdPiece();
      drawBoardArea();
      drawSidebar();
    }

    if (startNow == LOW && lastStart == HIGH) {
      screenState = MENU;
      drawMenu();
    }
  }

  else if (screenState == GAMEOVER) {
    if (startNow == LOW && lastStart == HIGH) {
      screenState = MENU;
      drawMenu();
    }
  }

  lastLeft = leftNow;
  lastRight = rightNow;
  lastDrop = dropNow;
  lastHold = holdNow;
  lastRotate = rotateNow;
  lastStart = startNow;
}

void drawMenu() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(4);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(70, 45);
  tft.print("TETRIS");

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(75, 120);
  tft.print("HIGH SCORE");
  tft.setCursor(105, 145);
  tft.print(highScore);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(45, 215);
  tft.print("< ");

  if (gameMode == CLASSIC) tft.print("CLASSIC");
  if (gameMode == ZEN) tft.print("ZEN MODE");
  if (gameMode == LEVEL) tft.print("LEVEL MODE");

  tft.print(" >");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(45, 310);
  tft.print("START = Play");

  tft.setCursor(45, 340);
  tft.print("L/R = Mode");

  tft.setCursor(45, 370);
  tft.print("HOLD = Hold");
}

void resetGame() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) board[r][c] = 0;
  }

  score = 0;
  linesCleared = 0;
  linesThisLevel = 0;
  level = 1;
  normalFallDelay = 600;
  fallDelay = normalFallDelay;
  holdType = -1;
  nextType = random(0, 7);
  canHold = true;

  screenState = PLAYING;

  spawnPiece(true);

  tft.fillScreen(TFT_BLACK);
  drawStaticUI();
  drawBoardArea();
  drawSidebar();
}

void spawnPiece(bool resetHold) {
  pieceType = nextType;
  nextType = random(0, 7);
  pieceRot = 0;
  pieceX = 3;
  pieceY = 0;

  if (resetHold) canHold = true;

  saveOldPiece();

  if (collides(pieceX, pieceY, pieceRot)) {
    if (gameMode == ZEN) {
      clearWholeBoard();
      pieceX = 3;
      pieceY = 0;
    } else {
      endGame();
    }
  }
}

void saveOldPiece() {
  oldPieceType = pieceType;
  oldPieceRot = pieceRot;
  oldPieceX = pieceX;
  oldPieceY = pieceY;
}

void holdPiece() {
  if (!canHold) {
    Serial.println("HOLD BLOCKED: already used this turn");
    return;
  }

  Serial.println("HOLD WORKING");

  int current = pieceType;

  if (holdType == -1) {
    holdType = current;
    pieceType = nextType;
    nextType = random(0, 7);
  } else {
    pieceType = holdType;
    holdType = current;
  }

  pieceRot = 0;
  pieceX = 3;
  pieceY = 0;
  canHold = false;
  saveOldPiece();

  if (collides(pieceX, pieceY, pieceRot)) {
    if (gameMode == ZEN) {
      clearWholeBoard();
    } else {
      endGame();
    }
  }
}

void rotatePiece() {
  int newRot = (pieceRot + 1) % 4;

  if (!collides(pieceX, pieceY, newRot)) {
    pieceRot = newRot;
  } else if (!collides(pieceX - 1, pieceY, newRot)) {
    pieceX--;
    pieceRot = newRot;
  } else if (!collides(pieceX + 1, pieceY, newRot)) {
    pieceX++;
    pieceRot = newRot;
  }
}

bool collides(int newX, int newY, int newRot) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shapes[pieceType][newRot][r] & (0b1000 >> c)) {
        int bx = newX + c;
        int by = newY + r;

        if (bx < 0 || bx >= COLS || by >= ROWS) return true;
        if (by >= 0 && board[by][bx] != 0) return true;
      }
    }
  }
  return false;
}

void lockPiece() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shapes[pieceType][pieceRot][r] & (0b1000 >> c)) {
        int bx = pieceX + c;
        int by = pieceY + r;

        if (by >= 0 && by < ROWS && bx >= 0 && bx < COLS) {
          board[by][bx] = pieceType + 1;
        }
      }
    }
  }
}

void clearLines() {
  int cleared = 0;

  for (int r = ROWS - 1; r >= 0; r--) {
    bool full = true;

    for (int c = 0; c < COLS; c++) {
      if (board[r][c] == 0) {
        full = false;
        break;
      }
    }

    if (full) {
      cleared++;

      for (int row = r; row > 0; row--) {
        for (int col = 0; col < COLS; col++) {
          board[row][col] = board[row - 1][col];
        }
      }

      for (int col = 0; col < COLS; col++) board[0][col] = 0;

      r++;
    }
  }

  if (cleared > 0) {
    linesCleared += cleared;
    linesThisLevel += cleared;
    score += cleared * cleared * 100;

    if (gameMode == LEVEL && linesThisLevel >= 10) {
      linesThisLevel = 0;
      level++;
      normalFallDelay = max(100, 600 - (level - 1) * 60);
    }

    if (gameMode == CLASSIC) {
      level = 1 + linesCleared / 10;
      normalFallDelay = max(100, 600 - (level - 1) * 50);
    }
  }
}

void clearWholeBoard() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) board[r][c] = 0;
  }
  drawBoardArea();
}

void endGame() {
  if (score > highScore) {
    highScore = score;
    prefs.putInt("high", highScore);
  }

  screenState = GAMEOVER;
  drawGameOver();
}

void drawStaticUI() {
  tft.drawRect(BOARD_X - 2, BOARD_Y - 2, COLS * BLOCK + 4, ROWS * BLOCK + 4, TFT_WHITE);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(235, 15);
  tft.print("TETRIS");
}

void drawBoardArea() {
  tft.fillRect(BOARD_X, BOARD_Y, COLS * BLOCK, ROWS * BLOCK, TFT_BLACK);
  drawBoardBlocks();
  drawCurrentPiece();
}

void drawBoardBlocks() {
  for (int r = 0; r < ROWS; r++) {
    for (int c = 0; c < COLS; c++) {
      if (board[r][c] != 0) drawBlock(c, r, colors[board[r][c]]);
    }
  }
}

void redrawMovingPiece() {
  erasePiece(oldPieceType, oldPieceRot, oldPieceX, oldPieceY);
  drawCurrentPiece();
}

void erasePiece(int type, int rot, int px, int py) {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shapes[type][rot][r] & (0b1000 >> c)) {
        int bx = px + c;
        int by = py + r;

        if (bx >= 0 && bx < COLS && by >= 0 && by < ROWS) {
          if (board[by][bx] == 0) {
            int screenX = BOARD_X + bx * BLOCK;
            int screenY = BOARD_Y + by * BLOCK;
            tft.fillRect(screenX, screenY, BLOCK, BLOCK, TFT_BLACK);
          }
        }
      }
    }
  }
}

void drawCurrentPiece() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shapes[pieceType][pieceRot][r] & (0b1000 >> c)) {
        int bx = pieceX + c;
        int by = pieceY + r;

        if (by >= 0) drawBlock(bx, by, colors[pieceType + 1]);
      }
    }
  }
}

void drawBlock(int col, int row, uint16_t color) {
  int px = BOARD_X + col * BLOCK;
  int py = BOARD_Y + row * BLOCK;

  tft.fillRect(px, py, BLOCK, BLOCK, color);
  tft.drawRect(px, py, BLOCK, BLOCK, TFT_BLACK);
  tft.drawRect(px + 1, py + 1, BLOCK - 2, BLOCK - 2, TFT_WHITE);
}

void drawSidebar() {
  int x = 235;

  tft.fillRect(x, 50, 80, 400, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  tft.setCursor(x, 55);
  tft.print("NEXT");
  drawMiniPiece(nextType, x, 85);

  tft.setCursor(x, 160);
  tft.print("HOLD");
  if (holdType != -1) drawMiniPiece(holdType, x, 190);

  tft.setCursor(x, 270);
  tft.print("SCORE");
  tft.setCursor(x, 295);
  tft.print(score);

  tft.setCursor(x, 335);
  tft.print("HIGH");
  tft.setCursor(x, 360);
  tft.print(highScore);

  tft.setCursor(x, 400);
  tft.print("LVL");
  tft.setCursor(x, 425);
  tft.print(level);
}

void drawMiniPiece(int type, int startX, int startY) {
  int mini = 10;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (shapes[type][0][r] & (0b1000 >> c)) {
        tft.fillRect(startX + c * mini, startY + r * mini, mini, mini, colors[type + 1]);
        tft.drawRect(startX + c * mini, startY + r * mini, mini, mini, TFT_BLACK);
      }
    }
  }
}

void drawGameOver() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(55, 150);
  tft.print("GAME OVER");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(70, 220);
  tft.print("Score: ");
  tft.print(score);

  tft.setCursor(55, 255);
  tft.print("High: ");
  tft.print(highScore);

  tft.setCursor(40, 320);
  tft.print("START = Menu");
}