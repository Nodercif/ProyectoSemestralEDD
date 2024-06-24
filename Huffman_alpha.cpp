#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <chrono>
#include <vector>
#include <bitset>
using namespace std;

struct Node {
    char character;
    int frecuencia;
    Node* left;
    Node* right;

    // Constructor
    Node(char ch, int freq) : character(ch), frecuencia(freq), left(nullptr), right(nullptr) {}
};

// Comparador de prioridad en cola
struct Compare {
    bool operator()(Node* left, Node* right) {
        return left->frecuencia > right->frecuencia;
    }
};

// Función árbol de Huffman (build Huffman tree BHT)
Node* BHT(map<char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> minHeap;

    // Crea nodo hoja para cada caracter y lo agrega a la cola de prioridad
    for (auto par : freqMap) {
        minHeap.push(new Node(par.first, par.second));
    }

    // Iterar hasta que el tamaño de la cola sea 1
    while (minHeap.size() > 1) {
        // Extrae los dos nodos con menor frecuencia
        Node* left = minHeap.top();
        minHeap.pop();
        Node* right = minHeap.top();
        minHeap.pop();

        // Crea un nuevo nodo con estos dos nodos como hijos y con la suma de sus frecuencias
        Node* sumNode = new Node('\0', left->frecuencia + right->frecuencia);
        sumNode->left = left;
        sumNode->right = right;

        // Inserta el nuevo nodo en la cola
        minHeap.push(sumNode);
    }

    // El nodo restante es la raíz del árbol de Huffman
    return minHeap.top();
}

// Función para asignar códigos binarios a los caracteres
void assignHuffmanCodes(Node* root, string code, map<char, string>& huffmanCodes) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCodes[root->character] = code;
    }

    // Recorre el árbol de Huffman
    assignHuffmanCodes(root->left, code + "0", huffmanCodes);
    assignHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// Libera memoria del árbol de Huffman (Free Huffman Tree FHT)
void FHT(Node* root) {
    if (!root) return;
    FHT(root->left);
    FHT(root->right);
    delete root;
}

// Función para escribir bits en un archivo
void writeBits(ofstream& output, const string& bits) {
    int bitBuffer = 0;
    int bitCount = 0;

    for (char bit : bits) {
        bitBuffer = (bitBuffer << 1) | (bit - '0');
        bitCount++;

        if (bitCount == 8) {
            output.put(static_cast<char>(bitBuffer));
            bitBuffer = 0;
            bitCount = 0;
        }
    }

    if (bitCount > 0) {
        bitBuffer <<= (8 - bitCount);
        output.put(static_cast<char>(bitBuffer));
    }
}

// Función para leer bits de un archivo
string readBits(ifstream& input) {
    string bits = "";
    char byte;

    while (input.get(byte)) {
        bits += bitset<8>(byte).to_string();
    }

    return bits;
}

// Función para decodificar el archivo comprimido
string decodeHuffman(Node* root, const string& encodedText) {
    string decodedText = "";
    Node* currentNode = root;

    for (char bit : encodedText) {
        if (bit == '0') {
            currentNode = currentNode->left;
        } else {
            currentNode = currentNode->right;
        }
        if (!currentNode->left && !currentNode->right) {
            decodedText += currentNode->character;
            currentNode = root;
        }
    }

    return decodedText;
}

int main() {
    // Lee el archivo de entrada
    ifstream inputFile("dblp.xml.50MB");
    string inputText;
    getline(inputFile, inputText, '\0');
    inputFile.close();

    // Comienza a medir el tiempo
    auto start = chrono::high_resolution_clock::now();

    // Calcula las frecuencias de los caracteres
    map<char, int> freqMap;
    for (char c : inputText) {
        freqMap[c]++;
    }

    // Construye el árbol de Huffman
    Node* huffmanTree = BHT(freqMap);

    // Asigna códigos binarios a los caracteres
    map<char, string> huffmanCodes;
    assignHuffmanCodes(huffmanTree, "", huffmanCodes);

    // Codifica el archivo
    ofstream outputFile("archivo_codificado.bin", ios::binary);
    string encodedText = "";
    for (char c : inputText) {
        encodedText += huffmanCodes[c];
    }
    writeBits(outputFile, encodedText);
    outputFile.close();

    cout << "Archivo codificado exitosamente." << endl;

    // Decodifica el archivo comprimido
    ifstream encodedFile("archivo_codificado.bin", ios::binary);
    string encodedStr = readBits(encodedFile);
    encodedFile.close();

    string decodedText = decodeHuffman(huffmanTree, encodedStr);

    ofstream decodedOutputFile("archivo_decodificado.txt");
    decodedOutputFile << decodedText;
    decodedOutputFile.close();

    cout << "Archivo decodificado exitosamente." << endl;

    // Libera memoria
    FHT(huffmanTree);

    // Termina de medir el tiempo
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo de ejecución: " << duration << " microsegundos" << endl;

    return 0;
}
