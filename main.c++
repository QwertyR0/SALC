#include <Arduino.h>
#include <TinyLiquidCrystal_I2C.h>
#include <TinyWireM.h>
#define EEPROM_I2C_ADDRESS 0x50
#define ARRAY_LENGTH 15

// const uint8_t arrowD[8] PROGMEM = {
    // 0b00100,
    // 0b00100,
    // 0b00100,
    // 0b10101, 
    // 0b11111,
    // 0b01110,
    // 0b00100,
    // 0b00000
// };

// const uint8_t doneT[8] PROGMEM = {
    // 0b00000,
    // 0b00000,
    // 0b00000,
    // 0b00001,
    // 0b00011,
    // 0b10110,
    // 0b11100,
    // 0b01000
// };

TinyLiquidCrystal_I2C lcd(0x27, 16, 2);

byte pressedButton = 0x00; /* 0 = none, 1 = a, 2 = b, 3 = start, 4 = sel */
byte variables[21] = {}; // abcdefghijklmnopqrstu
byte arrayVars[5][ARRAY_LENGTH] = {}; // vwxyz
uint8_t aVarsLength[5] = {0, 0, 0, 0, 0};
unsigned int address = 0;
bool halted = false;
byte keyPressVar = 0;
unsigned int branches[12] = {};
uint8_t branchIndex = 0;
uint8_t branchTypes[12] = {}; // 0 = none, 1 = deactiveIF, 2 = activeIF 3 = activeWHL, 4 = deactiveWHL

// yes I know uint8_t = byte

uint8_t searchFor(byte array[], uint8_t size, uint8_t id){
    uint8_t count = 0;

    for(uint8_t i = 0; i < size; i++){
        if(array[i] == id){
            count++;
        }
    }

    return count;
}

void processInput(){
    int readValue = analogRead(A3);
    byte pressedButton = 0x00;

    if (readValue >= 680){
        //   B
        pressedButton = 0x02;
    } else if (readValue >= 509){
        //   A
        pressedButton = 0x01;
    } else if (readValue >= 140){
        //   sel
        pressedButton = 0x04;
    } else if (readValue >= 130){
        //   start
        pressedButton = 0x03;
    }

    if(keyPressVar != 0 && halted == true && pressedButton != 0x00){
        variables[keyPressVar - 1] = pressedButton;
        halted = false;
        keyPressVar = 0;
    }
}

/*
int joinDigits(int base, int next){
    uint8_t count = 0;
    int nextClone = next;
    while(nextClone){
        nextClone = nextClone / 10;
        count++;
    }

    return base * count + next;
}
*/

// unsigned int joinSingularDigits(unsigned int base, uint8_t next){
//     return base * 10 + next;
// }

void processCommand(){
    // abcdefghijklmnopqrstuvwxyz#$"&. 1234567890

    byte commandArray[3] = {};
    byte argArray[15] = {};
    uint8_t aArrayLength = 0;
    byte stringArray[50] = {};
    uint8_t sArrayLength = 0;
    byte byteArray[15] = {};
    uint8_t bArrayLength = 0;
    bool whitespace = false;
    unsigned int addressToSub = 0;
    uint8_t dots = 0;
    unsigned int commandAdress = address;
    bool inString = false;
    uint8_t nextSkip = 0;
    while (!whitespace){
        byte data = readAddress(address);
        // Serial.print("Address In loop: ");
        // Serial.println(address);
        // Serial.print("AddresstoS     In loop: ");
        // Serial.println(addressToSub);
        if (data != 31 && (data != 32 || inString) && data != 29){
            if(nextSkip == 0){
                // NORMAL BYTE
                if (dots == 0){
                    commandArray[address - commandAdress] = data;
                } else {
                    if (inString){
                        stringArray[sArrayLength] = data;
                        sArrayLength++;
                        // If things go wrong uncomment:
                        addressToSub++;
                    } else {
                        /*else if(data == 30){*/
                        //     byte highB = readAddress(address + 1);
                        //     byte lowB = readAddress(address + 1);
                        //     uint16_t combinedValue = (uint16_t)highB << 8 | lowB;
                        //     numberArray[nArrayLength] = combinedValue;
                        //     nArrayLength++;
                        //     addressToSub++;
                        //     addressToSub++;
                        //     nextSkip = 2;
                        // }

                        argArray[address - addressToSub] = data; /* ALSO ADD HASH TO ARRAY */
                        aArrayLength++;
                        if (data == 27){
                            // HASH
                            byteArray[address - addressToSub] = readAddress(address + 1);
                            bArrayLength++;
                            addressToSub++;
                            nextSkip = 1;
                        } else if (data == 28){
                            // DOLAR
                            byte varName = readAddress(address + 1);
                            if(varName > 21){
                                uint8_t arrayL = aVarsLength[varName - 22];
                                for(uint8_t i = 0; i < arrayL; i++){
                                    stringArray[i + sArrayLength] = arrayVars[varName - 22][i];
                                }
                                sArrayLength = sArrayLength + arrayL;
                                stringArray[sArrayLength] = 29;
                                sArrayLength++;
                                data = 29;
                            } else {
                                byteArray[address - addressToSub] = variables[varName - 1];
                                bArrayLength++;
                                data = 27;
                            }
                            argArray[address - addressToSub] = data;
                            nextSkip = 1;
                            addressToSub++;
                        }
                    }
                }
            } else {
                nextSkip--;
            }
        } else if(data == 29){
            // QUOTE
            inString = !inString;
            if(inString == false){
                stringArray[sArrayLength] = 29;
                sArrayLength++;
            } else {
                argArray[address - addressToSub] = 29;
                aArrayLength++;
            }
            addressToSub++;
        } else if (data == 31){
            // DOT
            if (dots == 0){
                addressToSub = address;
            }
            addressToSub++;
            dots++;
        } else if (data == 32){
            // WHITESPACE
            whitespace = true;
        }
        address++;
    }

    // RUNNING CODE:

    // delay(10);
    // Serial.print("command name: ");
    // Serial.print(commandArray[0]);
    // Serial.print(" ");
    // Serial.println(commandArray[1]);
    // delay(10);
    // Serial.print("addr: ");
    // Serial.println(address);
    // Serial.print("to sub: ");
    // Serial.println(addressToSub);
    // delay(10);
    // Serial.println("args: ");
    // Serial.println(argArray[0]);
    // Serial.println(argArray[1]);
    // Serial.println(argArray[2]);
    // Serial.println(argArray[3]);
    // Serial.println(argArray[4]);
    // Serial.println(argArray[5]);
    // delay(10);
    // Serial.println("bytes: ");
    // Serial.println(byteArray[0]);
    // Serial.println(byteArray[1]);
    // Serial.println(byteArray[2]);
    // Serial.println(byteArray[3]);
    // Serial.println(byteArray[4]);
    // Serial.println(byteArray[5]);
    // delay(10);
    // Serial.println("string: ");
    // Serial.println(stringArray[0]);
    // Serial.println(stringArray[1]);
    // Serial.println(stringArray[2]);
    // Serial.println(stringArray[3]);
    // Serial.println(stringArray[4]);
    // Serial.println(stringArray[5]);
    // Serial.println("----------------------\n");
    // delay(50);
    if(branchIndex == 0 || (branchTypes[branchIndex - 1] != 1 && branchTypes[branchIndex - 1] != 5 && branchTypes[branchIndex - 1] != 6)){
        // BO:
        if(commandArray[0] == 2 && commandArray[1] == 15 && commandArray[2] == 0){
            lcd.backlight();
        }

        // BF:
        if(commandArray[0] == 2 && commandArray[1] == 6 && commandArray[2] == 0){
            lcd.noBacklight();
        }

        // DL:
        if(commandArray[0] == 4 && commandArray[1] == 12 && commandArray[2] == 0){
            if(byteArray[2] == 0xff){
                uint16_t delayTime = (byteArray[0] + 1) * 255 + byteArray[1];
                delay(delayTime);
            } else {
                uint16_t delayTime = byteArray[0] * 255 + byteArray[1];
                delay(delayTime);
            }
        }

        // LO:
        if(commandArray[0] == 12 && commandArray[1] == 15 && commandArray[2] == 0){
            if(argArray[0] > 21){
                // array
                // reset array:
                memset(arrayVars[argArray[0] - 22], 0, ARRAY_LENGTH);
                aVarsLength[argArray[0] - 22] = 0;
                bool quoteDetected = false;
                uint8_t count = 0;
                while (!quoteDetected){
                    if(stringArray[count] == 29){
                        count--;
                        quoteDetected = true;
                        break;
                    }
                    aVarsLength[argArray[0] - 22] = aVarsLength[argArray[0] - 22] + 1;
                    arrayVars[argArray[0] - 22][count] = stringArray[count];
                    count++;
                }
            } else {
                // byte var
                variables[argArray[0] - 1] = byteArray[1];
            }
        }

        // PR:
        if(commandArray[0] == 16 && commandArray[1] == 18 && commandArray[2] == 0){
            char toPrint[ARRAY_LENGTH] = {};
            bool quoteDetected = false;
            uint8_t count = 0;
            while (!quoteDetected){
                if(stringArray[count] == 29){
                    count--;
                    quoteDetected = true;
                    break;
                }
                if(stringArray[count] == 32){
                    toPrint[count] = 32;
                } else if(stringArray[count] > 32){
                    if(stringArray[count] == 42){
                        toPrint[count] = '0';
                    } else {
                        toPrint[count] = '0' + stringArray[count] - 32;
                    }
                } else {
                    toPrint[count] = 'A' + stringArray[count] - 1;
                }
                count++;
            }
            lcd.print(toPrint);
        }

        // CLR:
        if(commandArray[0] == 3 && commandArray[1] == 12 && commandArray[2] == 18){
            lcd.clear();
        }

        // CUR:
        if(commandArray[0] == 3 && commandArray[1] == 21 && commandArray[2] == 18){
            lcd.setCursor(byteArray[0], byteArray[1]);
        }

        // ADD:
        if(commandArray[0] == 1 && commandArray[1] == 4 && commandArray[2] == 4){
            variables[argArray[0] - 1] = byteArray[1] + byteArray[2];
        }

        // SUB:
        if(commandArray[0] == 19 && commandArray[1] == 21 && commandArray[2] == 2){
            variables[argArray[0] - 1] = byteArray[1] - byteArray[2];
        }

        // MUL:
        if(commandArray[0] == 13 && commandArray[1] == 21 && commandArray[2] == 12){
            variables[argArray[0] - 1] = byteArray[1] * byteArray[2];
        }

        // DIV:
        if(commandArray[0] == 4 && commandArray[1] == 9 && commandArray[2] == 22){
            variables[argArray[0] - 1] = byteArray[1] / byteArray[2];
        }

        // RM:
        if(commandArray[0] == 18 && commandArray[1] == 13 && commandArray[2] == 0){
            variables[argArray[0] - 1] = byteArray[1] % byteArray[2];
        }

        // RD:
        if(commandArray[0] == 18 && commandArray[1] == 4 && commandArray[2] == 0){
            if(argArray[1] == 27){
                variables[argArray[0] - 1] = random(byteArray[1]);
            } else {
                variables[argArray[0] - 1] = random(255);
            }
        }

        // GT:
        if(commandArray[0] == 7 && commandArray[1] == 20 && commandArray[2] == 0){
            if(byteArray[2] == 255){
                address = (byteArray[0] + 1) * 255 + byteArray[1];
            } else {
                address = byteArray[0] * 255 + byteArray[1];
            }
        }

        // GC:
        if(commandArray[0] == 7 && commandArray[1] == 3 && commandArray[2] == 0){
            variables[argArray[1] - 1] = address % 255;
            if(address > 65280){
                variables[argArray[0] - 1] =  address / 256;
                variables[argArray[2] - 1] = 255;
            } else {
                variables[argArray[0] - 1] =  address / 255;
                variables[argArray[2] - 1] = 0;
            }
        }

        // MIL:
        if(commandArray[0] == 7 && commandArray[1] == 20 && commandArray[2] == 0){
            variables[argArray[0] - 1] = millis() & 255;
        }

        // BI:
        if(commandArray[0] == 2 && commandArray[1] == 20 && commandArray[2] == 0){
            variables[argArray[0] - 1] = ~byteArray[1];
        }

        // BA:
        if(commandArray[0] == 2 && commandArray[1] == 1 && commandArray[2] == 0){
            variables[argArray[0] - 1] = byteArray[1] & byteArray[2];
        }

        // BO:
        if(commandArray[0] == 2 && commandArray[1] == 15 && commandArray[2] == 0){
            variables[argArray[0] - 1] = byteArray[1] | byteArray[2];
        }

        // BX:
        if(commandArray[0] == 2 && commandArray[1] == 15 && commandArray[2] == 0){
            variables[argArray[0] - 1] = byteArray[1] ^ byteArray[2];
        }

        // CE:
        if(commandArray[0] == 3 && commandArray[1] == 5 && commandArray[2] == 0){
            if(byteArray[1] == byteArray[2]){
                variables[argArray[0] - 1] = 255;
            } else {
                variables[argArray[0] - 1] = 0;
            }
        }

        // CG:
        if(commandArray[0] == 3 && commandArray[1] == 7 && commandArray[2] == 0){
            if(byteArray[1] > byteArray[2]){
                variables[argArray[0] - 1] = 255;
            } else {
                variables[argArray[0] - 1] = 0;
            }
        }

        // CL:
        if(commandArray[0] == 3 && commandArray[1] == 12 && commandArray[2] == 0){
            if(byteArray[1] < byteArray[2]){
                variables[argArray[0] - 1] = 255;
            } else {
                variables[argArray[0] - 1] = 0;
            }
        }

        // YO:
        if(commandArray[0] == 25 && commandArray[1] == 15 && commandArray[2] == 0){
            if(byteArray[1] == 255 || byteArray[2] == 255){
                variables[argArray[0] - 1] = 255;
            } else {
                variables[argArray[0] - 1] = 0;
            }
        }

        // HLT:
        if(commandArray[0] == 8 && commandArray[1] == 12 && commandArray[2] == 20){
            halted = true;
        }

        // ER:
        if(commandArray[0] == 5 && commandArray[1] == 18 && commandArray[2] == 0){
            unsigned int addr = 0; 
            if(byteArray[3] == 255){
                addr = (byteArray[1] + 1) * 255 + byteArray[2];
            } else {
                addr = byteArray[1] * 255 + byteArray[2];
            }
            variables[argArray[0] - 1] = readAddress(addr);
        }

        // EW:
        if(commandArray[0] == 5 && commandArray[1] == 23 && commandArray[2] == 0){
            unsigned int addr = 0; 
            if(byteArray[3] == 255){
                addr = (byteArray[1] + 1) * 255 + byteArray[2];
            } else {
                addr = byteArray[1] * 255 + byteArray[2];
            }
            writeAddress(addr, byteArray[0]);
        }

        // IF:
        if(commandArray[0] == 9 && commandArray[1] == 6 && commandArray[2] == 0){
            if(byteArray[0] == 255){
                // active
                branches[branchIndex] = address;
                branchTypes[branchIndex] = 2;
                branchIndex++;
            } else {
                branches[branchIndex] = address;
                branchTypes[branchIndex] = 1;
                branchIndex++;
            }
        }

        // WHL:
        if(commandArray[0] == 23 && commandArray[1] == 8 && commandArray[2] == 12){
            if(byteArray[0] == 255){
                // active
                branches[branchIndex] = commandAdress;
                branchTypes[branchIndex] = 3;
                branchIndex++;
            } else {
                branches[branchIndex] = commandAdress;
                branchTypes[branchIndex] = 1;
                branchIndex++;
            }
        }

        // BRK:
        // FIXME:
        if(commandArray[0] == 2 && commandArray[1] == 18 && commandArray[2] == 11){
            while(true){
                byte oldType = branchTypes[branchIndex - 1];
                if(oldType == 3) break;
                branches[branchIndex - 1] = 0;
                branchTypes[branchIndex - 1] = 0;
                branchIndex--;
            }
            branches[branchIndex - 1] = 0;
            branchTypes[branchIndex - 1] = 1;
        }

        // KPI:
        if(commandArray[0] == 11 && commandArray[1] == 16 && commandArray[2] == 9){
            halted = true;
            keyPressVar = argArray[0];
        }

        // KP:
        if(commandArray[0] == 11 && commandArray[1] == 17 && commandArray[2] == 0){
            variables[argArray[0] - 1] = pressedButton;
        }

        // SC:
        if(commandArray[0] == 19 && commandArray[1] == 3 && commandArray[2] == 0){
            byte doneArray[ARRAY_LENGTH] = {};
            uint8_t doneArrLength = 0;
            bool quoteDetected = false;
            uint8_t count = 0;
            while (!quoteDetected){
                if(stringArray[count] == 29){
                    count++;
                    quoteDetected = true;
                    break;
                }
                doneArray[doneArrLength] = stringArray[count];
                doneArrLength++;
                count++;
            }
            quoteDetected = false;
            while (!quoteDetected){
                if(stringArray[count] == 29){
                    count++;
                    quoteDetected = true;
                    break;
                }
                doneArray[doneArrLength] = stringArray[count];
                doneArrLength++;
                count++;
            }
            memset(arrayVars[argArray[0] - 22], 0, aVarsLength[argArray[0] - 22]);
            memcpy(arrayVars[argArray[0] - 22], doneArray, doneArrLength);
            aVarsLength[argArray[0] - 22] = doneArrLength;
        }

        // GI:
        if(commandArray[0] == 7 && commandArray[1] == 9 && commandArray[2] == 0){
            if(argArray[0] > 21) /*string*/ {
                arrayVars[argArray[0] - 22][0] = stringArray[byteArray[1]];
            } else /*hash*/ {
                variables[argArray[0] - 1] = stringArray[byteArray[1]];
            }
        }

        // GL:
        if(commandArray[0] == 7 && commandArray[1] == 12 && commandArray[2] == 0){
            uint8_t count = 0;
            // just realised that there was actually no need for quoteDetected shit
            while(true){
                if(stringArray[count] == 29){
                    count--;
                    break;
                }
                count++;
            }
            variables[argArray[0] - 1] = count++;
        }

        // SI:
        if(commandArray[0] == 19 && commandArray[1] == 9 && commandArray[2] == 0){
            if(argArray[2] == 29) /*string*/ {
                // TODO: Replace string continuously length of the stringArray[0]
                arrayVars[argArray[0] - 22][byteArray[1]] = stringArray[0];
                if(byteArray[1] + 1 > aVarsLength[argArray[0] - 22]){
                    aVarsLength[argArray[0] - 22] = byteArray[1] + 1;
                }
            } else if(argArray[2] == 27) /*hash*/ {
                arrayVars[argArray[0] - 22][byteArray[1]] = byteArray[2];
                if(byteArray[1] + 1 > aVarsLength[argArray[0] - 22]){
                    aVarsLength[argArray[0] - 22] = byteArray[1] + 1;
                }
            }
        }

        // NS:
        if(commandArray[0] == 14 && commandArray[1] == 19 && commandArray[2] == 0){
            // TODO: OPTIMIZE
            uint8_t digit;
            uint8_t count = 0;
            byte newArray[3] = {};

            digit = (byteArray[1] / 100) % 10;
            if(digit == 0) digit = 10;
            if(((digit != 10 || byteArray[2] != 255) && !(digit == 10 && byteArray[2] == 255)) || (newArray[count - 1] != 10 && newArray[count - 1] != 0)){
                count++;
                newArray[count - 1] = 32 + digit;
            }

            digit = (byteArray[1] / 10) % 10;
            if(digit == 0) digit = 10;
            if(((digit != 10 || byteArray[2] != 255) && !(digit == 10 && byteArray[2] == 255)) || (newArray[count - 1] != 10 && newArray[count - 1] != 0)){
                count++;
                newArray[count - 1] = 32 + digit;
            }

            digit = byteArray[1] % 10;
            if(digit == 0) digit = 10;
            if(((digit != 10 || byteArray[2] != 255) && !(digit == 10 && byteArray[2] == 255)) || (newArray[count - 1] != 10 && newArray[count - 1] != 0)){
                count++;
                newArray[count - 1] = 32 + digit;
            }

            memset(arrayVars[argArray[0] - 22], 0, aVarsLength[argArray[0] - 22]);
            memcpy(arrayVars[argArray[0] - 22], newArray, count);
            aVarsLength[argArray[0] - 22] = count;
        }
 
        // CN:
        if(commandArray[0] == 3 && commandArray[1] == 14 && commandArray[2] == 0){
            if(byteArray[2] != 0){
                if(stringArray[byteArray[2]] > 32 && stringArray[byteArray[2]] < 43){
                    variables[argArray[0] - 1] = stringArray[byteArray[2]] - 33;
                }
            } else {
                // first [0]
                if(stringArray[0] > 32 && stringArray[0] < 43){
                    variables[argArray[0] - 1] = stringArray[0] - 33;
                }
            }
        }

        // ICN:
        if(commandArray[0] == 3 && commandArray[1] == 14 && commandArray[2] == 0){
            if(byteArray[2] != 0){
                if(stringArray[byteArray[2]] > 32 && stringArray[byteArray[2]] < 43){
                    variables[argArray[0] - 1] = 255;
                }
            } else {
                // first [0]
                if(stringArray[0] > 32 && stringArray[0] < 43){
                    variables[argArray[0] - 1] = 255;
                }
            }
        }

        // SR DEV ONLY:
        /*
        if(commandArray[0] == 19 && commandArray[1] == 18 && commandArray[2] == 0){
            Serial.println(byteArray[0]);
        }
        */
    } // Global State Funcs:

    // ELS:
    if(commandArray[0] == 5 && commandArray[1] == 12 && commandArray[2] == 19){
        if(argArray[0] == 27){
            // ELS IF
            if(byteArray[0] == 255 && (branchTypes[branchIndex - 1] == 1 || branchTypes[branchIndex - 1] == 5)){
                // active
                branches[branchIndex - 1] = address;
                branchTypes[branchIndex - 1] = 4;
            } else if(branchTypes[branchIndex - 1] == 4 || branchTypes[branchIndex - 1] == 6){
                branchTypes[branchIndex - 1] = 6;
            } else {
                branches[branchIndex - 1] = address;
                branchTypes[branchIndex - 1] = 5;
            }
        } else {
            // ELS
            if(branchTypes[branchIndex - 1] == 2 || branchTypes[branchIndex - 1] == 4){
                branchTypes[branchIndex - 1] = 1;
                branches[branchIndex - 1] = address;
            } else if(branchTypes[branchIndex - 1] == 1 || branchTypes[branchIndex - 1] == 5) {
                branchTypes[branchIndex - 1] = 2;
                branches[branchIndex - 1] = address;
            }
        }
    }

    // FI:
    if(commandArray[0] == 6 && commandArray[1] == 9 && commandArray[2] == 0){
        if(branchTypes[branchIndex - 1] == 3){
            address = branches[branchIndex - 1];
        }
        branchTypes[branchIndex - 1] = 0;
        branches[branchIndex - 1] = 0;
        branchIndex--;
    }
}

// to find the place of the hash or the quote loop trough the arg array and count the hashes or array
// then take the count and pass it to their arrays

void setup(){
    TinyWireM.begin();
    lcd.init();
    lcd.home();
    // lcd.createChar(0, arrowD);
    // lcd.createChar(1, doneT);
    lcd.clear();
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    randomSeed(analogRead(A2));
}

void loop(){
      processInput();
    if(halted != true){
      processCommand();
    }
}

void writeAddress(unsigned int address, int val){
    TinyWireM.beginTransmission(EEPROM_I2C_ADDRESS);
    TinyWireM.write((int)(address >> 8));   // MSB
    TinyWireM.write((int)(address & 0xFF)); // LSB
    TinyWireM.write(val);
    TinyWireM.endTransmission();

    delay(5);
}

byte readAddress(unsigned int address){
    byte rData = 0x00;

    TinyWireM.beginTransmission(EEPROM_I2C_ADDRESS);

    TinyWireM.write((int)(address >> 8));   // MSB
    TinyWireM.write((int)(address & 0xFF)); // LSB
    TinyWireM.endTransmission();

    TinyWireM.requestFrom(EEPROM_I2C_ADDRESS, 1);

    rData = TinyWireM.read();

    return rData;
}
