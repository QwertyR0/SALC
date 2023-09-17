const process = require("process");
const fs = require("fs")
let program = "BF DL.#7.#215 BO";
let compiled = "";
// abcdefghijklmnopqrstuvwxyz#$". 

if(process.argv[2].toLowerCase() === "t" || process.argv[2].toLowerCase() === "true"){
    function readSalFile(filename) {
        try {
            return fs.readFileSync(filename, 'utf-8');
        } catch (error) {
            console.error(`Error reading ${filename}: ${error}`);
            process.exit(1);
        }
    }
    
    const filename = 'main.sal';
    program = readSalFile(filename);
    program = program.replace(/[\r\n]+/g, " ");
    program = program.replace(/\n/g, " ");
}

function processCode(char) {
    char = char.toLowerCase();
    switch (char) {
        case "a":
            return "0x01, ";
        case "b":
            return "0x02, ";
        case "c":
            return "0x03, ";
        case "d":
            return "0x04, ";
        case "e":
            return "0x05, ";
        case "f":
            return "0x06, ";
        case "g":
            return "0x07, ";
        case "h":
            return "0x08, ";
        case "i":
            return "0x09, ";
        case "j":
            return "0x0A, ";
        case "k":
            return "0x0B, ";
        case "l":
            return "0x0C, ";
        case "m":
            return "0x0D, ";
        case "n":
            return "0x0E, ";
        case "o":
            return "0x0F, ";
        case "p":
            return "0x10, ";
        case "q":
            return "0x11, ";
        case "r":
            return "0x12, ";
        case "s":
            return "0x13, ";
        case "t":
            return "0x14, ";
        case "u":
            return "0x15, ";
        case "v":
            return "0x16, ";
        case "w":
            return "0x17, ";
        case "x":
            return "0x18, ";
        case "y":
            return "0x19, ";
        case "z":
            return "0x1A, ";
        case "#":
            return "0x1B, ";
        case "$":
            return "0x1C, ";
        case "\"":
            return "0x1D, ";
        case "&":
            return "0x1E, "
        case ".":
            return "0x1F, ";
        case " ":
            return "0x20, ";
        case "1":
            return "0x21, ";
        case "2":
            return "0x22, ";
        case "3":
            return "0x23, ";
        case "4":
            return "0x24, ";
        case "5":
            return "0x25, ";
        case "6":
            return "0x26, ";
        case "7":
            return "0x27, ";
        case "8":
            return "0x28, ";
        case "9":
            return "0x29, ";
        case "0":
            return "0x2A, ";
        case "\\":
            return "0x2B, ";
        default:
            return "Invalid character";
    }
}

function count(str, find) {
    return (str.split(find)).length - 1;
}

function countDigits(str) {
    let digitCount = 0;
    
    for (let i = 0; i < str.length; i++) {
        if (/[0-9]/.test(str[i])) {
            digitCount++;
        }
    }
    
    return digitCount;
}

for(let i = 0; i < program.length; i++){
    if(program[i] === "#"){
        let numberData = "";
        let queue = 0;
        let foundEnd = false;
        while (!foundEnd){
            queue++;
            if(!isNaN(parseInt(program[i + queue]))){
                numberData += program[i + queue];
            } else {
                foundEnd = true;
                queue--;
            }
        }
        numberData = "0x" + parseInt(numberData, 10).toString(16).padStart(2, '0');
        compiled += processCode("#");
        compiled += numberData + ", ";
        i += queue;
    } else {
        const nextC = processCode(program[i]);
        if(nextC == "0x20, "){
            if(!compiled.endsWith("0x20, ")){
                compiled += nextC;
            }
        } else {
            compiled += nextC;
        }
    }
}

compiled = compiled.slice(0, -2);
if (!compiled.endsWith("0x20")) compiled += ", 0x20";
compiled = `const byte program[${count(compiled, ",") + 1}] PROGMEM = {${compiled}};`;

console.log(compiled);
