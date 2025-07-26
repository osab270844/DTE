#include "DeviceTreeParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace DTE {

// DTBParser implementation
std::shared_ptr<DeviceTree> DTBParser::parse(const std::string& filename) {
    try {
        auto tree = std::make_shared<DeviceTree>();
        if (parseDTBFile(filename, tree)) {
            tree->setSourceFile(filename);
            return tree;
        }
    } catch (const std::exception& e) {
        std::cerr << "DTB parsing error: " << e.what() << std::endl;
    }
    return nullptr;
}

bool DTBParser::canParse(const std::string& filename) const {
    // Check file extension
    if (filename.find(".dtb") != std::string::npos) {
        return true;
    }
    
    // Check file magic
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    
    uint32_t magic;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    
    // DTB magic number: 0xd00dfeed (big-endian)
    // Also check for little-endian: 0xedfe0dd0
    return magic == 0xd00dfeed || magic == 0xedfe0dd0;
}

bool DTBParser::parseDTBFile(const std::string& filename, std::shared_ptr<DeviceTree> tree) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    // Read file into memory
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (fileSize < 40) {
        throw std::runtime_error("File too small to be a valid DTB");
    }
    
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    
    if (file.gcount() != static_cast<std::streamsize>(fileSize)) {
        throw std::runtime_error("Failed to read entire file");
    }
    
    // Parse DTB header
    const uint32_t* header = reinterpret_cast<const uint32_t*>(buffer.data());
    uint32_t magic = header[0];
    uint32_t totalsize = header[1];
    uint32_t off_dt_struct = header[2];
    uint32_t off_dt_strings = header[3];
    uint32_t off_mem_rsvmap = header[4];
    uint32_t version = header[5];
    uint32_t last_comp_version = header[6];
    uint32_t boot_cpuid_phys = header[7];
    uint32_t size_dt_strings = header[8];
    uint32_t size_dt_struct = header[9];
    
    // Validate magic number and handle endianness
    bool isLittleEndian = false;
    if (magic == 0xd00dfeed) {
        isLittleEndian = false;
    } else if (magic == 0xedfe0dd0) {
        isLittleEndian = true;
        // Convert header values to host endianness
        totalsize = __builtin_bswap32(totalsize);
        off_dt_struct = __builtin_bswap32(off_dt_struct);
        off_dt_strings = __builtin_bswap32(off_dt_strings);
        off_mem_rsvmap = __builtin_bswap32(off_mem_rsvmap);
        version = __builtin_bswap32(version);
        last_comp_version = __builtin_bswap32(last_comp_version);
        boot_cpuid_phys = __builtin_bswap32(boot_cpuid_phys);
        size_dt_strings = __builtin_bswap32(size_dt_strings);
        size_dt_struct = __builtin_bswap32(size_dt_struct);
    } else {
        throw std::runtime_error("Invalid DTB magic number: 0x" + 
                               std::to_string(magic));
    }
    
    // Validate header values
    if (totalsize != fileSize) {
        throw std::runtime_error("File size mismatch: expected " + 
                               std::to_string(totalsize) + ", got " + 
                               std::to_string(fileSize));
    }
    
    if (off_dt_struct >= fileSize || off_dt_strings >= fileSize || 
        off_mem_rsvmap >= fileSize) {
        throw std::runtime_error("Invalid offset in DTB header");
    }
    
    if (off_dt_struct + size_dt_struct > fileSize) {
        throw std::runtime_error("Structure block extends beyond file");
    }
    
    if (off_dt_strings + size_dt_strings > fileSize) {
        throw std::runtime_error("Strings block extends beyond file");
    }
    
    // Validate version compatibility
    if (version < 16) {
        throw std::runtime_error("DTB version too old: " + std::to_string(version));
    }
    
    if (version > 17) {
        std::cerr << "Warning: DTB version " << version << " may not be fully supported" << std::endl;
    }
    
    // Parse structure block
    size_t structOffset = off_dt_struct;
    size_t nextOffset = structOffset;
    
    auto rootNode = parseDTBNode(buffer.data(), structOffset, nextOffset, 
                                off_dt_strings, isLittleEndian);
    if (rootNode) {
        tree->setRoot(rootNode);
        return true;
    }
    
    return false;
}

std::shared_ptr<DeviceTreeNode> DTBParser::parseDTBNode(const void* data, size_t offset, 
                                                        size_t& nextOffset, size_t stringsOffset,
                                                        bool isLittleEndian) {
    const uint32_t* structData = reinterpret_cast<const uint32_t*>(static_cast<const char*>(data) + offset);
    uint32_t token = isLittleEndian ? __builtin_bswap32(structData[0]) : structData[0];
    
    if (token == 0x00000001) { // FDT_BEGIN_NODE
        // Parse node name
        const char* nodeName = reinterpret_cast<const char*>(structData + 1);
        
        // Handle empty node names (root node)
        std::string name = nodeName;
        if (name.empty() || name == "/") {
            name = "/";
        }
        
        auto node = std::make_shared<DeviceTreeNode>(name);
        
        // Skip node name (including null terminator)
        size_t nameLen = strlen(nodeName) + 1;
        size_t nameWords = (nameLen + 3) / 4; // Round up to word boundary
        size_t currentOffset = offset + 4 + nameWords * 4;
        
        // Parse properties and child nodes
        while (currentOffset < nextOffset) {
            const uint32_t* currentToken = reinterpret_cast<const uint32_t*>(static_cast<const char*>(data) + currentOffset);
            uint32_t currentTokenValue = isLittleEndian ? __builtin_bswap32(currentToken[0]) : currentToken[0];
            
            if (currentTokenValue == 0x00000002) { // FDT_PROP
                auto prop = parseDTBProperty(data, currentOffset, currentOffset, 
                                           stringsOffset, isLittleEndian);
                node->addProperty(prop);
            } else if (currentTokenValue == 0x00000001) { // FDT_BEGIN_NODE
                auto childNode = parseDTBNode(data, currentOffset, currentOffset, 
                                            stringsOffset, isLittleEndian);
                if (childNode) {
                    node->addChild(childNode);
                }
            } else if (currentTokenValue == 0x00000003) { // FDT_END_NODE
                currentOffset += 4;
                break;
            } else if (currentTokenValue == 0x00000009) { // FDT_END
                break;
            } else {
                // Skip unknown tokens
                currentOffset += 4;
            }
        }
        
        nextOffset = currentOffset;
        return node;
    }
    
    return nullptr;
}

DeviceTreeProperty DTBParser::parseDTBProperty(const void* data, size_t offset, 
                                              size_t& nextOffset, size_t stringsOffset,
                                              bool isLittleEndian) {
    const uint32_t* propData = reinterpret_cast<const uint32_t*>(static_cast<const char*>(data) + offset);
    
    // FDT_PROP token
    uint32_t len = isLittleEndian ? __builtin_bswap32(propData[1]) : propData[1];
    uint32_t nameoff = isLittleEndian ? __builtin_bswap32(propData[2]) : propData[2];
    
    // Validate string offset - simplified validation
    if (nameoff >= 0x1000000) { // Reasonable upper limit
        throw std::runtime_error("Invalid property name offset");
    }
    
    // Get property name from strings block
    const char* propName = static_cast<const char*>(data) + stringsOffset + nameoff;
    
    // Validate property name
    if (strlen(propName) == 0) {
        throw std::runtime_error("Empty property name");
    }
    
    // Get property value
    const char* propValue = reinterpret_cast<const char*>(propData + 3);
    
    // Create property value based on length and content
    PropertyValue value;
    if (len == 0) {
        value = std::string("");
    } else if (len > 0 && propValue[len - 1] == '\0') {
        // String property - check if it's a valid string
        std::string strValue(propValue, len - 1);
        // Validate string contains only printable characters
        if (std::all_of(strValue.begin(), strValue.end(), 
                       [](char c) { return c >= 32 && c <= 126; })) {
            value = strValue;
        } else {
            // Treat as binary if not a valid string
            std::vector<uint8_t> binaryData(propValue, propValue + len);
            value = binaryData;
        }
    } else {
        // Binary property
        std::vector<uint8_t> binaryData(propValue, propValue + len);
        value = binaryData;
    }
    
    nextOffset = offset + 12 + ((len + 3) / 4) * 4; // Round up to word boundary
    return DeviceTreeProperty(propName, value);
}

// DTSParser implementation
std::shared_ptr<DeviceTree> DTSParser::parse(const std::string& filename) {
    try {
        auto tree = std::make_shared<DeviceTree>();
        if (parseDTSFile(filename, tree)) {
            tree->setSourceFile(filename);
            return tree;
        }
    } catch (const std::exception& e) {
        std::cerr << "DTS parsing error: " << e.what() << std::endl;
    }
    return nullptr;
}

bool DTSParser::canParse(const std::string& filename) const {
    return filename.find(".dts") != std::string::npos;
}

bool DTSParser::parseDTSFile(const std::string& filename, std::shared_ptr<DeviceTree> tree) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string line;
    auto rootNode = parseDTSNode(file, line);
    if (rootNode) {
        tree->setRoot(rootNode);
        return true;
    }
    
    return false;
}

std::shared_ptr<DeviceTreeNode> DTSParser::parseDTSNode(std::istream& stream, std::string& line) {
    // Skip to node definition
    while (std::getline(stream, line)) {
        // Skip comments and empty lines
        std::string trimmedLine = line;
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
        if (trimmedLine.empty() || (trimmedLine[0] == '/' && trimmedLine[1] == '/') || trimmedLine[0] == '*') {
            continue;
        }
        
        if (line.find("/dts-v1/;") != std::string::npos) {
            continue; // Skip version header
        }
        
        // Check if this line contains a node definition
        if (line.find("{") != std::string::npos) {
            // Extract node name
            size_t bracePos = line.find("{");
            std::string nodeName = line.substr(0, bracePos);
            
            // Trim whitespace
            nodeName.erase(0, nodeName.find_first_not_of(" \t"));
            nodeName.erase(nodeName.find_last_not_of(" \t") + 1);
            
            // Handle root node
            if (nodeName.empty() || nodeName == "/") {
                nodeName = "/";
            }
            
            auto node = std::make_shared<DeviceTreeNode>(nodeName);
            
            // Parse node content using a stack-based approach
            std::vector<std::shared_ptr<DeviceTreeNode>> nodeStack;
            nodeStack.push_back(node);
            
            while (std::getline(stream, line)) {
                std::string trimmedLine = line;
                trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));
                
                if (trimmedLine.find("};") != std::string::npos) {
                    // End of current node, pop from stack
                    if (nodeStack.size() > 1) {
                        nodeStack.pop_back();
                    } else {
                        break; // End of root node
                    }
                    continue;
                }
                
                // Skip empty lines and comments
                if (trimmedLine.empty() || (trimmedLine[0] == '/' && trimmedLine[1] == '/') || trimmedLine[0] == '*') {
                    continue;
                }
                
                // Check if it's a property
                if (line.find("=") != std::string::npos) {
                    try {
                        auto prop = parseDTSProperty(stream, line);
                        nodeStack.back()->addProperty(prop);
                    } catch (const std::exception& e) {
                        std::cerr << "Warning: Failed to parse property: " << e.what() << std::endl;
                        continue;
                    }
                }
                // Check if it's a child node (node name followed by {)
                else if (line.find("{") != std::string::npos) {
                    try {
                        // Extract child node name from current line
                        size_t bracePos = line.find("{");
                        std::string childNodeName = line.substr(0, bracePos);
                        
                        // Trim whitespace
                        childNodeName.erase(0, childNodeName.find_first_not_of(" \t"));
                        childNodeName.erase(childNodeName.find_last_not_of(" \t") + 1);
                        
                        auto childNode = std::make_shared<DeviceTreeNode>(childNodeName);
                        
                        // Add child to current node and push to stack
                        nodeStack.back()->addChild(childNode);
                        nodeStack.push_back(childNode);
                        
                    } catch (const std::exception& e) {
                        std::cerr << "Warning: Failed to parse child node: " << e.what() << std::endl;
                        continue;
                    }
                }
            }
            
            return node;
        }
    }
    
    return nullptr;
}

DeviceTreeProperty DTSParser::parseDTSProperty(std::istream& stream, std::string& line) {
    // Extract property name
    size_t equalPos = line.find("=");
    if (equalPos == std::string::npos) {
        throw std::runtime_error("Invalid property syntax: missing '='");
    }
    
    std::string propName = line.substr(0, equalPos);
    
    // Trim whitespace
    propName.erase(0, propName.find_first_not_of(" \t"));
    propName.erase(propName.find_last_not_of(" \t") + 1);
    
    if (propName.empty()) {
        throw std::runtime_error("Empty property name");
    }
    
    // Parse property value
    std::string value = parseDTSValue(stream, line);
    
    // Determine value type and create property
    PropertyValue propValue;
    if (value.empty()) {
        propValue = std::string("");
    } else if (value[0] == '"' && value[value.length() - 1] == '"') {
        // String value
        propValue = value.substr(1, value.length() - 2);
    } else if (value[0] == '<' && value[value.length() - 1] == '>') {
        // Cell values
        std::vector<uint32_t> cells;
        std::string cellStr = value.substr(1, value.length() - 2);
        std::istringstream cellStream(cellStr);
        std::string cell;
        
        while (std::getline(cellStream, cell, ' ')) {
            if (!cell.empty()) {
                // Remove 0x prefix if present
                if (cell.substr(0, 2) == "0x") {
                    cell = cell.substr(2);
                }
                try {
                    cells.push_back(std::stoul(cell, nullptr, 16));
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid cell value: " + cell);
                }
            }
        }
        propValue = cells;
    } else if (value[0] == '[' && value[value.length() - 1] == ']') {
        // Binary values
        std::vector<uint8_t> binary;
        std::string binaryStr = value.substr(1, value.length() - 2);
        std::istringstream binaryStream(binaryStr);
        std::string byte;
        
        while (std::getline(binaryStream, byte, ' ')) {
            if (!byte.empty()) {
                // Remove 0x prefix if present
                if (byte.substr(0, 2) == "0x") {
                    byte = byte.substr(2);
                }
                try {
                    binary.push_back(std::stoul(byte, nullptr, 16));
                } catch (const std::exception& e) {
                    throw std::runtime_error("Invalid binary value: " + byte);
                }
            }
        }
        propValue = binary;
    } else {
        // Treat as string
        propValue = value;
    }
    
    return DeviceTreeProperty(propName, propValue);
}

std::string DTSParser::parseDTSValue(std::istream& stream, std::string& line) {
    size_t equalPos = line.find("=");
    std::string value = line.substr(equalPos + 1);
    
    // Trim whitespace
    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);
    
    // Remove semicolon if present
    if (!value.empty() && value[value.length() - 1] == ';') {
        value = value.substr(0, value.length() - 1);
    }
    
    // Handle multi-line values
    if (value.find("\\") != std::string::npos) {
        std::string fullValue = value;
        while (std::getline(stream, line)) {
            if (line.find(";") != std::string::npos) {
                fullValue += line.substr(0, line.find(";"));
                break;
            }
            fullValue += line;
        }
        return fullValue;
    }
    
    return value;
}

// DeviceTreeParserFactory implementation
std::unique_ptr<DeviceTreeParser> DeviceTreeParserFactory::createParser(const std::string& filename) {
    auto parsers = getAllParsers();
    
    for (auto& parser : parsers) {
        if (parser->canParse(filename)) {
            return std::move(parser);
        }
    }
    
    return nullptr;
}

std::vector<std::unique_ptr<DeviceTreeParser>> DeviceTreeParserFactory::getAllParsers() {
    std::vector<std::unique_ptr<DeviceTreeParser>> parsers;
    parsers.push_back(std::make_unique<DTBParser>());
    parsers.push_back(std::make_unique<DTSParser>());
    return parsers;
}

// DeviceTreeUtils implementation
namespace DeviceTreeUtils {

bool dtbToDts(const std::string& dtbFile, const std::string& dtsFile) {
#ifdef HAVE_DTC
    // Use system dtc command
    std::string command = "dtc -I dtb -O dts -o " + dtsFile + " " + dtbFile + " 2>/dev/null";
    return system(command.c_str()) == 0;
#else
    std::cerr << "Warning: dtc not available - conversion not supported" << std::endl;
    return false;
#endif
}

bool dtsToDtb(const std::string& dtsFile, const std::string& dtbFile) {
#ifdef HAVE_DTC
    // Use system dtc command
    std::string command = "dtc -I dts -O dtb -o " + dtbFile + " " + dtsFile + " 2>/dev/null";
    return system(command.c_str()) == 0;
#else
    std::cerr << "Warning: dtc not available - conversion not supported" << std::endl;
    return false;
#endif
}

bool validateDeviceTree(const std::string& filename) {
#ifdef HAVE_DTC
    // Use system dtc command for validation
    std::string command = "dtc -I dts -O dts " + filename + " >/dev/null 2>&1";
    return system(command.c_str()) == 0;
#else
    std::cerr << "Warning: dtc not available - using basic validation only" << std::endl;
    // Basic validation without dtc
    std::ifstream file(filename);
    return file.good();
#endif
}

std::string getDeviceTreeInfo(const std::string& filename) {
#ifdef HAVE_DTC
    // Use system dtc command to get info
    std::string command = "dtc -I dts -O dts " + filename + " 2>&1";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    pclose(pipe);
    return result;
#else
    return "dtc not available - limited information";
#endif
}

bool extractFromKernel(const std::string& kernelFile, const std::string& outputFile) {
    // Use system command to extract device tree from kernel
    std::string command = "scripts/extract-dtb.py -o " + outputFile + " " + kernelFile + " 2>/dev/null";
    return system(command.c_str()) == 0;
}

} // namespace DeviceTreeUtils

} // namespace DTE 