#include <Keypad.h>                // Include the library for keypad
#include <SD.h>                    // Include the library for SD card module
#include <SPI.h>                   // Include the library for SPI communication (used by SD card)
#include <Wire.h>                  // Include the library for I2C communication (used by LCD)
#include <LiquidCrystal_I2C.h>     // Include the library for the LCD display with I2C interface

#define CS_PIN 10                  // Define the Chip Select pin for the SD card module
#define CHANNEL_1_PIN A0           // Define the pin for Channel 1 (relay or other device)
#define CHANNEL_2_PIN A1           // Define the pin for Channel 2 (relay or other device)
#define MAX_USERS 5                // Define the maximum number of users allowed

// Define keypad configuration
const byte ROWS = 4;               // Define the number of rows on the keypad
const byte COLS = 4;               // Define the number of columns on the keypad
char keys[ROWS][COLS] = {          // Map the keypad buttons to their corresponding characters
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // Define the row pins connected to the keypad
byte colPins[COLS] = {5, 4, 3, 2}; // Define the column pins connected to the keypad

// Create instances of keypad and LCD
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // Create a Keypad object
LiquidCrystal_I2C lcd(0x27, 16, 2); // Create a LiquidCrystal_I2C object with I2C address 0x27, 16x2 display

// Define user data structure
struct User {
  char password[5];               // Store user password (4 characters + null terminator)
};
User users[MAX_USERS];            // Create an array of users
int userCount = 0;                // Initialize the user count to 0
char adminPass[5] = "1234";       // Define the default admin password

void setup() {
  // Setup channel pins as outputs and set them to LOW (off)
  pinMode(CHANNEL_1_PIN, OUTPUT);  // Set CHANNEL_1_PIN as output
  pinMode(CHANNEL_2_PIN, OUTPUT);  // Set CHANNEL_2_PIN as output
  digitalWrite(CHANNEL_1_PIN, LOW); // Turn off Channel 1
  digitalWrite(CHANNEL_2_PIN, LOW); // Turn off Channel 2

  // Initialize LCD display
  lcd.init();                      // Initialize the LCD
  lcd.backlight();                 // Turn on the LCD backlight

  // Initialize SD card module and check if initialization is successful
  if (!SD.begin(CS_PIN)) {          // Begin communication with SD card module
    lcd.clear();                    // Clear the LCD
    lcd.print("SD Init Failed");    // Print error message if SD card initialization fails
    while (true);                   // Halt execution if SD card fails to initialize
  }

  // Display a welcome message
  lcd.clear();                      // Clear the LCD
  lcd.print("Welcome!");            // Print welcome message
  delay(2000);                      // Wait for 2 seconds

  loadUsersFromSD();                // Load users from SD card
  mainMenu();                       // Display the main menu
}

void mainMenu() {
  lcd.clear();                      // Clear the LCD
  lcd.print("1. Add User");         // Print the option to add a new user
  lcd.setCursor(0, 1);              // Set the cursor to the beginning of the second row
  lcd.print("2. Login");            // Print the option to log in

  char choice = waitForKey();       // Wait for a keypress and store the pressed key
  if (choice == '1') {              // If '1' is pressed
    addUser();                      // Call addUser function
  } else if (choice == '2') {       // If '2' is pressed
    login();                        // Call login function
  } else if (choice == 'A') {       // If 'A' is pressed
    changeAdminPass();              // Call changeAdminPass function
  } else if (choice == 'B') {       // If 'B' is pressed
    removeUser();                   // Call removeUser function
  }
}

void addUser() {
  lcd.clear();                      // Clear the LCD
  lcd.print("Enter Admin:");        // Prompt to enter admin password

  if (!validatePassword(adminPass)) { // If the entered password is incorrect
    lcd.clear();                    // Clear the LCD
    lcd.print("Wrong Pass");        // Display wrong password message
    delay(2000);                    // Wait for 2 seconds
    mainMenu();                     // Return to the main menu
    return;                         // Exit the function
  }

  if (userCount >= MAX_USERS) {     // If the user count has reached the maximum
    lcd.clear();                    // Clear the LCD
    lcd.print("Max Users");         // Display max users message
    delay(2000);                    // Wait for 2 seconds
    mainMenu();                     // Return to the main menu
    return;                         // Exit the function
  }

  lcd.clear();                      // Clear the LCD
  lcd.print("New Pass:");           // Prompt to enter a new password
  strcpy(users[userCount].password, inputPassword()); // Store the entered password for the new user
  userCount++;                      // Increment the user count
  saveUsersToSD();                  // Save the updated user list to the SD card

  lcd.clear();                      // Clear the LCD
  lcd.print("User Added!");         // Display user added message
  delay(2000);                      // Wait for 2 seconds

  mainMenu();                       // Return to the main menu
}

void removeUser() {
  lcd.clear();                      // Clear the LCD
  lcd.print("Enter Admin:");        // Prompt to enter admin password

  if (!validatePassword(adminPass)) { // If the entered password is incorrect
    lcd.clear();                    // Clear the LCD
    lcd.print("Wrong Pass");        // Display wrong password message
    delay(2000);                    // Wait for 2 seconds
    mainMenu();                     // Return to the main menu
    return;                         // Exit the function
  }

  if (userCount == 0) {             // If there are no users to remove
    lcd.clear();                    // Clear the LCD
    lcd.print("No Users");          // Display no users message
    delay(2000);                    // Wait for 2 seconds
    mainMenu();                     // Return to the main menu
    return;                         // Exit the function
  }

  lcd.clear();                      // Clear the LCD
  lcd.print("Select User:");        // Prompt to select a user to remove

  for (int i = 0; i < userCount; i++) { // Loop through each user
    lcd.setCursor(0, i % 2);        // Set the cursor position
    lcd.print(i + 1);               // Print the user number
    lcd.print(". USER");            // Print "USER"
    lcd.print(i + 1);               // Print the user number
    if (i % 2 == 1) delay(2000);    // If the cursor is on the second line, wait for 2 seconds
  }

  char choice = waitForKey();       // Wait for a keypress and store the pressed key
  int index = choice - '1';         // Convert the keypress to an index for the users array

  if (index >= 0 && index < userCount) { // If the selected user is valid
    for (int i = index; i < userCount - 1; i++) { // Shift the user list to remove the selected user
      users[i] = users[i + 1];       // Move the next user into the current position
    }
    userCount--;                    // Decrement the user count
    saveUsersToSD();                // Save the updated user list to the SD card

    lcd.clear();                    // Clear the LCD
    lcd.print("User Removed!");     // Display user removed message
    delay(2000);                    // Wait for 2 seconds
  } else {
    lcd.clear();                    // Clear the LCD
    lcd.print("Invalid User");      // Display invalid user message
    delay(2000);                    // Wait for 2 seconds
  }

  mainMenu();                       // Return to the main menu
}

void login() {
  lcd.clear();                      // Clear the LCD
  lcd.print("Select User:");        // Prompt to select a user for login

  for (int i = 0; i < userCount; i++) { // Loop through each user
    lcd.setCursor(0, i % 2);        // Set the cursor position
    lcd.print(i + 1);               // Print the user number
    lcd.print(". USER");            // Print "USER"
    lcd.print(i + 1);               // Print the user number
    if (i % 2 == 1) delay(2000);    // If the cursor is on the second line, wait for 2 seconds
  }

  char choice = waitForKey();       // Wait for a keypress and store the pressed key
  int index = choice - '1';         // Convert the keypress to an index for the users array

  if (index >= 0 && index < userCount) { // If the selected user is valid
    lcd.clear();                    // Clear the LCD
    lcd.print("Enter Pass:");       // Prompt to enter the user's password

    if (validatePassword(users[index].password)) { // If the entered password is correct
      lcd.clear();                  // Clear the LCD
      lcd.print("1. Ch1:");         // Display the status of Channel 1
      lcd.print(digitalRead(CHANNEL_1_PIN) == HIGH ? " ON" : " OFF"); // Display ON/OFF status of Channel 1
      lcd.setCursor(0, 1);          // Set the cursor to the beginning of the second row
      lcd.print("2. Ch2:");         // Display the status of Channel 2
      lcd.print(digitalRead(CHANNEL_2_PIN) == HIGH ? " ON" : " OFF"); // Display ON/OFF status of Channel 2

      char chChoice = waitForKey(); // Wait for a keypress to select a channel

      if (chChoice == '1') {        // If '1' is pressed
        digitalWrite(CHANNEL_1_PIN, !digitalRead(CHANNEL_1_PIN)); // Toggle the state of Channel 1
      } else if (chChoice == '2') { // If '2' is pressed
        digitalWrite(CHANNEL_2_PIN, !digitalRead(CHANNEL_2_PIN)); // Toggle the state of Channel 2
      }
    } else {
      lcd.clear();                  // Clear the LCD
      lcd.print("Wrong Pass");      // Display wrong password message
      delay(2000);                  // Wait for 2 seconds
    }
  } else {
    lcd.clear();                    // Clear the LCD
    lcd.print("Invalid User");      // Display invalid user message
    delay(2000);                    // Wait for 2 seconds
  }

  mainMenu();                       // Return to the main menu
}

void changeAdminPass() {
  lcd.clear();                      // Clear the LCD
  lcd.print("Enter Old Pass:");     // Prompt to enter the old admin password

  if (!validatePassword(adminPass)) { // If the entered password is incorrect
    lcd.clear();                    // Clear the LCD
    lcd.print("Wrong Pass");        // Display wrong password message
    delay(2000);                    // Wait for 2 seconds
    mainMenu();                     // Return to the main menu
    return;                         // Exit the function
  }

  lcd.clear();                      // Clear the LCD
  lcd.print("New Admin Pass:");     // Prompt to enter a new admin password
  strcpy(adminPass, inputPassword()); // Store the new admin password
  saveAdminPassToSD();              // Save the new admin password to the SD card

  lcd.clear();                      // Clear the LCD
  lcd.print("Pass Changed!");       // Display password changed message
  delay(2000);                      // Wait for 2 seconds

  mainMenu();                       // Return to the main menu
}

char* inputPassword() {
  static char password[5];          // Create a static array to store the entered password
  int index = 0;                    // Initialize the index for the password array

  while (index < 4) {               // Loop until 4 digits are entered
    char key = keypad.getKey();     // Get the pressed key from the keypad
    if (key) {                      // If a key is pressed
      password[index] = key;        // Store the key in the password array
      lcd.print('*');               // Display a '*' on the LCD for each digit
      index++;                      // Increment the index
    }
  }
  password[4] = '\0';               // Add a null terminator to the password string
  return password;                  // Return the entered password
}

bool validatePassword(char* correctPass) {
  char* enteredPass = inputPassword(); // Get the entered password from the user
  return strcmp(enteredPass, correctPass) == 0; // Compare the entered password with the correct password
}

void saveUsersToSD() {
  File file = SD.open("USERS.TXT", FILE_WRITE); // Open the USERS.TXT file on the SD card for writing
  if (file) {                     // If the file is opened successfully
    for (int i = 0; i < userCount; i++) { // Loop through each user
      file.println(users[i].password); // Write the user's password to the file
    }
    file.close();                   // Close the file
  }
}

void loadUsersFromSD() {
  File file = SD.open("USERS.TXT"); // Open the USERS.TXT file on the SD card for reading
  if (file) {                       // If the file is opened successfully
    userCount = 0;                  // Reset the user count
    while (file.available() && userCount < MAX_USERS) { // While there is data to read and the user count is less than the maximum
      file.readBytesUntil('\n', users[userCount].password, 5); // Read each user's password from the file
      userCount++;                  // Increment the user count
    }
    file.close();                   // Close the file
  }
}

void saveAdminPassToSD() {
  File file = SD.open("ADMIN.TXT", FILE_WRITE); // Open the ADMIN.TXT file on the SD card for writing
  if (file) {                       // If the file is opened successfully
    file.println(adminPass);        // Write the admin password to the file
    file.close();                   // Close the file
  }
}

void loadAdminPassFromSD() {
  File file = SD.open("ADMIN.TXT"); // Open the ADMIN.TXT file on the SD card for reading
  if (file) {                       // If the file is opened successfully
    file.readBytesUntil('\n', adminPass, 5); // Read the admin password from the file
    file.close();                   // Close the file
  }
}

char waitForKey() {
  char key;                         // Declare a variable to store the pressed key
  do {                              // Repeat until a key is pressed
    key = keypad.getKey();          // Get the pressed key from the keypad
  } while (!key);                   // Loop until a key is pressed
  return key;                       // Return the pressed key
}
