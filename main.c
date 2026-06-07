/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.c
* @brief          : Whispers Beneath the Triangle - STM32 UART Game
******************************************************************************
*/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */
static char txBuffer[512];
static char rxLineBuffer[64];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DMA_Init(void);

/* USER CODE BEGIN PFP */
int Game_Run(void);
void uartPrint(const char *text);
void uartPrintChar(char c);
int uartPrintf(const char *fmt, ...);
int uartScanf(const char *fmt, ...);
int uartGetchar(void);
void uartReadLine(char *buffer, int maxLen);
void clearScreen(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* =====================================================
* STM32 UART COMPATIBILITY LAYER
* =====================================================
* These helpers let the original Windows console game run on UART.
* printf(), scanf(), getchar(), Sleep(), and system("cls") are replaced
* without changing every gameplay function by hand.
*/
void uartPrint(const char *text) {
/* PuTTY needs CRLF line endings. Convert every '\n' into '\r\n'
 * so output does not appear like a staircase.
 */
while (*text) {
if (*text == '\n') {
uint8_t crlf[2] = {'\r', '\n'};
HAL_UART_Transmit(&huart2, crlf, 2, HAL_MAX_DELAY);
} else {
HAL_UART_Transmit(&huart2, (uint8_t*)text, 1, HAL_MAX_DELAY);
}
text++;
}
}

void uartPrintChar(char c) {
HAL_UART_Transmit(&huart2, (uint8_t*)&c, 1, HAL_MAX_DELAY);
}

int uartPrintf(const char *fmt, ...) {
va_list args;
int len;

va_start(args, fmt);
len = vsnprintf(txBuffer, sizeof(txBuffer), fmt, args);
va_end(args);

if (len < 0) {
return len;
}

/* If a message is longer than txBuffer, send the truncated safe part. */
if (len >= (int)sizeof(txBuffer)) {
len = sizeof(txBuffer) - 1;
txBuffer[len] = '\0';
}

uartPrint(txBuffer);
return len;
}

int uartGetchar(void) {
static uint8_t skipNextLF = 0;
uint8_t ch;

while (1) {
HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY);

/* PuTTY commonly sends CR when ENTER is pressed. Some terminals send CRLF.
 * Convert CR to '\n' and discard the following LF if it appears.
 */
if (skipNextLF && ch == '\n') {
skipNextLF = 0;
continue;
}

skipNextLF = 0;

if (ch == '\r') {
skipNextLF = 1;
return '\n';
}

return (int)ch;
}
}

void uartReadLine(char *buffer, int maxLen) {
int i = 0;
char ch;

if (maxLen <= 0) {
return;
}

while (i < maxLen - 1) {
ch = (char)uartGetchar();

if (ch == '\r' || ch == '\n') {
break;
}

/* Backspace support for serial terminals. */
if ((ch == '\b' || ch == 127) && i > 0) {
i--;
uartPrint("\b \b");
continue;
}

buffer[i++] = ch;
uartPrintChar(ch);  // Echo typed character back to terminal.
}

buffer[i] = '\0';
uartPrintChar('\r');
uartPrintChar('\n');
}

/* =====================================================
 * UART scanf replacement
 * =====================================================
 * Supports the formats used by this game:
 *   scanf("%d", &intValue);
 *   scanf(" %c", &charValue);
 *   scanf("%f", &floatValue);
 * Returns 1 when a value was read successfully, 0 on invalid input.
 */
int uartScanf(const char *fmt, ...) {
va_list args;
int result = 0;
char *endptr = NULL;
long intValue = 0;
float floatValue = 0.0f;
int *intOut = NULL;
char *charOut = NULL;
float *floatOut = NULL;
char ch = 0;

va_start(args, fmt);

if (strchr(fmt, 'd') != NULL) {
intOut = va_arg(args, int *);
uartReadLine(rxLineBuffer, sizeof(rxLineBuffer));

endptr = NULL;
intValue = strtol(rxLineBuffer, &endptr, 10);

while (endptr != NULL && (*endptr == ' ' || *endptr == '\t')) {
endptr++;
}

if (endptr != NULL && endptr != rxLineBuffer && *endptr == '\0') {
*intOut = (int)intValue;
result = 1;
}
}
else if (strchr(fmt, 'c') != NULL) {
charOut = va_arg(args, char *);

do {
ch = (char)uartGetchar();
} while (ch == '\n' || ch == ' ' || ch == '\t');

*charOut = ch;
uartPrintChar(ch);
uartPrintChar('\r');
uartPrintChar('\n');
result = 1;
}
else if (strchr(fmt, 'f') != NULL) {
floatOut = va_arg(args, float *);
uartReadLine(rxLineBuffer, sizeof(rxLineBuffer));

endptr = NULL;
floatValue = strtof(rxLineBuffer, &endptr);

while (endptr != NULL && (*endptr == ' ' || *endptr == '\t')) {
endptr++;
}

if (endptr != NULL && endptr != rxLineBuffer && *endptr == '\0') {
*floatOut = floatValue;
result = 1;
}
}

va_end(args);
return result;
}
void clearScreen(void) {
/* ANSI clear-screen sequence. Works in PuTTY, Tera Term, Arduino Serial Monitor alternatives, etc. */
uartPrint("\033[2J\033[H");
}

/* Redirect original console calls to STM32 UART/HAL. */
#define printf  uartPrintf
#define scanf   uartScanf
#define getchar uartGetchar
#define Sleep   HAL_Delay


// -------------------- CONSTANTS --------------------
#define EASY 1
#define HARD 2

#define MASON 1
#define IVAN 2

#define HK_M27 1
#define M1_GARAND 2
#define AK_105 3
#define SHAK_12 4

#define OCEAN_FLOOR 1
#define MOSASAUR_ARENA 2
#define PYRAMID_CHAMBER 3
#define CRYSTAL_INTERIOR 4
#define LEVIATHAN_ARENA 5
#define CRYSTAL_SPIRES 6
#define FLESH_MOUND 7

// -------------------- GLOBAL VARIABLES --------------------
int difficulty;
int character;
int currentWeapon;
int currentArea = OCEAN_FLOOR;

int machineryInvestigated = 0;
int obelisksScanned = 0;
int mosasaurDefeated = 0;
int pyramidOpened = 0;
int crystalKeyHalf = 0;
int serpentKeyHalf = 0;
int leviathanGateKey = 0;
int poisonPuzzleMode = 0;

int torpedoes = 6;
int depthCharges = 4;
int sonarDisrupted = 0;
int mosasaurTurnCount = 0;

int hkAmmo = 40;
int garandAmmo = 8;
int akAmmo = 30;
int shakAmmo = 10;

int playerHP = 100;
int enemyHP;
int bandages = 2;

int mosasaurUpgrade = 0;
int prismUpgrade = 0;
int scaleUpgrade = 0;

int enemyType;
int enemyDamage;
int barrierActive = 0;
int barrierCooldown = 0;
int enemyDodging = 0;

int crystalTurns = 0;
int burnTurns = 0;
int poisonTurns = 0;
int frostfireTurns = 0;
int enemyCrystalTurns = 0;

int playerParalyzed = 0;
int crystalDamage = 5;
int burnDamage = 5;
int poisonDamage = 10;
int frostfireDamage = 6;
int enemyCrystalDamage = 8;

int prismTurnCount = 0;
int prismReforms = 3;
int prismPhase = 0;

int serpentCrystalCharge = 0;
const int serpentCrystalGoal = 3;

// -------------------- FUNCTIONS --------------------
void slowPrint(const char *text);
void continuePrompt();
void startGame();
void chooseCharacter();
void chooseDifficulty();
void equipWeapons();
void fireWeapon(int weapon, int shots);
void oceanFloorDiscovery();
void investigateMachinery();
void scanObelisks();
void mosasaurFight();
void submarineMenu();
void fireTorpedo();
void dropDepthCharge();
void fireMachineGun();
void sonarScan();
void sonarGlitch();
void repairSonar();
void mosasaurTurn();
void resetSubmarine();
void pyramidChamber();
void pressureEqualizerPuzzle();
void chooseAlienPath();
void leviathanFight();
void crystalSpires();
void fleshMound();
void updateLeviathanGateKey();
void prismStalkerDefeated();
void imminentDiffraction();
void prismStalkerTurn();
void hollowSerpentTurn();
void baitSerpentLightning();
const char* getZodiacName(int z);
extern int getOppositeZodiac(int z);
void printZodiacSymbol(int z);
void createEnemy(int type);
void giveUpgrade();
void applyStatusEffects();
void applyEnemyStatusEffects();
void combatMenu();
void leviathanCombatMenu(int seerActive, int seerHP, int revealedZodiac, int obelisksBroken, int damagePhaseTurns);
void showMap();
void switchWeapon();
void attackEnemy();
void reloadWeapon();
int useBandage();
int getPlayerMaxHP();
void resetCheckpointState();
int retryFromCheckpoint(const char *encounterName);
void enemyTurn();
void spawnBarrier();
void barrierInfo();
int handleBarrierWithCurrentWeapon(int baseDamage);
const char* getCharacterName();
const char* getWeaponName(int weapon);
extern int CalculateDamage(int baseDamage, int armorActive, int headshot);
extern int GetBulletBounceChance(int weapon);
// -------------------- MAIN --------------------
int Game_Run(void) {
srand(HAL_GetTick());
printf("               / \\\n");
printf("              /   \\\n");
printf("             /     \\\n");
printf("            /   /\\  \\\n");
printf("           /   /  \\  \\\n");
printf("          /   /    \\  \\\n");
printf("         /   /______\\  \\\n");
printf("        /               \\\n");
printf("       /_________________\\\n\n");
printf("========================================\n");
printf("      WELCOME TO WHISPERS BENEATH\n");
printf("              THE TRIANGLE\n\n");
printf("        HUNT FOR THE LEVIATHAN\n");
printf("========================================\n");
startGame();
chooseDifficulty();

slowPrint("=== MISSION BRIEFING ===\nWelcome, Operative.\n\nAn aircraft carrier has vanished without a trace.\nLast known location: 25.0000 N, 71.0000 W, North Atlantic Ocean.\n\nYou are being deployed via submarine to investigate.\n\nYour mission:\n- Locate the wreck\n- Enter the submerged crystal pyramid structure\n- Solve ancient mechanisms\n- Eliminate hostile entities\n\nWarning: A massive aquatic organism has been detected.\nInitial scans resemble a blue whale in size and shape...\nbut its behavior is highly aggressive.\nThreat level: EXTREME.\n\nFurther anomalies suggest dimensional instability beyond the site...\nPrepare for unknown environments.\n\nSelect your operative.\n");
chooseCharacter();
equipWeapons();


oceanFloorDiscovery();

if (playerHP <= 0) {
printf("\nGAME OVER! The submarine was destroyed.\n");
return 0;
}

if (mosasaurDefeated) {
printf("\nThe Mosasaur has been defeated!\n");
slowPrint("\nThe submerged crystal pyramid reacts to the guardian's defeat...\n");
slowPrint("The entrance to the Pyramid Chamber opens.\n");
pyramidChamber();
}

return 0;
}

// -------------------- SLOW PRINT --------------------
void slowPrint(const char *text) {
while (*text) {
printf("%c", *text);
/* STM32 UART output is sent immediately by uartPrint/uartPrintf. */
Sleep(35);
text++;
}
}
void continuePrompt() {
printf("\nPress ENTER to continue...");
while (getchar() != '\n');
}
// -------------------- START GAME --------------------
void startGame() {
char choice;
printf("=== MAIN MENU ===\n");
printf("Enter p to play or x to exit: ");
scanf(" %c", &choice);

if (choice == 'x' || choice == 'X') {
printf("Exiting game.\n");
while (1) { }
}
}
// -------------------- CHARACTER SELECTION --------------------
void chooseCharacter() {
printf("\n=== CHARACTER SELECTION ===\n");
printf("1. Mason\n");
printf("   Weapons: HK M27 and M1 Garand\n");
printf("2. Ivan\n");
printf("   Weapons: AK-105 and ShAK-12\n");
printf("Choice: ");
scanf("%d", &character);

if (character == MASON) {
currentWeapon = HK_M27;
slowPrint("You selected Mason.\n");
}
else if (character == IVAN) {
currentWeapon = AK_105;
slowPrint("You selected Ivan.\n");
}
else {
slowPrint("Invalid character. Defaulting to Mason.\n");
character = MASON;
currentWeapon = HK_M27;
}
}
// -------------------- DIFFICULTY --------------------
void chooseDifficulty() {
printf("\nSelect difficulty:\n");
printf("1. Easy\n");
printf("2. Hard\n");
printf("Choice: ");
scanf("%d", &difficulty);
clearScreen();

if (difficulty != EASY && difficulty != HARD) {
printf("Invalid Difficulty, setting to Easy.\n");
difficulty = EASY;
}
}
// -------------------- WEAPON CHOICE --------------------
void equipWeapons() {
hkAmmo = 40;
garandAmmo = 8;
akAmmo = 30;
shakAmmo = 10;

printf("\n=== ARMORY ===\n");
printf("Character: %s\n", getCharacterName());

if (character == MASON) {
printf("Weapons equipped:\n");
printf("- HK M27\n");
printf("  Ammo Type: 5.56x45mm\n");
printf("  Magazine Capacity: 40 rounds\n");
printf("- M1 Garand\n");
printf("  Ammo Type: .30-06\n");
printf("  Clip Capacity: 8 rounds\n");
}
else if (character == IVAN) {
printf("Weapons equipped:\n");
printf("- AK-105\n");
printf("  Ammo Type: 5.45x39mm\n");
printf("  Magazine Capacity: 30 rounds\n");
printf("- ShAK-12\n");
printf("  Ammo Type: 12.7x55mm\n");
printf("  Magazine Capacity: 10 rounds\n");
}
}
// -------------------- OCEAN FLOOR DISCOVERY --------------------
void oceanFloorDiscovery() {
int choice;

currentArea = OCEAN_FLOOR;

slowPrint("\n=== OCEAN FLOOR DISCOVERY ===\n");
slowPrint("Your submarine settles near the wreck of the vanished aircraft carrier.\n");
slowPrint("Beneath the ocean lies a massive crystalline pyramid.\n");
slowPrint("The water above it appears to be glowing.\n");
slowPrint("It is not natural.\n\n");

while (machineryInvestigated == 0 || obelisksScanned == 0) {
printf("\n--- OCEAN FLOOR OBJECTIVES ---\n");
printf("1. Investigate machinery [%s]\n", machineryInvestigated ? "COMPLETE" : "INCOMPLETE");
printf("2. Scan obelisks [%s]\n", obelisksScanned ? "COMPLETE" : "INCOMPLETE");
printf("3. Show map\n");
printf("Choice: ");
scanf("%d", &choice);

if (choice == 1) {
investigateMachinery();
}
else if (choice == 2) {
scanObelisks();
}
else if (choice == 3) {
showMap();
}
else {
printf("Invalid choice.\n");
}
}

slowPrint("\nBoth ocean floor objectives are complete.\n");
slowPrint("The pyramid entrance remains sealed...\n");
slowPrint("A massive shadow moves through the water.\n");
slowPrint("A prehistoric looking sea creature is guarding the entrance!\n");

mosasaurFight();
}
void investigateMachinery() {
if (machineryInvestigated) {
printf("You already investigated the machinery.\n");
return;
}

slowPrint("\nYou approach the wrecked carrier machinery...\n");
slowPrint("The control panels are dead, but strange energy pulses through the metal.\n");
slowPrint("Your compass points toward the submerged pyramid.\n");

machineryInvestigated = 1;
}
void scanObelisks() {
if (obelisksScanned) {
printf("You already scanned the obelisks.\n");
return;
}

slowPrint("\nYou scan the glowing obelisks around the pyramid entrance...\n");
slowPrint("Ancient symbols flash across your scanner.\n");
slowPrint("Ancient symbols flicker across your display...\n\n");
// Aries
printf("   .-.   .-.\n");
printf("  (_  \\ /  _)\n");
printf("      |\n");
printf("      |\n\n");
Sleep(600);

// Taurus
printf("   .     .\n");
printf("  '.___.'\n");
printf("  .'   `.\n");
printf(" :       :\n");
printf(" :       :\n");
printf("  `.___.'\n\n");
Sleep(600);

// Gemini
printf("   ._____.\n");
printf("     | |\n");
printf("     | |\n");
printf("    _|_|_\n");
printf("   '     '\n\n");
Sleep(600);

// Cancer
printf("     .--.\n");
printf("    /   _`.\n");
printf("   (_) ( )\n");
printf("  '.    /\n");
printf("    `--'\n\n");
Sleep(600);

// Leo
printf("     .--.\n");
printf("    (    )\n");
printf("   (_)  /\n");
printf("       (_,\n\n");
Sleep(600);

// Virgo
printf("   _\n");
printf("  ' `:--.--.\n");
printf("     |  |  |_\n");
printf("     |  |  | )\n");
printf("     |  |  |/\n");
printf("         (J\n\n");
Sleep(600);

// Libra
printf("       __\n");
printf("  ___.'  '.___\n");
printf("  ____________\n\n");
Sleep(600);

// Scorpius
printf("   _\n");
printf("  ' `:--.--.\n");
printf("     |  |  |\n");
printf("     |  |  |\n");
printf("     |  |  |  ..,\n");
printf("           `---':\n\n");
Sleep(600);

// Sagittarius
printf("      ...\n");
printf("      .':\n");
printf("    .'\n");
printf(" `..'\n");
printf(" .'.`\n\n");
Sleep(600);

// Capricorn
printf("       _\n");
printf(" \\     /_)\n");
printf("  \\    /`.\n");
printf("   \\  /   ;\n");
printf("    \\/ __.'\n\n");
Sleep(600);

// Aquarius
printf("  .-\"-._.-\"-._.-\n");
printf("  .-\"-._.-\"-._.-\n\n");
Sleep(600);

// Pisces
printf("    `-.    .-'\n");
printf("       :  :\n");
printf("     --:--:--\n");
printf("       :  :\n");
printf("    .-'    `-.\n\n");
Sleep(600);


obelisksScanned = 1;
}
// -------------------- MOSASAUR SUBMARINE FIGHT --------------------
void sonarScan() {
clearScreen();
slowPrint("Initializing sonar...\n");
Sleep(400);

for (int i = 0; i < 3; i++) {
clearScreen();

printf("\n        SONAR ACTIVE\n\n");
printf("          ~      \n");
printf("       ~     ~   \n");
printf("    ~    O     ~ \n");
printf("       ~     ~   \n");
printf("          ~      \n");

Sleep(300);

clearScreen();
printf("\n        SONAR ACTIVE\n\n");
printf("       ~     ~   \n");
printf("    ~    O     ~ \n");
printf(" ~             ~ \n");
printf("    ~         ~  \n");
printf("       ~     ~   \n");

Sleep(300);
}

slowPrint("\nContact detected...\n");
Sleep(400);
slowPrint("Size: MASSIVE\n");
slowPrint("Movement: FAST\n");
slowPrint("Classification: UNKNOWN\n");
}
void sonarGlitch() {
clearScreen();

printf("\n!!! SONAR FAILURE !!!\n\n");

for (int i = 0; i < 3; i++) {
printf(" ~~~ #### ~~~ #### ~~~ \n");
printf(" #### ~~~ #### ~~~ ####\n");
printf(" ~~~ #### ~~~ #### ~~~ \n\n");
Sleep(200);
}

slowPrint("Signal lost...\n");
}
void mosasaurFight() {
int retry;
currentArea = MOSASAUR_ARENA;
createEnemy(1);
slowPrint("\nScanning surrounding waters...\n");
sonarScan();
slowPrint("\nThe signal is getting closer...\n");
slowPrint("That is NOT a whale.\n");
slowPrint("\n=== MOSASAUR ENCOUNTER ===\n");
slowPrint("The submarine shakes as the Mosasaur circles the hull.\n");
slowPrint("Weapons online: torpedoes and depth charges.\n");

while (1) {
resetSubmarine();
createEnemy(1);

while (playerHP > 0 && enemyHP > 0) {
submarineMenu();
}

if (playerHP <= 0) {
slowPrint("\nThe submarine has been destroyed!\n");

printf("\nRetry from checkpoint?\n1. Yes\n2. No\nChoice: ");
scanf("%d", &retry);

if (retry == 1) {
slowPrint("Reinitializing systems...\n");
continue; // restart fight
} else {
slowPrint("Mission failed.\n");
return;
}
}

// WIN
slowPrint("\nThe Mosasaur sinks into the abyss.\n");
mosasaurDefeated = 1;
slowPrint("The ocean grows still.\n");
Sleep(500);
slowPrint("You exit the submarine and approach the corpse.\n");
Sleep(700);
slowPrint("You cut into the Mosasaur...\n");
Sleep(700);
slowPrint("You remove its heart.\n");
Sleep(500);
slowPrint("...\n");
Sleep(500);
slowPrint("You consume the Mosasaur's heart.\n");
Sleep(700);
slowPrint("Your body surges with energy.\n");
Sleep(700);
playerHP = 100;
giveUpgrade();

mosasaurDefeated = 1;
break;
}
}
void submarineMenu() {
int choice;
clearScreen();
printf("\n=== SUBMARINE COMBAT ===\n");
printf("Submarine HP: %d\n", playerHP);
printf("Mosasaur HP: %d\n", enemyHP);
printf("Torpedoes: %d\n", torpedoes);
printf("Depth Charges: %d\n", depthCharges);
printf("Sonar: %s\n", sonarDisrupted ? "DISRUPTED" : "ONLINE");
if ((mosasaurTurnCount + 1) % 3 == 0) {
slowPrint("The water begins to vibrate...\n");
}

printf("\n1. Fire Torpedo\n");
printf("2. Drop Depth Charge\n");
printf("3. Repair Sonar\n");
printf("4. Show Map\n");
printf("5. Fire Heavy Machine Gun\n");
printf("Choice: ");
scanf("%d", &choice);

if (choice == 1) {
fireTorpedo();
}
else if (choice == 2) {
dropDepthCharge();
}
else if (choice == 3) {
repairSonar();
}
else if (choice == 4) {
showMap();
}
else if (choice == 5) {
fireMachineGun();
}
else {
printf("Invalid choice.\n");
}
}
void fireTorpedo() {
int accuracy;

if (torpedoes <= 0) {
printf("No torpedoes left!\n");
Sleep(1200);
return;
}

torpedoes--;
accuracy = sonarDisrupted ? 10 : 70;

slowPrint("Torpedo launched!\n");

if (rand() % 100 < accuracy) {
enemyHP -= 120;
printf("Direct hit! You dealt 120 damage.\n");
}
else {
printf("Torpedo missed!\n");
}

Sleep(1200);

if (enemyHP > 0) {
mosasaurTurn();
}
}
void dropDepthCharge() {
int accuracy;

if (depthCharges <= 0) {
printf("No depth charges left!\n");
Sleep(1200);
return;
}

depthCharges--;
accuracy = sonarDisrupted ? 10 : 50;

slowPrint("Depth charge dropped!\n");

if (rand() % 100 < accuracy) {
enemyHP -= 180;
printf("Explosion hit! You dealt 180 damage.\n");
}
else {
printf("Depth charge missed!\n");
}

Sleep(1200);

if (enemyHP > 0) {
mosasaurTurn();
}
}
void fireMachineGun() {
slowPrint("Heavy machine gun firing!\n");

enemyHP -= 40;
printf("Guaranteed hit! You dealt 40 damage.\n");

Sleep(1200);

if (enemyHP > 0) {
mosasaurTurn();
}
}
void repairSonar() {
if (!sonarDisrupted) {
printf("Sonar is already online.\n");
Sleep(1200);
return;
}

slowPrint("Repairing sonar systems...\n");
sonarDisrupted = 0;
slowPrint("Sonar restored!\n");
Sleep(1200);

if (enemyHP > 0) {
mosasaurTurn();
}
}
void resetSubmarine() {
playerHP = (difficulty == HARD) ? 300 : 250;
torpedoes = 6;
depthCharges = 4;
sonarDisrupted = 0;
mosasaurTurnCount = 0;
}
void mosasaurTurn() {
mosasaurTurnCount++;

if (mosasaurTurnCount % 3 == 0) {
slowPrint("\nThe water vibrates violently...\n");
Sleep(400);

slowPrint("Sonic roar!\n");
sonarGlitch();
sonarDisrupted = 1;
slowPrint("Sonar disrupted! Accuracy reduced to 10%.\n");

Sleep(1200);
}
else {
slowPrint("\nThe Mosasaur lunges at the submarine!\n");
playerHP -= enemyDamage;
printf("Submarine took %d damage.\n", enemyDamage);

Sleep(1200);
}
}
// -------------------- PRESSURE EQUALIZER PUZZLE --------------------
void pyramidChamber() {
currentArea = PYRAMID_CHAMBER;

slowPrint("\n=== PYRAMID CHAMBER ===\n");

// Environment description
slowPrint("You step out of the submarine and onto dry sand.\n");
slowPrint("No water touches the pyramid.\n");
slowPrint("An invisible barrier holds back the ocean itself.\n\n");

slowPrint("You enter the structure.\n");
slowPrint("Your compass spins wildly.\n");
slowPrint("Instrumentation is unreliable here.\n");
slowPrint("You must rely on observation alone.\n\n");

// Room description
slowPrint("Crystal pillars rise from the chamber floor.\n");
slowPrint("The Room begins to fill with water.\n");
slowPrint("At the center stands a dark crystalline core with 3 small rotating pillars around it.\n\n");
slowPrint("You quickly put on your oxygen tank and diving mask.\n");
slowPrint("You must quickly figure out how equalize the pressure so that the water will drain.\n");

//puzzle triggers
pressureEqualizerPuzzle();

// Outcome if puzzle is solved
slowPrint("\nThe water drains from the pyramid.\n");
pyramidOpened = 1; // marks progress
currentArea = CRYSTAL_INTERIOR;
showMap();
slowPrint("The core hums, glowing brighter.\n");
slowPrint("Energy erupts from the crystal.\n");
slowPrint("The chamber collapses inward.\n");
slowPrint("Your submarine slams into the structure.\n");
slowPrint("Everything is pulled into the glowing core.\n");
slowPrint("You wake up in the wreckage and you are now in an alien world.\n");
slowPrint("The ground beneath you gleams like scales, reflecting the light of a blood-red star.\n");
slowPrint("Above, a sky of countless stars stretches endlessly.\n");
slowPrint("In the distance, a massive serpent coils around an ancient temple.\n");
slowPrint("Two paths stretch out before you.\n\n");
slowPrint("To your left, jagged crystal spires rise from the ground.\n");
slowPrint("To your right, a mound piled high with bones, flesh, and scales.\n");
slowPrint("You don't know where you are.\n");
slowPrint("But standing still isn't an option.\n\n");
slowPrint("Which direction do you take?\n\n");
printf("1. Enter the Crystal Spires\n");
printf("2. Approach the Flesh Mound\n");
chooseAlienPath();
}
void pressureEqualizerPuzzle() {
int retry = 1;

while (retry == 1) {
int alpha = 10, beta = 50, gamma = 20;
const int TARGET = 1000;
int tolerance = (difficulty == HARD) ? 100 : 200;
int choice, solved = 0, moves = 0;
int maxMoves = (difficulty == HARD) ? 7 : 10;

while (!solved && moves < maxMoves) {
int totalPressure = (alpha * 20) + (beta * -12) + (gamma * 15);

clearScreen();
printf("=== %s ===\n", poisonPuzzleMode ? "POISON PRESSURE LOCK" : "PYRAMID HYDRAULIC LOCK");
printf("Goal: reach %d pressure.\n\n", TARGET);
printf("Pressure Rules:\n");
printf("- Alpha: heavy (+20 per unit)\n");
printf("- Beta: buoyant (-12 per unit)\n");
printf("- Gamma: stable (+15 per unit)\n\n");
printf("Current Pressure: %d\n", totalPressure);
printf("Moves remaining: %d\n", maxMoves - moves);
if (difficulty == HARD) printf("Tolerance: +/- %d\n", tolerance);

printf("\n[Alpha: %d]  [Beta: %d]  [Gamma: %d]\n\nGauge: [", alpha, beta, gamma);
for (int i = 0; i < 20; i++) printf("%c", (i < (int)(totalPressure / 10)) ? '#' : '-');

printf("]\n\n1. Pump ALPHA (+100 Alpha, -50 Beta)\n");
printf("2. Pump BETA  (+100 Beta, -50 Gamma)\n");
printf("3. Pump GAMMA (+100 Gamma, -50 Alpha)\n");
printf("4. Drain All  (-150 each)\n");
if (difficulty == HARD) printf("5. Emergency Valve (-pressure shift)\n");

printf("\nAction: ");
/* STM32 UART output is sent immediately by uartPrint/uartPrintf. */

if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
printf("Invalid input.\n");
Sleep(700);
continue;
}

moves++;

switch (choice) {
case 1: alpha += 100; beta -= 50; break;
case 2: beta += 100; gamma -= 50; break;
case 3: gamma += 100; alpha -= 50; break;
case 4: alpha -= 150; beta -= 150; gamma -= 150; break;
case 5:
if (difficulty == HARD) {
alpha -= 50;
beta += 50;
gamma -= 50;
printf("\nValve releases pressure...\n");
Sleep(800);
} else {
printf("Invalid choice.\n");
moves--;
Sleep(700);
}
break;
default:
printf("Invalid choice.\n");
moves--;
Sleep(700);
break;
}

if (alpha < 0) alpha = 0;
if (beta < 0) beta = 0;
if (gamma < 0) gamma = 0;

totalPressure = (alpha * 20) + (beta * -12) + (gamma * 15);

if (totalPressure >= TARGET - tolerance && totalPressure <= TARGET + tolerance) {
clearScreen();
if (poisonPuzzleMode) {
printf("\n[SUCCESS] The pressure stabilizes!\n");
printf("The poison drains before it reaches you.\n");
} else {
printf("\n[SUCCESS] The door opens!\n");
}
Sleep(1500);
solved = 1;
} else {
Sleep(400);
}
}

if (solved) return;

clearScreen();

if (poisonPuzzleMode) {
printf("\n[FAILURE] The poison reaches you!\n");
printf("The toxic liquid burns into your body!\n");

poisonTurns += 3;
poisonDamage *= 2;

printf("Poison damage has doubled!\n");
Sleep(1500);
return;
}

printf("\n[FAILURE] The chamber floods!\n");
playerHP -= 200;
printf("The crushing water deals 200 damage!\nPlayer HP: %d\n", playerHP);

if (playerHP <= 0) {
printf("\nYou were killed by the flood.\n");
}

printf("\nRestart from checkpoint?\n1. Yes\n2. No\nChoice: ");
/* STM32 UART output is sent immediately by uartPrint/uartPrintf. */

if (scanf("%d", &retry) != 1) {
while (getchar() != '\n');
retry = 2;
}

if (retry == 1) {
playerHP = 100;
printf("\nCheckpoint restored.\n");
Sleep(1000);
clearScreen();
}
}
}

// -------------------- ALIEN WORLD PATH CHOICE --------------------
void updateLeviathanGateKey() {
if (crystalKeyHalf && serpentKeyHalf) {
leviathanGateKey = 1;
}
}
void chooseAlienPath() {
int choice;

while (!leviathanGateKey) {
updateLeviathanGateKey();

if (leviathanGateKey) {
break;
}

printf("\n=== ALIEN WORLD CROSSROADS ===\n");
printf("Leviathan Gate Key: %s\n", leviathanGateKey ? "COMPLETE" : "INCOMPLETE");
printf("Crystal Spires Key Half: %s\n", crystalKeyHalf ? "ACQUIRED" : "MISSING");
printf("Flesh Mound Key Half: %s\n", serpentKeyHalf ? "ACQUIRED" : "MISSING");
printf("\n1. Enter the Crystal Spires\n");
printf("2. Approach the Flesh Mound\n");
printf("3. Show Map\n");
printf("Choice: ");

if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
printf("Invalid input.\n");
Sleep(700);
continue;
}

if (choice == 1) {
crystalSpires();
}
else if (choice == 2) {
fleshMound();
}
else if (choice == 3) {
showMap();
}
else {
printf("Invalid choice.\n");
}
}

currentArea = LEVIATHAN_ARENA;
showMap();
slowPrint("\nThe two key halves lock together in your hands.\n");
slowPrint("The completed Leviathan Gate Key burns with ancient light.\n");
slowPrint("Far ahead, the Leviathan Gate opens.\n");
leviathanFight();
}

// -------------------- PRISM FIGHT --------------------
void crystalSpires(){
currentArea = CRYSTAL_SPIRES;
showMap();

if (crystalKeyHalf) {
slowPrint("\nYou return to the Crystal Spires. The shattered altar is silent.\n");
slowPrint("There is nothing else to claim here.\n");
return;
}

slowPrint("\n=== CRYSTAL SPIRES ===\n");
slowPrint("You walk between jagged towers of living crystal.\n");
slowPrint("Every footstep echoes like glass breaking underwater.\n");
slowPrint("Light bends unnaturally between the spires.\n");
slowPrint("Something moves inside the reflections.\n");
slowPrint("A Prism Stalker steps out of the light, its body splitting into sharp angles.\n");
Sleep(1500);

while (1) {
playerHP = getPlayerMaxHP();
createEnemy(2);

while (playerHP > 0 && enemyHP > 0) {
combatMenu();
}

if (playerHP > 0) break;

slowPrint("\nYou were defeated in the Crystal Spires.\n");
if (!retryFromCheckpoint("Crystal Spires")) return;
}

prismStalkerDefeated();
}
void prismStalkerDefeated() {
slowPrint("\nThe Prism Stalker fractures violently.\n");
slowPrint("Its body collapses into thousands of dull crystal shards.\n");
slowPrint("The light inside the spires fades.\n");
slowPrint("At the center of the shattered body, half of a black crystal key rises from the altar.\n");
slowPrint("You take it. The spires stop watching you.\n");

crystalKeyHalf = 1;
updateLeviathanGateKey();

slowPrint("\nCrystal Spires Key Half acquired.\n");
slowPrint("The path behind you grows silent. The Flesh Mound still waits.\n");
}
void imminentDiffraction() {
int choice;
int darkChance;
int damageTaken;

if (difficulty == EASY) {
darkChance = 100;
damageTaken = 20;
} else {
darkChance = 70;
damageTaken = 35;
}

slowPrint("\nIMMINENT DIFFRACTION!\n");
slowPrint("The Prism Stalker opens a thin slit through its own body.\n");
slowPrint("Bands of bright light and dark shadow stretch across the arena.\n");
slowPrint("The creature keeps moving, dragging the dark fringes out of place.\n");
slowPrint("You have only a moment to react.\n\n");

printf("1. Chase the moving dark fringe\n");
printf("2. Shoot the slit with your primary weapon\n");
printf("3. Freeze in place\n");
printf("Choice: ");

if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
choice = 3;
}

if (choice == 1) {
if (rand() % 100 < darkChance) {
printf("You dive into the shifting dark fringe and avoid the beam!\n");
} else {
printf("The dark fringe slides away before you reach it!\n");
printf("Focused light tears through you!\n");
playerHP -= damageTaken;
}
}
else if (choice == 2) {
if (currentWeapon == HK_M27 || currentWeapon == AK_105) {
printf("You snap your primary weapon up and fire into the moving slit!\n");
printf("The diffraction pattern collapses around the Prism Stalker!\n");
enemyHP -= 25;
printf("The Prism Stalker takes 25 damage.\n");
} else {
printf("Your heavy weapon is too slow to track the moving slit!\n");
printf("The bright fringe burns through you!\n");
playerHP -= damageTaken;
}
}
else {
printf("You hesitate as the bright fringe passes through you!\n");
playerHP -= damageTaken;
}

Sleep(1200);
}
void prismStalkerTurn() {
clearScreen();
prismTurnCount++;

if (prismTurnCount % 4 == 0) {
imminentDiffraction();
return;
}

if (prismTurnCount % 3 == 1) {
slowPrint("The Prism Stalker lunges forward and stabs you with a crystal spike!\n");
playerHP -= enemyDamage;
}
else if (prismTurnCount % 3 == 2) {
slowPrint("The Prism Stalker focuses starlight like a magnifying glass!\n");
playerHP -= 10;
if (burnTurns == 0) {
burnTurns = 2;
printf("You are burned!\n");
}
}
else {
printf("The Prism Stalker splits into shimmering afterimages!\n");
printf("Your next attack may miss unless you wait it out.\n");
enemyDodging = 1;
}
spawnBarrier();
}
// -------------------- Serpent --------------------
const char* getZodiacName(int z) {
const char* names[] = {
"Aries", "Taurus", "Gemini", "Cancer",
"Leo", "Virgo", "Libra", "Scorpius",
"Sagittarius", "Capricorn", "Aquarius", "Pisces"
};

if (z < 0 || z > 11) return "Unknown";
return names[z];
}

void printZodiacSymbol(int z) {
if (z == 0) {
printf("   .-.   .-.\n");
printf("  (_  \\ /  _)\n");
printf("      |\n");
printf("      |\n\n");
} else if (z == 1) {
printf("   .     .\n");
printf("  '.___.'\n");
printf("  .'   `.\n");
printf(" :       :\n");
printf(" :       :\n");
printf("  `.___.'\n\n");
} else if (z == 2) {
printf("   ._____.\n");
printf("     | |\n");
printf("     | |\n");
printf("    _|_|_\n");
printf("   '     '\n\n");
} else if (z == 3) {
printf("     .--.\n");
printf("    /   _`.\n");
printf("   (_) ( )\n");
printf("  '.    /\n");
printf("    `--'\n\n");
} else if (z == 4) {
printf("     .--.\n");
printf("    (    )\n");
printf("   (_)  /\n");
printf("       (_,\n\n");
} else if (z == 5) {
printf("   _\n");
printf("  ' `:--.--.\n");
printf("     |  |  |_\n");
printf("     |  |  | )\n");
printf("     |  |  |/\n");
printf("         (J\n\n");
} else if (z == 6) {
printf("       __\n");
printf("  ___.'  '.___\n");
printf("  ____________\n\n");
} else if (z == 7) {
printf("   _\n");
printf("  ' `:--.--.\n");
printf("     |  |  |\n");
printf("     |  |  |\n");
printf("     |  |  |  ..,\n");
printf("           `---':\n\n");
} else if (z == 8) {
printf("      ...\n");
printf("      .':\n");
printf("    .'\n");
printf(" `..'\n");
printf(" .'.`\n\n");
} else if (z == 9) {
printf("       _\n");
printf(" \\     /_)\n");
printf("  \\    /`.\n");
printf("   \\  /   ;\n");
printf("    \\/ __.'\n\n");
} else if (z == 10) {
printf("  .-\"-._.-\"-._.-\n");
printf("  .-\"-._.-\"-._.-\n\n");
} else if (z == 11) {
printf("    `-.    .-'\n");
printf("       :  :\n");
printf("     --:--:--\n");
printf("       :  :\n");
printf("    .-'    `-.\n\n");
}
}
void fleshMound() {
currentArea = FLESH_MOUND;
showMap();

if (serpentKeyHalf) {
slowPrint("\nYou return to the Flesh Mound. The bone altar is silent.\n");
slowPrint("There is nothing else to claim here.\n");
return;
}

slowPrint("\n=== FLESH MOUND ===\n");
slowPrint("You approach the mound of bones, flesh, and scales.\n");
slowPrint("A massive shape coils along the temple roof...\n");
slowPrint("It watches you without eyes.\n");
slowPrint("Its body is but scales suspended in the air.\n");
slowPrint("They grind and shift, held together by a magnetic force.\n");
slowPrint("It is aware of your presence.\n");
slowPrint("Within the mound, half of a black crystal key is buried in a ribcage altar.\n");
slowPrint("The key is protected by the Hollow Serpent's magnetic field.\n");
slowPrint("Crystal pyramids surround the temple floor. Their cores hum when lightning passes near them.\n");
slowPrint("You must overload the crystal device binding the serpent together.\n");

while (1) {
playerHP = getPlayerMaxHP();
createEnemy(3);

while (playerHP > 0 && enemyHP > 0) {
combatMenu();
}

if (playerHP > 0) break;

slowPrint("\nYou were defeated at the Flesh Mound.\n");
if (!retryFromCheckpoint("Flesh Mound")) return;
}

slowPrint("\nThe Hollow Serpent's scales collapse as the magnetic field dies.\n");
slowPrint("The bone altar cracks open.\n");
slowPrint("You pull the black crystal key free. The bones collapse behind you.\n");
giveUpgrade();
serpentKeyHalf = 1;
updateLeviathanGateKey();
slowPrint("Flesh Mound Key Half acquired.\n");
}
void baitSerpentLightning() {
int serpentSign;
int oppositeSign;
int crystal1;
int crystal2;
int crystal3;
int correct;
int choice;

slowPrint("\nYou sprint between the crystal pyramids and draw the Hollow Serpent's attention.\n");
slowPrint("The serpent coils upward and releases an electrical breath discharge!\n");

if (barrierActive != 0) {
printf("The serpent's barrier absorbs the EMP surge!\n");
printf("The crystal pyramid fails to overload. Your turn is wasted!\n");
hollowSerpentTurn();
return;
}

serpentSign = rand() % 12;
oppositeSign = getOppositeZodiac(serpentSign);
correct = (rand() % 3) + 1;

crystal1 = rand() % 12;
crystal2 = rand() % 12;
crystal3 = rand() % 12;

if (correct == 1) crystal1 = oppositeSign;
else if (correct == 2) crystal2 = oppositeSign;
else crystal3 = oppositeSign;

printf("\nThe Hollow Serpent's lightning coils into the sign of %s!\n", getZodiacName(serpentSign));
printZodiacSymbol(serpentSign);

printf("Choose the opposite zodiac crystal to create polarity.\n");
printf("Opposite needed: %s\n\n", getZodiacName(oppositeSign));

printf("1. %s Crystal\n", getZodiacName(crystal1));
printZodiacSymbol(crystal1);

printf("2. %s Crystal\n", getZodiacName(crystal2));
printZodiacSymbol(crystal2);

printf("3. %s Crystal\n", getZodiacName(crystal3));
printZodiacSymbol(crystal3);

printf("Choice: ");
if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
choice = 0;
}

if (choice == correct) {
serpentCrystalCharge++;
printf("The opposite zodiac crystal creates polarity!\n");
printf("The lightning breath overloads the crystal pyramid!\n");
printf("Crystal Pyramid Charge: %d/%d\n", serpentCrystalCharge, serpentCrystalGoal);

if (serpentCrystalCharge >= serpentCrystalGoal) {
printf("\nThe crystal pyramids overload!\n");
printf("An EMP blast tears through the magnetic device holding the Hollow Serpent together!\n");
enemyHP = 0;
Sleep(1500);
return;
}
} else {
printf("The wrong zodiac crystal fails to create polarity!\n");
printf("The energy backfires through the arena!\n");
playerHP -= 10;
}

hollowSerpentTurn();
}
void hollowSerpentTurn() {
if (enemyHP <= 0) {
return;
}
slowPrint("\nThe Hollow Serpent releases a lightning breath discharge!\n");
playerHP -= enemyDamage;
printf("You took %d damage.\n", enemyDamage);
if (crystalTurns == 0) {
if (rand() % 100 < 30) {
crystalTurns = 3;
printf("The lightning carries crystallized venom into your blood!\n");
}
}
spawnBarrier();
Sleep(1200);
}
// -------------------- LEVIATHAN FINAL FIGHT --------------------
// -------------------- LEVIATHAN UNIVERSAL COMBAT MENU --------------------
void leviathanCombatMenu(int seerActive, int seerHP, int revealedZodiac, int obelisksBroken, int damagePhaseTurns) {
printf("\n=== COMBAT ===\n");
printf("Enemy: Leviathan\n");
printf("Leviathan HP: %d\n", enemyHP);
printf("Obelisks Broken: %d/3\n", obelisksBroken);
printf("Damage Phase Turns: %d\n", damagePhaseTurns);

if (seerActive) {
printf("Seer HP: %d\n", seerHP);
}

if (revealedZodiac >= 0) {
int oppositePlate = getOppositeZodiac(revealedZodiac);

printf("\nRevealed Obelisk Sign: %s\n", getZodiacName(revealedZodiac));
printZodiacSymbol(revealedZodiac);

printf("Opposite Plate Needed: %s\n", getZodiacName(oppositePlate));
printZodiacSymbol(oppositePlate);
}

printf("\nPlayer HP: %d\n", playerHP);
printf("Current Weapon: %s\n", getWeaponName(currentWeapon));
printf("Bandages: %d/2\n", bandages);

if (character == MASON) {
printf("HK M27 Ammo: %d\n", hkAmmo);
printf("M1 Garand Ammo: %d\n", garandAmmo);
} else {
printf("AK-105 Ammo: %d\n", akAmmo);
printf("ShAK-12 Ammo: %d\n", shakAmmo);
}

printf("\nStatuses:\n");
printf("Frostfire: %s\n", frostfireTurns > 0 ? "ACTIVE" : "NONE");
printf("Poison: %s\n", poisonTurns > 0 ? "ACTIVE" : "NONE");
printf("Crystallized Venom: %s\n", crystalTurns > 0 ? "ACTIVE" : "NONE");

if (enemyDodging && seerActive) {
printf("Seer Prediction: ACTIVE - next attack may miss\n");
}

printf("\n1. Attack Leviathan\n");
printf("2. Attack Seer\n");
printf("3. Stand on Zodiac Plate\n");
printf("4. Switch Weapon\n");
printf("5. Reload\n");
printf("6. Prepare\n");
printf("7. Show Map\n");
printf("8. Use Bandage\n");
printf("Choice: ");
}

void leviathanFight() {
int choice;

currentArea = LEVIATHAN_ARENA;
slowPrint("\n=== LEVIATHAN ARENA ===\n");
slowPrint("Three obelisks rise around the arena.\n");
slowPrint("Zodiac plates glow beneath your feet.\n");
slowPrint("The Leviathan descends from the blood-red sky.\n");

while (1) {
playerHP = getPlayerMaxHP();
int leviathanTurnCount = 0;
int seerActive = 0;
int seerHP = 0;
int revealedZodiac = -1;
int obelisksBroken = 0;
int damagePhaseTurns = 0;
int playerBracing = 0;

createEnemy(4);

while (playerHP > 0 && enemyHP > 0) {
int playerUsedTurn = 1;

clearScreen();
applyStatusEffects();
if (playerHP <= 0) break;

if (playerParalyzed) {
playerParalyzed = 0;
printf("You are frozen solid and lose your turn!\n");
}
else {
leviathanCombatMenu(seerActive, seerHP, revealedZodiac, obelisksBroken, damagePhaseTurns);

if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
printf("Invalid input.\n");
continuePrompt();
continue;
}

switch (choice) {
case 1: {
int damage = 0;
int rounds = 1;

if (damagePhaseTurns <= 0) {
printf("The Leviathan's obelisk armor absorbs your attack!\n");
break;
}

if (currentWeapon == HK_M27) {
printf("How many HK M27 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > hkAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
hkAmmo -= rounds;
fireWeapon(HK_M27, rounds);
damage = rounds * 5;
}
else if (currentWeapon == AK_105) {
printf("How many AK-105 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > akAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
akAmmo -= rounds;
fireWeapon(AK_105, rounds);
damage = rounds * 6;
}
else if (currentWeapon == M1_GARAND) {
if (garandAmmo <= 0) { printf("*PING* CLIP EJECTED! The Garand is empty!\n"); continuePrompt(); playerUsedTurn = 0; break; }
garandAmmo--;
fireWeapon(M1_GARAND, 1);
damage = 40;
}
else if (currentWeapon == SHAK_12) {
printf("How many ShAK-12 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > shakAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
shakAmmo -= rounds;
fireWeapon(SHAK_12, rounds);
damage = rounds * 18;
}

damage = CalculateDamage(damage, difficulty == HARD, 0);
enemyHP -= damage;
damagePhaseTurns--;
printf("You dealt %d damage to the exposed Leviathan.\n", damage);
break;
}

case 2: {
int damage = 0;
int rounds = 1;

if (!seerActive) {
printf("There is no Seer active.\n");
break;
}

if (currentWeapon == HK_M27) {
printf("How many HK M27 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > hkAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
hkAmmo -= rounds;
fireWeapon(HK_M27, rounds);
damage = rounds * 5;
}
else if (currentWeapon == AK_105) {
printf("How many AK-105 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > akAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
akAmmo -= rounds;
fireWeapon(AK_105, rounds);
damage = rounds * 6;
}
else if (currentWeapon == M1_GARAND) {
if (garandAmmo <= 0) { printf("*PING* CLIP EJECTED! The Garand is empty!\n"); continuePrompt(); playerUsedTurn = 0; break; }
garandAmmo--;
fireWeapon(M1_GARAND, 1);
damage = 40;
}
else if (currentWeapon == SHAK_12) {
printf("How many ShAK-12 rounds do you want to fire? ");
scanf("%d", &rounds);
if (rounds <= 0 || rounds > shakAmmo) { printf("Invalid ammo amount.\n"); continuePrompt(); playerUsedTurn = 0; break; }
shakAmmo -= rounds;
fireWeapon(SHAK_12, rounds);
damage = rounds * 18;
}

damage = CalculateDamage(damage, difficulty == HARD, 0);

if (enemyDodging) {
printf("The Seer dodged your attack! All bullets missed.\n");
enemyDodging = 0;
break;
}

seerHP -= damage;
printf("You dealt %d damage to the Seer.\n", damage);

if (seerHP <= 0) {
seerActive = 0;
enemyDodging = 0;
revealedZodiac = rand() % 12;
printf("The Seer shatters!\n");
printf("The Seers psionic energy reveals the sign of %s!\n", getZodiacName(revealedZodiac));
printf("\n=== REVEALED ZODIAC SYMBOL ===\n");
printf("%s\n", getZodiacName(revealedZodiac));
printZodiacSymbol(revealedZodiac);

printf("\nStudy this symbol. Press ENTER when ready to continue...");
while (getchar() != '\n');
}
break;
}

case 3:
if (seerActive) {
printf("The Seer hides the obelisk symbols. Kill it first!\n");
continuePrompt();
}
else if (revealedZodiac < 0) {
printf("No obelisk symbol is revealed yet.\n");
continuePrompt();
}
else {
int plate;
printf("\nChoose the zodiac plate to stand on:\n");
printf("The revealed obelisk sign is: %s\n", getZodiacName(revealedZodiac));
printZodiacSymbol(revealedZodiac);

for (int i = 0; i < 12; i++) {
printf("%d. %s Plate\n", i + 1, getZodiacName(i));
printZodiacSymbol(i);
}

printf("Choice: ");

if (scanf("%d", &plate) != 1) {
while (getchar() != '\n');
plate = 0;
}

int correctPlate = getOppositeZodiac(revealedZodiac);

printf("\nThe opposite zodiac sign is: %s\n",
getZodiacName(correctPlate));

if (plate - 1 == correctPlate) {
obelisksBroken++;
revealedZodiac = -1;

printf("The opposite plate erupts with light!\n");
printf("The Obelisk shatters! [%d/3]\n", obelisksBroken);

if (obelisksBroken >= 3) {
obelisksBroken = 0;
damagePhaseTurns = 2;

printf("\nAll three obelisks are broken!\n");
printf("The Leviathan's armor opens for 2 turns!\n");
}
}
else {
printf("The wrong zodiac polarity burns your body!\n");
playerHP -= 15;
}
}
break;

case 4:
switchWeapon();
playerUsedTurn = 0;
break;

case 5:
reloadWeapon();
playerUsedTurn = 0;
break;

case 6:
playerBracing = 1;
printf("You brace yourself for the next attack.\n");
break;

case 7:
showMap();
playerUsedTurn = 0;
break;

case 8:
if (!useBandage()) playerUsedTurn = 0;
break;

default:
printf("Invalid choice.\n");
continuePrompt();
playerUsedTurn = 0;
break;
}
}

if (!playerUsedTurn || playerHP <= 0 || enemyHP <= 0) {
continuePrompt();
continue;
}

if (seerActive) {
int damage = 15;
printf("\nThe Seer lashes out with psychic force!\n");
if (playerBracing) {
damage /= 2;
playerBracing = 0;
printf("You brace against the Seer's attack!\n");
}
if (scaleUpgrade) damage = (damage * 8) / 10;
playerHP -= damage;
printf("You took %d damage from the Seer.\n", damage);

if (rand() % 100 < 20) {
enemyDodging = 1;
printf("The Seer foresees your next move...\n");
}

continuePrompt();
continue;
}

leviathanTurnCount++;

if (damagePhaseTurns == 0 && rand() % 100 < 25) {
slowPrint("\nThe Leviathan floods the arena with poison liquid!\n");

if (difficulty == EASY) {
slowPrint("The toxic liquid rises briefly, then recedes as ancient channels open beneath the arena.\n");
} else {
slowPrint("The liquid rises rapidly. The drainage pressure is failing!\n");
poisonPuzzleMode = 1;
pressureEqualizerPuzzle();
poisonPuzzleMode = 0;
}

continuePrompt();
continue;
}

if (leviathanTurnCount % 2 == 0 && !seerActive && revealedZodiac < 0 && damagePhaseTurns == 0) {
seerActive = 1;
seerHP = 100;
enemyDodging = 0;
printf("\nThe Leviathan summons a Seer!\n");
continuePrompt();
continue;
}

{
int damage = enemyDamage;
printf("\nThe Leviathan releases a frostfire breath attack!\n");

if (playerBracing) {
damage /= 2;
playerBracing = 0;
printf("You brace against the blast!\n");
}

if (scaleUpgrade) damage = (damage * 8) / 10;
playerHP -= damage;
printf("You took %d damage.\n", damage);

if (frostfireTurns == 0 && rand() % 100 < 25) {
frostfireTurns = 3;
printf("The Leviathan has engulfed you in frostfire!\n");
printf("It will burn for 3 turns, then freeze you.\n");
}
}

continuePrompt();
}

if (playerHP <= 0) {
slowPrint("\nYou were destroyed by the Leviathan.\n");
if (retryFromCheckpoint("Leviathan Arena")) continue;
return;
}

slowPrint("\nThe Leviathan collapses into the arena floor.\n");
slowPrint("The blood-red star fades from the sky.\n");
slowPrint("You have survived the alien world.\n");
slowPrint("Your vision blurs as the alien world fades away.\n");
slowPrint("Moments later, you awaken inside your ship.\n");
slowPrint("All systems stabilize. The anomaly is gone.\n");
printf("\n");
printf(" \\     / (_)     | |                   \n");
printf("  \\   /   _  ___| |_ ___  _ __ _   _  \n");
printf("   \\ /    | |/ __| __/ _ \\| '__| | | | \n");
printf("    \\     | | (__| || (_) | |  | |_| | \n");
printf("     \\    |_|\\___|\\__\\___/|_|   \\__, | \n");
printf("                                 __/ | \n");
printf("                                |___/  \n");
return;
}
}
// -------------------- BANDAGES --------------------
int getPlayerMaxHP() {
if (mosasaurUpgrade || currentArea == LEVIATHAN_ARENA || playerHP > 100) {
return 200;
}
return 100;
}
int useBandage() {
if (bandages <= 0) {
printf("No bandages left for this enemy.\n");
return 0;
}

if (playerHP >= getPlayerMaxHP()) {
printf("You are already at full health.\n");
return 0;
}

bandages--;
playerHP = getPlayerMaxHP();
slowPrint("You use a bandage and restore to full health.\n");
Sleep(2000);
printf("Bandages remaining: %d/2\n", bandages);
return 1;
}
// -------------------- CHECKPOINTS --------------------
void resetCheckpointState() {
playerHP = getPlayerMaxHP();
bandages = 2;

crystalTurns = 0;
burnTurns = 0;
poisonTurns = 0;
frostfireTurns = 0;
enemyCrystalTurns = 0;
playerParalyzed = 0;

poisonDamage = 10;
barrierActive = 0;
barrierCooldown = 0;
enemyDodging = 0;

hkAmmo = 40;
garandAmmo = 8;
akAmmo = 30;
shakAmmo = 10;
}
int retryFromCheckpoint(const char *encounterName) {
int retry;

printf("\nRestart from checkpoint before %s?\n", encounterName);
printf("1. Yes\n2. No\nChoice: ");

if (scanf("%d", &retry) != 1) {
while (getchar() != '\n');
retry = 2;
}

if (retry == 1) {
resetCheckpointState();
printf("\nCheckpoint restored.\n");
Sleep(1000);
clearScreen();
return 1;
}

return 0;
}
// -------------------- MENU --------------------
void combatMenu() {
int choice;

clearScreen();

applyStatusEffects();
applyEnemyStatusEffects();

if (enemyType == 2 && enemyHP <= 0 && prismReforms > 0) {
prismReforms--;
prismPhase++;
enemyHP = 50;
barrierActive = 0;
barrierCooldown = 0;
enemyDodging = 0;
enemyCrystalTurns = 0;

printf("\nThe Prism Stalker shatters into fragments!\n");
printf("The shards vibrate, pull together, and reform into a harder body.\n");
printf("Reforms remaining: %d\n", prismReforms);
continuePrompt();
return;
}

if (playerHP <= 0 || enemyHP <= 0) return;

if (playerParalyzed) {
playerParalyzed = 0;
printf("You are frozen solid and lose your turn!\n");
enemyTurn();
continuePrompt();
return;
}

printf("\n=== COMBAT ===\n");

if (enemyType == 2) {
printf("Enemy: Prism Stalker\n");
printf("Enemy HP: %d\n", enemyHP);
printf("Reforms Remaining: %d\n", prismReforms);
printf("Prism Phase: %d\n", prismPhase);
}
else if (enemyType == 3) {
printf("Enemy: Hollow Serpent\n");
printf("Crystal Pyramid Charge: %d/%d\n", serpentCrystalCharge, serpentCrystalGoal);
}
else {
printf("Enemy HP: %d\n", enemyHP);
}

printf("\nPlayer HP: %d\n", playerHP);
printf("Current Weapon: %s\n", getWeaponName(currentWeapon));
printf("Bandages: %d/2\n", bandages);

if (character == MASON) {
printf("HK M27 Ammo: %d\n", hkAmmo);
printf("M1 Garand Ammo: %d\n", garandAmmo);
} else {
printf("AK-105 Ammo: %d\n", akAmmo);
printf("ShAK-12 Ammo: %d\n", shakAmmo);
}

printf("\nStatuses:\n");
printf("Burn: %s\n", burnTurns > 0 ? "ACTIVE" : "NONE");
printf("Poison: %s\n", poisonTurns > 0 ? "ACTIVE" : "NONE");
printf("Frostfire: %s\n", frostfireTurns > 0 ? "ACTIVE" : "NONE");
printf("Crystallized Venom: %s\n", crystalTurns > 0 ? "ACTIVE" : "NONE");
printf("Enemy Crystallized: %s\n", enemyCrystalTurns > 0 ? "YES" : "NO");

if (enemyDodging) {
printf("Afterimages: ACTIVE - next attack may miss\n");
}

barrierInfo();

if (enemyType == 2 && (prismTurnCount + 1) % 4 == 0) {
slowPrint("The crystal air begins to split into bright and dark bands...\n");
}

printf("\n1. Attack\n");
printf("2. Switch Weapon\n");
printf("3. Reload\n");
printf("4. Use Bandage\n");
printf("5. Show Map\n");

if (enemyType == 3) {
printf("6. Bait lightning into crystal pyramid\n");
}

printf("Choice: ");

if (scanf("%d", &choice) != 1) {
while (getchar() != '\n');
printf("Invalid input.\n");
Sleep(700);
return;
}

switch (choice) {
case 1:
attackEnemy();
continuePrompt();
break;

case 2:
switchWeapon();
continuePrompt();
break;

case 3:
reloadWeapon();
continuePrompt();
break;

case 4:
if (useBandage()) enemyTurn();
continuePrompt();
break;

case 5:
showMap();
break;

case 6:
if (enemyType == 3) {
baitSerpentLightning();
} else {
printf("Invalid choice.\n");
}
continuePrompt();
break;

default:
printf("Invalid choice.\n");
continuePrompt();
break;
}
}
// -------------------- NAMES --------------------
const char* getCharacterName() {
if (character == IVAN) {
return "Ivan";
}

return "Mason";
}
const char* getWeaponName(int weapon) {
if (weapon == HK_M27) return "HK M27";
if (weapon == M1_GARAND) return "M1 Garand";
if (weapon == AK_105) return "AK-105";
if (weapon == SHAK_12) return "ShAK-12";

return "Unknown Weapon";
}
// -------------------- MAP --------------------
void showMap() {
printf("\n============================== MAP ==============================\n\n");

printf("                    [Ocean Floor Discovery] %s\n",
currentArea == OCEAN_FLOOR ? "<-- YOU ARE HERE" : "");
printf("                    Machinery: %s\n", machineryInvestigated ? "COMPLETE" : "UNKNOWN");
printf("                    Obelisks:  %s\n", obelisksScanned ? "COMPLETE" : "UNKNOWN");

printf("                              |\n");
printf("                              v\n");

printf("                       [Mosasaur Arena] %s\n",
currentArea == MOSASAUR_ARENA ? "<-- YOU ARE HERE" : "");
printf("                       Mosasaur: %s\n",
mosasaurDefeated ? "DEFEATED" : "GUARDING PYRAMID");

printf("                              |\n");
printf("                              v\n");

printf("                       [Pyramid Chamber] %s\n",
currentArea == PYRAMID_CHAMBER ? "<-- YOU ARE HERE" : "");

printf("                              |\n");
printf("                              v\n");

printf("                      [Crystal Interior] %s\n",
currentArea == CRYSTAL_INTERIOR ? "<-- YOU ARE HERE" : "");
printf("                      Pyramid: %s\n",
pyramidOpened ? "OPENED" : "LOCKED");

printf("                              |\n");
printf("                              v\n");

printf("                    [Alien World Crossroads]\n");
printf("                         /              \\\n");
printf("                        v                v\n");

printf("              [Crystal Spires]    [Flesh Mound]\n");

printf("              %-18s %-18s\n",
currentArea == CRYSTAL_SPIRES ? "<-- YOU ARE HERE" : "",
currentArea == FLESH_MOUND ? "<-- YOU ARE HERE" : "");

printf("              Key Half: %-8s Key Half: %-8s\n",
crystalKeyHalf ? "ACQUIRED" : "MISSING",
serpentKeyHalf ? "ACQUIRED" : "MISSING");

printf("                         \\              /\n");
printf("                          v            v\n");

printf("                       [Leviathan Arena] %s\n",
currentArea == LEVIATHAN_ARENA ? "<-- YOU ARE HERE" : "");

printf("                       Gate Key: %s\n",
leviathanGateKey ? "COMPLETE" : "INCOMPLETE");

printf("\n=================================================================\n");
printf("\nPress ENTER to return...");
while (getchar() != '\n');
}
// -------------------- UPGRADES --------------------
void giveUpgrade() {
if (enemyType == 1 && !mosasaurUpgrade) {
mosasaurUpgrade = 1;
printf("You gained the Mosasaur Core! Increased max HP.\n");
playerHP += 100;
}
else if (enemyType == 2 && !prismUpgrade) {
prismUpgrade = 1;
printf("You gained the Prism Lens! Headshot chance increased, attacks can crystallize enemies, and bullets may ricochet into a second hit.\n");
}
else if (enemyType == 3 && !scaleUpgrade) {
scaleUpgrade = 1;
printf("You gained scale Plating! Reduced incoming damage by 20%%.\n");
}
else{
printf("no upgrade\n");
}
}

void barrierInfo() {
if (barrierActive == 0) {
printf("Barrier: NONE\n");
}
else if (barrierActive == 1) {
slowPrint("Barrier: ACTIVE - Light rifle required\n");
}
else if (barrierActive == 2) {
slowPrint("Barrier: ACTIVE - Heavy weapon required\n");
}
else if (barrierActive == 3) {
printf("Barrier: REFLECTIVE - shoot it to break stance, bullets ricochet back\n");
}
}
// -------------------- ENEMY TYPES --------------------
void createEnemy(int type) {
enemyType = type;

garandAmmo = 8;
hkAmmo = 40;
akAmmo = 30;
shakAmmo = 10;
enemyCrystalTurns = 0;
bandages = 2;
barrierActive = 0;
barrierCooldown = 0;
enemyDodging = 0;

if (enemyType == 1) {
// Mosasaur
enemyHP = (difficulty == EASY) ? 500 : 1000;
enemyDamage = 20;
torpedoes = 6;
depthCharges = 4;
sonarDisrupted = 0;
mosasaurTurnCount = 0;
}
else if (enemyType == 2) {
// Prism Stalker
enemyHP = 50;
enemyDamage = 20;
prismTurnCount = 0;
prismReforms = 3;
prismPhase = 0;
}
else if (enemyType == 3) {
// Hollow Serpent
enemyHP = 200;
enemyDamage = 10;
serpentCrystalCharge = 0;
}
else if (enemyType == 4) {
// Leviathan
enemyHP = 1000;
enemyDamage = 30;
}
else if (enemyType == 5) {
// Seer
enemyHP = 100;
enemyDamage = 15;
}
else {
enemyHP = 100;
enemyDamage = 10;
}
}
// -------------------- SWITCH WEAPON --------------------
void switchWeapon() {
if (character == MASON) {
if (currentWeapon == HK_M27) {
currentWeapon = M1_GARAND;
} else {
currentWeapon = HK_M27;
}
}
else if (character == IVAN) {
if (currentWeapon == AK_105) {
currentWeapon = SHAK_12;
} else {
currentWeapon = AK_105;
}
}

printf("Switched to %s.\n", getWeaponName(currentWeapon));
}
// -------------------- BARRIER --------------------
int handleBarrierWithCurrentWeapon(int baseDamage) {
if (barrierActive == 0) return 1;
// Barrier that is destroyed by light weapons otherwise, enemy remains immune
if (barrierActive == 1 && (currentWeapon == HK_M27 || currentWeapon == AK_105)) {
slowPrint("\nYour light rifle tears through the ballistic barrier!\n");
barrierActive = 0;
barrierCooldown = 1;
Sleep(1200);
return 1;
}
// Barrier that is destroyed by heavy weapons otherwise, enemy remains immune
if (barrierActive == 2 && (currentWeapon == M1_GARAND || currentWeapon == SHAK_12)) {
slowPrint("\nYour heavy weapon shatters the reinforced barrier!\n");
barrierActive = 0;
barrierCooldown = 1;
Sleep(1200);
return 1;
}
// Emergency barrier break if out of heavy ammo
if (garandAmmo == 0 && shakAmmo == 0) {
slowPrint("\nYou are out of heavy ammo!\n");
slowPrint("You force your way through the reinforced barrier!\n");
playerHP -= 15;
slowPrint("The barrier shatters, but you take 15 damage!\n");
barrierActive = 0;
barrierCooldown = 1;
Sleep(1500);
return 1;
}
// Special reflective barrier that breaks with any weapon but you take the 1/2 same damage you dealt to the barrier
if (barrierActive == 3 && enemyType == 2) {
slowPrint("\nYour bullets ricochet off the Prism Stalker's reflective barrier!\n");
slowPrint("The stance breaks, but the ricochet cuts back into you!\n");
playerHP -= baseDamage / 2;
barrierActive = 0;
barrierCooldown = 1;
Sleep(1500);
return 1;
}
// if wrong weapon is used
printf("\n%s is ineffective against this barrier!\n", getWeaponName(currentWeapon));
slowPrint("The barrier absorbs the attack. No damage gets through.\n");
Sleep(1500);
return 0;
}
void spawnBarrier() {
// hard difficulty spawns barriers
if (difficulty != HARD || barrierActive != 0) return;
// Enemy has to wait at least 1 turn after player breaks barrier
if (barrierCooldown > 0) {
barrierCooldown--;
return;
}
// no barrier at all for mosasaur
if (enemyType == 1) return;
// 25% chance for a barrier to spawn
if (rand() % 100 < 25) {
if (enemyType == 2) {
barrierActive = (rand() % 3) + 1;
} else {
barrierActive = (rand() % 2) + 1;
}

if (barrierActive == 1) {
slowPrint("\nEnemy deployed a ballistic barrier!\n");
slowPrint("Use a light rifle: HK M27 or AK-105.\n");
}
else if (barrierActive == 2) {
slowPrint("\nEnemy deployed a reinforced barrier!\n");
slowPrint("Use a heavy weapon: M1 Garand or ShAK-12.\n");
}
else if (barrierActive == 3) {
slowPrint("\nThe Prism Stalker forms a reflective barrier!\n");
slowPrint("Bullets will ricochet back.\n");
}

Sleep(1800);
}
}

// -------------------- UNIFIED WEAPON FIRE --------------------
// -------------------- ATTACK ENEMY --------------------
void attackEnemy() {
int baseDamage = 0;
int headshot = 0;
int shotsFired = 0;

if (currentWeapon == HK_M27) {
int rounds;

printf("How many HK M27 rounds do you want to fire? ");
scanf("%d", &rounds);

if (hkAmmo <= 0) {
printf("HK M27 is empty! You need to reload!\n");
return;
}

if (rounds <= 0 || rounds > hkAmmo) {
printf("Invalid ammo amount.\n");
return;
}

hkAmmo -= rounds;
shotsFired = rounds;
fireWeapon(HK_M27, rounds);

baseDamage = rounds * 5;

if (rand() % 100 < (prismUpgrade ? 25 : 15)) {
headshot = 1;
}
}

else if (currentWeapon == M1_GARAND) {
if (garandAmmo <= 0) {
printf("*PING* CLIP EJECTED! The Garand is empty!\n");
printf("Reload or switch weapons!\n");
return;
}

garandAmmo--;
shotsFired = 1;
fireWeapon(M1_GARAND, 1);

baseDamage = 100;

if (rand() % 100 < 45) {
headshot = 1;
}
}

else if (currentWeapon == AK_105) {
int rounds;

printf("How many AK-105 rounds do you want to fire? ");
scanf("%d", &rounds);

if (akAmmo <= 0) {
printf("AK-105 is empty! You need to reload!\n");
return;
}

if (rounds <= 0 || rounds > akAmmo) {
printf("Invalid ammo amount.\n");
return;
}

akAmmo -= rounds;
shotsFired = rounds;
fireWeapon(AK_105, rounds);

baseDamage = rounds * 6;

if (rand() % 100 < (prismUpgrade ? 25 : 15)) {
headshot = 1;
}
}

else if (currentWeapon == SHAK_12) {
int rounds;

printf("How many ShAK-12 rounds do you want to fire? ");
scanf("%d", &rounds);

if (shakAmmo <= 0) {
printf("ShAK-12 is empty! You need to reload!\n");
return;
}

if (rounds <= 0 || rounds > shakAmmo) {
printf("Invalid ammo amount.\n");
return;
}

shakAmmo -= rounds;
shotsFired = rounds;
fireWeapon(SHAK_12, rounds);

baseDamage = rounds * 50;

if (rounds <= 2) {
if (rand() % 100 < 30) {
headshot = 1;
}
} else {
int missed = rounds / 2;
baseDamage -= missed * 18;

printf("Recoil is uncontrollable! %d rounds missed!\n", missed);

if (rand() % 100 < 10) {
headshot = 1;
}
}
}

if (enemyDodging) {
if (enemyType == 2) printf("The Prism Stalker's afterimages scatter your attack! All bullets missed.\n");
else slowPrint("The Seer foresees your next move...\nThe Seer dodged your attack! All bullets missed.\n");
enemyDodging = 0;
enemyTurn();
return;
}

if (barrierActive != 0) {
handleBarrierWithCurrentWeapon(baseDamage);
enemyTurn();
return;
}

if (enemyType == 3) {
printf("Your bullets pass through the Hollow Serpent harmlessly!\n");
printf("Its body is hollow scales held together by magnetism. Direct attacks do nothing.\n");
if (enemyHP > 0) {
enemyTurn();
}
return;
}

int armorActive = (difficulty == HARD);
int damage = CalculateDamage(baseDamage, armorActive, headshot);

if (enemyType == 2 && prismPhase > 0) {
int reduction = prismPhase * 20;
damage = damage * (100 - reduction) / 100;
if (damage < 1) damage = 1;
printf("The reformed prism body reduces damage by %d%%!\n", reduction);
}

if (enemyCrystalTurns > 0) {
damage = (damage * 13) / 10;
printf("Crystallization amplifies your damage!\n");
}

if (prismUpgrade) {
int bounceChance = GetBulletBounceChance(currentWeapon);

if (rand() % 100 < bounceChance) {
damage *= 2;
printf("Prism ricochet! The bullet bounces and hits the enemy twice.\n");
}
}

if (headshot) {
printf("HEADSHOT!\n");
}

enemyHP -= damage;

printf("Impact! (%d rounds fired)\n", shotsFired);
printf("You dealt %d damage.\n", damage);
Sleep(2000);


if (enemyType == 2 && enemyHP <= 0 && prismReforms > 0) {
prismReforms--;
prismPhase++;
enemyHP = 50;
barrierActive = 0;
barrierCooldown = 0;
enemyDodging = 0;
enemyCrystalTurns = 0;

printf("\nThe Prism Stalker shatters into fragments!\n");
printf("The shards vibrate, pull together, and reform into a harder body.\n");
printf("Reforms remaining: %d\n", prismReforms);
return;
}

if (prismUpgrade && enemyCrystalTurns == 0) {
if (rand() % 100 < 25) {
enemyCrystalTurns = 3;
printf("Prism shards crystallize the enemy! They will take damage over time and suffer increased damage.\n");
}
}

if (enemyHP > 0) {
enemyTurn();
}
}
// -------------------- FIRE GUNS --------------------
void fireWeapon(int weapon, int shots) {
const char *name = getWeaponName(weapon);

for (int s = 0; s < shots; s++) {
clearScreen();
printf("Firing %s:\n\n", name);

if (weapon == HK_M27 || weapon == AK_105) {
printf("      )\\\\\n");
printf("     )  \\\\\n");
printf("====#=====>\n");
printf("     )  /\n");
printf("      )/\n");
} else {
printf("        ))))\\\\\n");
printf("     )))     \\\\\n");
printf("===###########====>\n");
printf("     )))     /\n");
printf("        ))))/\n");
}

Sleep(35);

clearScreen();
printf("Firing %s:\n\n", name);

if (weapon == HK_M27 || weapon == AK_105) {
printf("       )\\\\\n");
printf("    )))  \\\\\n");
printf("===###=====>\n");
printf("    )))  /\n");
printf("       )/\n");
} else {
printf("       ))))))\\\\\n");
printf("    ))))      \\\\\n");
printf("==#############===>\n");
printf("    ))))      /\n");
printf("       ))))))/\n");
}

Sleep(35);
}
}
// -------------------- RELOAD WEAPON --------------------
void reloadWeapon() {

if (currentWeapon == HK_M27) {
if (hkAmmo == 40) {
printf("HK M27 already full.\n");
return;
}

hkAmmo = 40;
printf("Reloaded HK M27.\n");
}

else if (currentWeapon == AK_105) {
if (akAmmo == 30) {
printf("AK-105 already full.\n");
return;
}

akAmmo = 30;
printf("Reloaded AK-105.\n");
}

else if (currentWeapon == M1_GARAND) {
printf("The M1 Garand cannot be reloaded in combat!\n");
printf("Use your shots carefully.\n");
return;
}

else if (currentWeapon == SHAK_12) {
printf("The ShAK-12 is too heavy to reload in combat!\n");
printf("Use controlled bursts or switch weapons.\n");
return;
}

enemyTurn();
}
// -------------------- ENEMY TURN --------------------
void enemyTurn() {
printf("\nEnemy attacks!\n");

if (enemyType == 2) {
prismStalkerTurn();
return;
}

int damageTaken = enemyDamage;

if (scaleUpgrade) {
damageTaken = (damageTaken * 8) / 10;
}

playerHP -= damageTaken;
printf("You took %d damage.\n", damageTaken);

if (enemyType == 3) {
if (crystalTurns == 0) {
if (rand() % 100 < 30) {
crystalTurns = 3;
printf("The venom crystallizes your blood!\n");
}
}
}

else if (enemyType == 4) {
if (frostfireTurns == 0) {
if (rand() % 100 < 25) {
frostfireTurns = 3;
printf("The Leviathan has engulfed you in frostfire!\n");
}
}
}

else if (enemyType == 5) {
if (rand() % 100 < 20) {
enemyDodging = 1;
}
}

spawnBarrier();
}
// -------------------- PLAYER STATUS EFFECTS --------------------
void applyStatusEffects() {
if (crystalTurns > 0) {
playerHP -= crystalDamage;
crystalTurns--;
printf("Crystals grow inside you! You take %d damage.\n", crystalDamage);
}

if (burnTurns > 0) {
playerHP -= burnDamage;
burnTurns--;
printf("Burning light scorches you! You take %d damage.\n", burnDamage);
}

if (frostfireTurns > 0) {
playerHP -= frostfireDamage;
frostfireTurns--;
printf("Frostfire burns through your body! You take %d damage.\n", frostfireDamage);

if (frostfireTurns == 0) {
playerParalyzed = 1;
printf("The frostfire suddenly freezes solid around your body!\n");
printf("You will lose your next turn!\n");
}
}

if (poisonTurns > 0) {
playerHP -= poisonDamage;
poisonTurns--;
printf("Poison hurts you! You take %d damage.\n", poisonDamage);
}
}
// -------------------- ENEMY STATUS EFFECTS --------------------
void applyEnemyStatusEffects() {
if (enemyCrystalTurns > 0) {
enemyHP -= enemyCrystalDamage;
enemyCrystalTurns--;
printf("Crystals tear through the enemy! They take %d damage.\n", enemyCrystalDamage);
}
}

/* USER CODE END 0 */

/**
* @brief  The application entry point.
* @retval int
*/
int main(void)
{
HAL_Init();
SystemClock_Config();

MX_GPIO_Init();
MX_USART2_UART_Init();
MX_DMA_Init();

/* USER CODE BEGIN 2 */
Game_Run();
/* USER CODE END 2 */

while (1)
{
}
}

/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void)
{
RCC_OscInitTypeDef RCC_OscInitStruct = {0};
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
{
Error_Handler();
}

RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
RCC_OscInitStruct.HSIState = RCC_HSI_ON;
RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
RCC_OscInitStruct.PLL.PLLM = 1;
RCC_OscInitStruct.PLL.PLLN = 10;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
Error_Handler();
}

RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
{
Error_Handler();
}
}

/**
* @brief USART2 Initialization Function
* @param None
* @retval None
*/
static void MX_USART2_UART_Init(void)
{
huart2.Instance = USART2;
huart2.Init.BaudRate = 115200;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
huart2.Init.OverSampling = UART_OVERSAMPLING_16;
huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

if (HAL_UART_Init(&huart2) != HAL_OK)
{
Error_Handler();
}
}

/**
* Enable DMA controller clock
*/
static void MX_DMA_Init(void)
{
__HAL_RCC_DMA1_CLK_ENABLE();

HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);

HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

/**
* @brief GPIO Initialization Function
* @param None
* @retval None
*/
static void MX_GPIO_Init(void)
{
GPIO_InitTypeDef GPIO_InitStruct = {0};

__HAL_RCC_GPIOC_CLK_ENABLE();
__HAL_RCC_GPIOH_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOB_CLK_ENABLE();

#ifdef LD2_GPIO_Port
HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
#endif

#ifdef B1_Pin
GPIO_InitStruct.Pin = B1_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef LD2_Pin
GPIO_InitStruct.Pin = LD2_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
#endif
}

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
__disable_irq();

while (1)
{
	 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Onboard Green LED
	    HAL_UART_Transmit(&huart2, (uint8_t*)"Alive\r\n", 7, 100);
	    HAL_Delay(500);
}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
