#ifndef CLIAPP_H
#define CLIAPP_H

#include "DeviceTree.h"
#include "DeviceTreeParser.h"
#include "DeviceTreeDiff.h"
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace DTE {

// CLI command structure
struct CLICommand {
    std::string name;
    std::string description;
    std::string usage;
    std::function<int(const std::vector<std::string>&)> handler;
};

// Main CLI application class
class CLIApp {
public:
    CLIApp(int argc, char* argv[]);
    int run();

private:
    // Command handlers
    int handleInfo(const std::vector<std::string>& args);
    int handleValidate(const std::vector<std::string>& args);
    int handleDiff(const std::vector<std::string>& args);
    int handleExport(const std::vector<std::string>& args);
    int handleConvert(const std::vector<std::string>& args);
    int handleSearch(const std::vector<std::string>& args);
    int handleList(const std::vector<std::string>& args);
    int handleHelp(const std::vector<std::string>& args);

    // Utility methods
    void setupCommands();
    bool loadDeviceTree(const std::string& filename);
    void printUsage() const;
    void printVersion() const;
    void printHelp(const std::string& command = "") const;
    void printDeviceTreeInfo() const;
    void printValidationResults() const;
    void printDiffResults(const DeviceTreeDiff& diff) const;
    void printTree(const std::shared_ptr<DeviceTreeNode>& node, const std::string& prefix = "") const;

    // Output methods
    void printError(const std::string& message) const;
    void printWarning(const std::string& message) const;
    void printSuccess(const std::string& message) const;
    void printInfo(const std::string& message) const;

    // Member variables
    std::string programName_;
    std::vector<std::string> args_;
    std::map<std::string, CLICommand> commands_;
    std::shared_ptr<DeviceTree> deviceTree_;
};

} // namespace DTE

#endif // CLIAPP_H 