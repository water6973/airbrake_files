// Configuration Variables
unsigned long calculationInterval = 50; // Time interval for calculations in milliseconds
unsigned long delayBetweenCycles = 1000; // Delay between cycles in milliseconds
int numberOfDigits = 15; // Number of digits after the decimal point

void setup() {
  Serial.begin(9600); // Initialize serial communication at 9600 bits per second
  randomSeed(analogRead(0)); // Initialize random seed
}

void loop() {
  unsigned long startTime = millis(); // Start time of the calculation period
  unsigned long endTime = startTime + calculationInterval; // End time of the calculation period

  long count = 0; // Counter for the number of calculations

  // Perform calculations until the current time is greater than the end time
  while(millis() < endTime) {
    // Generate two random floating point numbers based on the numberOfDigits variable 
    // Generating random numbers is itself a computation - typically these tests would have the computer multiply the same numbers together over and over again, but that's no fun.
    double a = generateRandomFloat(numberOfDigits);
    double b = generateRandomFloat(numberOfDigits);

    double calculationResult = a * b; // Perform a floating point multiplication

    count++; // Increment the counter
  }

  // Once the interval is complete, output the number of calculations performed
  Serial.print("Calculations completed in ");
  Serial.print(calculationInterval);
  Serial.print(" ms: ");
  Serial.println(count);

  delay(delayBetweenCycles); // Delay to make the output easier to read
}

float generateRandomFloat(int digitsAfterDecimal) {
  float multiplier = pow(10, digitsAfterDecimal);
  return random(1, multiplier) / multiplier;
}
