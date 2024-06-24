#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

using namespace std;

struct LZ77Token {
    short offset;
    char length;
    char next;
};

// Función para encontrar la mayor coincidencia
LZ77Token findLongestMatch(const string& input, size_t currentPos, size_t windowSize) {
    size_t maxMatchLength = 0;
    size_t matchOffset = 0;
    char nextChar = input[currentPos];

    for (size_t i = (currentPos > windowSize) ? currentPos - windowSize : 0; i < currentPos; ++i) {
        size_t matchLength = 0;
        while (currentPos + matchLength < input.size() && input[i + matchLength] == input[currentPos + matchLength]) {
            matchLength++;
        }

        if (matchLength > maxMatchLength) {
            maxMatchLength = matchLength;
            matchOffset = currentPos - i;
            if (currentPos + matchLength < input.size()) {
                nextChar = input[currentPos + matchLength];
            } else {
                nextChar = '\0';
            }
        }
    }

    return {static_cast<short>(matchOffset), static_cast<char>(maxMatchLength), nextChar};
}

// Función para descomprimir la secuencia de tokens y obtener el texto original
string descomprimir(const vector<LZ77Token>& tokens) {
    string output;

    for (const LZ77Token& token : tokens) {
        if (token.offset == 0 && token.length == 0) {
            output += token.next;
        } else {
            size_t startPos = output.size() - token.offset;
            for (int i = 0; i < token.length; ++i) {
                output += output[startPos + i];
            }
            output += token.next;
        }
    }

    return output;
}

int main() {
    // Lee el archivo de entrada
    ifstream inputFile("dblp.xml.50MB");
    string inputText;
    getline(inputFile, inputText, '\0');
    inputFile.close();

    // Comienza a medir el tiempo
    auto start = chrono::high_resolution_clock::now();

    vector<LZ77Token> tokens;
    size_t windowSize = 4096;
    size_t pos = 0;

    while (pos < inputText.size()) {
        LZ77Token token = findLongestMatch(inputText, pos, windowSize);
        tokens.push_back(token);
        pos += token.length + 1;
    }

    // Guarda los tokens en el archivo comprimido en formato binario
    ofstream outputFile("archivo_comprimido.bin", ios::binary);
    for (const LZ77Token& token : tokens) {
        outputFile.write(reinterpret_cast<const char*>(&token.offset), sizeof(token.offset));
        outputFile.write(reinterpret_cast<const char*>(&token.length), sizeof(token.length));
        outputFile.write(&token.next, sizeof(token.next));
    }
    outputFile.close();

    cout << "Archivo comprimido exitosamente." << endl;

    // Leer los tokens del archivo comprimido
    ifstream inputFileBin("archivo_comprimido.bin", ios::binary);
    vector<LZ77Token> tokensRead;
    LZ77Token token;
    while (inputFileBin.read(reinterpret_cast<char*>(&token.offset), sizeof(token.offset))) {
        inputFileBin.read(reinterpret_cast<char*>(&token.length), sizeof(token.length));
        inputFileBin.read(&token.next, sizeof(token.next));
        tokensRead.push_back(token);
    }
    inputFileBin.close();

    // Descomprimir el archivo
    string decompressedText = descomprimir(tokensRead);

    // Guarda el archivo descomprimido
    ofstream decompressedFile("archivo_descomprimido.txt");
    decompressedFile << decompressedText;
    decompressedFile.close();

    cout << "Archivo descomprimido exitosamente." << endl;

    // Termina de medir el tiempo
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo de ejecución: " << duration << " microsegundos" << endl;

    return 0;
}
