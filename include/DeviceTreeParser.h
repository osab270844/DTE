#ifndef DEVICETREEPARSER_H
#define DEVICETREEPARSER_H

#include "DeviceTree.h"
#include <memory>
#include <string>

namespace DTE {

// Base parser interface
class DeviceTreeParser {
public:
    virtual ~DeviceTreeParser() = default;
    virtual std::shared_ptr<DeviceTree> parse(const std::string& filename) = 0;
    virtual bool canParse(const std::string& filename) const = 0;
};

// DTB (Device Tree Blob) parser
class DTBParser : public DeviceTreeParser {
public:
    std::shared_ptr<DeviceTree> parse(const std::string& filename) override;
    bool canParse(const std::string& filename) const override;

private:
    bool parseDTBFile(const std::string& filename, std::shared_ptr<DeviceTree> tree);
    std::shared_ptr<DeviceTreeNode> parseDTBNode(const void* data, size_t offset, 
                                                size_t& nextOffset, size_t stringsOffset,
                                                bool isLittleEndian);
    DeviceTreeProperty parseDTBProperty(const void* data, size_t offset, 
                                       size_t& nextOffset, size_t stringsOffset,
                                       bool isLittleEndian);
};

// DTS (Device Tree Source) parser
class DTSParser : public DeviceTreeParser {
public:
    std::shared_ptr<DeviceTree> parse(const std::string& filename) override;
    bool canParse(const std::string& filename) const override;

private:
    bool parseDTSFile(const std::string& filename, std::shared_ptr<DeviceTree> tree);
    std::shared_ptr<DeviceTreeNode> parseDTSNode(std::istream& stream, std::string& line);
    DeviceTreeProperty parseDTSProperty(std::istream& stream, std::string& line);
    std::string parseDTSValue(std::istream& stream, std::string& line);
};

// Parser factory
class DeviceTreeParserFactory {
public:
    static std::unique_ptr<DeviceTreeParser> createParser(const std::string& filename);
    static std::vector<std::unique_ptr<DeviceTreeParser>> getAllParsers();
};

// Utility functions for device tree operations
namespace DeviceTreeUtils {
    bool dtbToDts(const std::string& dtbFile, const std::string& dtsFile);
    bool dtsToDtb(const std::string& dtsFile, const std::string& dtbFile);
    bool validateDeviceTree(const std::string& filename);
    std::string getDeviceTreeInfo(const std::string& filename);
    bool extractFromKernel(const std::string& kernelFile, const std::string& outputFile);
}

} // namespace DTE

#endif // DEVICETREEPARSER_H 