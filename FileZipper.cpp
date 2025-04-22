#include <iostream>     // For standard input/output
#include <fstream>      // For file input/output streams
#include <string>       // For string manipulation
#include <queue>        // For priority_queue used in Huffman Tree
#include <vector>       // For vector used in priority_queue
#include <map>          // For storing character-to-code mappings

// Node structure for Huffman Tree
struct HuffmanNode {
    char character;              // Character (valid only in leaf nodes)
    int frequency;               // Frequency of the character
    HuffmanNode* left;           // Left child
    HuffmanNode* right;          // Right child

    HuffmanNode(char ch, int freq, HuffmanNode* l = nullptr, HuffmanNode* r = nullptr)
        : character(ch), frequency(freq), left(l), right(r) {}
};

// Custom comparator to create a min-heap based on frequency
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;  // Min-heap behavior
    }
};

// Recursive function to generate Huffman codes
void buildHuffmanCodes(HuffmanNode* node, const std::string& code, std::map<char, std::string>& huffmanCodes) {
    if (node == nullptr) return;

    if (node->left == nullptr && node->right == nullptr) {
        huffmanCodes[node->character] = code;  // Store code for character
    }

    buildHuffmanCodes(node->left, code + "0", huffmanCodes);
    buildHuffmanCodes(node->right, code + "1", huffmanCodes);
}

// Display usage instructions to the user
void showUsage(const std::string& programName) {
    std::cout << "Usage:\n"
        << "  " << programName << " zip <input_file> <output_file>\n"
        << "  " << programName << " unzip <input_file> <output_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        showUsage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFileName = argv[2];
    std::string outputFileName = argv[3];

    std::ifstream inputFile(inputFileName, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Cannot open input file '" << inputFileName << "'\n";
        return 1;
    }

    std::ofstream outputFile(outputFileName, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error: Cannot open output file '" << outputFileName << "'\n";
        return 1;
    }

    if (mode == "zip") {
        std::cout << "Zipping file: " << inputFileName << "\n";

        // Step 1: Count character frequencies
        const int ASCII_SIZE = 256;
        int freq[ASCII_SIZE] = { 0 };
        char ch;
        while (inputFile.get(ch)) {
            freq[static_cast<unsigned char>(ch)]++;
        }

        // Step 2: Build the min-heap using frequencies
        std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;
        for (int i = 0; i < ASCII_SIZE; ++i) {
            if (freq[i] > 0) {
                pq.push(new HuffmanNode(static_cast<char>(i), freq[i]));
            }
        }

        // Step 3: Build Huffman Tree
        while (pq.size() > 1) {
            HuffmanNode* left = pq.top(); pq.pop();
            HuffmanNode* right = pq.top(); pq.pop();
            int combinedFreq = left->frequency + right->frequency;
            pq.push(new HuffmanNode('\0', combinedFreq, left, right));
        }
        HuffmanNode* root = pq.top();

        // Step 4: Generate Huffman codes
        std::map<char, std::string> huffmanCodes;
        buildHuffmanCodes(root, "", huffmanCodes);

        // Step 5: Encode input using generated codes
        inputFile.clear();
        inputFile.seekg(0, std::ios::beg);
        std::string encodedData;
        while (inputFile.get(ch)) {
            encodedData += huffmanCodes[ch];
        }

        // Step 6: Write codes and data to output file
        outputFile << "CODES:\n";
        for (const auto& pair : huffmanCodes) {
            outputFile << pair.first << ":" << pair.second << "\n";
        }
        outputFile << "DATA:\n";
        outputFile << encodedData << "\n";

        std::cout << "\nHuffman Codes:\n";
        for (const auto& pair : huffmanCodes) {
            std::cout << "'" << pair.first << "': " << pair.second << "\n";
        }

        // Print frequencies to console
        std::cout << "\nCharacter frequencies:\n";
        for (int i = 0; i < ASCII_SIZE; ++i) {
            if (freq[i] > 0) {
                std::cout << "'" << static_cast<char>(i) << "' (" << i << "): " << freq[i] << " times\n";
            }
        }

        // Print encoded bitstring to console
        std::cout << "\nEncoded Bitstring:\n" << encodedData << "\n";

    }

    else if (mode == "unzip") {
        std::cout << "Unzipping file: " << inputFileName << "\n";

        // Step 1: Read Huffman codes
        std::map<std::string, char> codeToChar;
        std::string line;

        std::getline(inputFile, line);  // Expect "CODES:"
        if (line != "CODES:") {
            std::cerr << "Error: Invalid file format — missing CODES section.\n";
            return 1;
        }

        while (std::getline(inputFile, line)) {
            if (line == "DATA:") break;
            if (line.size() >= 3 && line[1] == ':') {
                char ch = line[0];
                std::string code = line.substr(2);
                codeToChar[code] = ch;
            }
        }

        // Step 2: Read encoded bitstring
        std::string encodedData;
        std::getline(inputFile, encodedData);

        // Step 3: Decode bitstring using Huffman codes
        std::string currentCode;
        for (char bit : encodedData) {
            currentCode += bit;
            if (codeToChar.count(currentCode)) {
                outputFile.put(codeToChar[currentCode]);
                currentCode.clear();
            }
        }

        std::cout << "Decompression completed. Decoded file written to: " << outputFileName << "\n";
    }

    else {
        std::cerr << "Error: Unknown mode '" << mode << "'\n";
        showUsage(argv[0]);
        return 1;
    }

    inputFile.close();
    outputFile.close();
    return 0;
}
