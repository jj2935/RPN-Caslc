// PROJECT  :RPN Calculator
// PURPOSE  :To calculate equations in postfix
// COURSE   :ICS3U-E2
// AUTHOR   :C. D'Arcy
// DATE     :2025 04 21
// MCU      :328P (Nano)
// STATUS   :Working
// REFERENCE: http://darcy.rsgc.on.ca/ACES/TEI3M/Tasks.html#RPN

#include <LiquidCrystal.h>
#include "Stack.h"

#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define KEYPAD_PIN A5
#define KEYPAD_IDLE_THRESHOLD 20
#define INPUT_BUFFER_SIZE 17
#define VALUE_BUFFER_SIZE 10
#define DECIMAL_PLACES_RESULT 4
#define DECIMAL_PLACES_STACK 2
#define NUMBER_WIDTH_RESULT 7
#define NUMBER_WIDTH_STACK 7
#define ERROR_DISPLAY_MS 1000
#define KEYPAD_NUM_KEYS 16

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
uint16_t thresholds[] = { 55, 58, 62, 66, 75, 81, 88, 97, 116, 132, 152, 179, 255, 341, 512, 1024 };
char keys[] = { '+', 'E', '.', '0', '-', '3', '2', '1', '*', '6', '5', '4', '/', '9', '8', '7' };
Stack stack;
char inputBuffer[INPUT_BUFFER_SIZE];
uint8_t inputIndex = 0;
bool newInput = true, showResult = false;
float resultValue = 0;

void setup() {
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.print("RPN Calculator");
  delay(ERROR_DISPLAY_MS);
  lcd.clear();
  updateDisplay();
}

void loop() {
  char key = getKey();
  if (!key) return;
  
  if (isdigit(key)) handleDigit(key);
  else if (key == '.') handleDecimal();
  else if (key == 'E') handleEnter();
  else if (strchr("+-*/", key)) handleOperator(key);
  
  updateDisplay();
  while (analogRead(KEYPAD_PIN) > KEYPAD_IDLE_THRESHOLD);
}

char getKey() {
  uint16_t val = analogRead(KEYPAD_PIN);
  if (val < KEYPAD_IDLE_THRESHOLD) return 0;
  val = analogRead(KEYPAD_PIN);
  for (uint8_t i = 0; i < KEYPAD_NUM_KEYS; i++)
    if (val <= thresholds[i]) return keys[i];
  return 0;
}

void handleDigit(char key) {
  if (newInput) { inputIndex = 0; newInput = showResult = false; inputBuffer[0] = '\0'; }
  if (inputIndex < INPUT_BUFFER_SIZE - 1) {
    inputBuffer[inputIndex++] = key;
    inputBuffer[inputIndex] = '\0';
  }
}

void handleDecimal() {
  if (newInput) {
    strcpy(inputBuffer, "0."); inputIndex = 2; newInput = showResult = false;
  } else if (!strchr(inputBuffer, '.') && inputIndex < INPUT_BUFFER_SIZE - 1) {
    inputBuffer[inputIndex++] = '.'; inputBuffer[inputIndex] = '\0';
  }
}

void handleEnter() {
  stack.push(atof(inputBuffer));
  inputBuffer[0] = '\0'; inputIndex = 0; newInput = true;
}

void handleOperator(char op) {
  if (stack.size() < 2) { showError("Stack<2"); return; }

  float b = stack.pop(), a = stack.pop(), res = 0;
  
  switch (op) {
    case '+': res = a + b; break;
    case '-': res = a - b; break;
    case '*': res = a * b; break;
    case '/':
      if (b == 0) {
        showError("Div by 0"); stack.push(a); stack.push(b); return;
      }
      res = a / b; break;
  }
  
  stack.push(res);
  resultValue = res; showResult = true; newInput = true;
}

void showError(const char* msg) {
  lcd.clear(); lcd.print("Error: "); lcd.print(msg);
  delay(ERROR_DISPLAY_MS); lcd.clear();
}

void updateDisplay() {
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Top: ");
  
  if (!stack.isEmpty()) {
    char buffer[VALUE_BUFFER_SIZE];
    dtostrf(stack.peek(), NUMBER_WIDTH_STACK, DECIMAL_PLACES_STACK, buffer);
    lcd.print(buffer);
  } else lcd.print("(empty)");

  lcd.setCursor(0, 1);
  if (showResult) {
    char buffer[VALUE_BUFFER_SIZE];
    lcd.print("Ans: ");
    dtostrf(resultValue, NUMBER_WIDTH_RESULT, DECIMAL_PLACES_RESULT, buffer);
    lcd.print(buffer);
  } else lcd.print(inputBuffer);
}




