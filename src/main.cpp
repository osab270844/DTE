#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("Device Tree Explorer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DTE Project");
    app.setOrganizationDomain("dte-project.org");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    // Load translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);
    
    QTranslator appTranslator;
    appTranslator.load("dte_" + QLocale::system().name(),
                       QStandardPaths::locate(QStandardPaths::AppDataLocation, "translations"));
    app.installTranslator(&appTranslator);
    
    // Setup command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Device Tree Explorer - Visualize and edit Device Tree files");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Add command line options
    QCommandLineOption fileOption(QStringList() << "f" << "file",
                                  "Open device tree file", "filename");
    parser.addOption(fileOption);
    
    QCommandLineOption diffOption(QStringList() << "d" << "diff",
                                  "Compare with another device tree file", "filename");
    parser.addOption(diffOption);
    
    QCommandLineOption exportOption(QStringList() << "e" << "export",
                                    "Export as format (json/yaml/dts/dtb)", "format");
    parser.addOption(exportOption);
    
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                    "Output file for export", "filename");
    parser.addOption(outputOption);
    
    QCommandLineOption cliOption(QStringList() << "c" << "cli",
                                 "Run in CLI mode (no GUI)");
    parser.addOption(cliOption);
    
    QCommandLineOption validateOption(QStringList() << "v" << "validate",
                                      "Validate device tree and exit");
    parser.addOption(validateOption);
    
    // Parse command line arguments
    parser.process(app);
    
    // Check if running in CLI mode
    if (parser.isSet(cliOption)) {
        // TODO: Implement CLI mode
        qDebug() << "CLI mode not yet implemented";
        return 1;
    }
    
    // Load application settings
    QSettings settings;
    
    // Apply theme if specified
    QString theme = settings.value("appearance/theme", "default").toString();
    if (theme != "default") {
        app.setStyle(QStyleFactory::create(theme));
    }
    
    // Create and show main window
    DTE::MainWindow window;
    
    // Handle command line options
    if (parser.isSet(fileOption)) {
        QString filename = parser.value(fileOption);
        window.loadDeviceTree(filename);
    }
    
    if (parser.isSet(diffOption)) {
        QString diffFile = parser.value(diffOption);
        window.openDiffFile(diffFile);
    }
    
    // Handle export if specified
    if (parser.isSet(exportOption) && parser.isSet(outputOption)) {
        QString format = parser.value(exportOption);
        QString outputFile = parser.value(outputOption);
        
        // TODO: Implement export functionality
        qDebug() << "Export functionality not yet implemented";
    }
    
    // Handle validation if specified
    if (parser.isSet(validateOption)) {
        // TODO: Implement validation functionality
        qDebug() << "Validation functionality not yet implemented";
        return 0;
    }
    
    // Show the window
    window.show();
    
    // Set window position and size from settings
    QRect geometry = settings.value("window/geometry").toRect();
    if (geometry.isValid()) {
        window.setGeometry(geometry);
    }
    
    // Set window state from settings
    Qt::WindowStates state = static_cast<Qt::WindowStates>(
        settings.value("window/state", Qt::WindowNoState).toInt());
    window.setWindowState(state);
    
    return app.exec();
} 