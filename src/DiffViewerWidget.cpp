#include "DiffViewerWidget.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>

namespace DTE {

DiffViewerWidget::DiffViewerWidget(QWidget* parent)
    : QWidget(parent), diff_(nullptr, nullptr) {
    
    // Create layout
    layout_ = new QVBoxLayout(this);
    
    // Create filter layout
    filterLayout_ = new QHBoxLayout();
    
    // Filter type combo
    filterTypeCombo_ = new QComboBox();
    filterTypeCombo_->addItem("All Changes", -1);
    filterTypeCombo_->addItem("Added", static_cast<int>(DiffType::Added));
    filterTypeCombo_->addItem("Removed", static_cast<int>(DiffType::Removed));
    filterTypeCombo_->addItem("Modified", static_cast<int>(DiffType::Modified));
    filterLayout_->addWidget(new QLabel("Type:"));
    filterLayout_->addWidget(filterTypeCombo_);
    
    // Filter path edit
    filterPathEdit_ = new QLineEdit();
    filterPathEdit_->setPlaceholderText("Filter by path...");
    filterLayout_->addWidget(new QLabel("Path:"));
    filterLayout_->addWidget(filterPathEdit_);
    
    // Export button
    exportButton_ = new QPushButton("Export");
    filterLayout_->addWidget(exportButton_);
    
    filterLayout_->addStretch();
    layout_->addLayout(filterLayout_);
    
    // Create diff table
    diffTable_ = new QTableWidget();
    diffTable_->setColumnCount(4);
    diffTable_->setHorizontalHeaderLabels({"Type", "Path", "Property", "Description"});
    diffTable_->setAlternatingRowColors(true);
    diffTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    diffTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Resize columns
    diffTable_->horizontalHeader()->setStretchLastSection(true);
    diffTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    diffTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    diffTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    
    layout_->addWidget(diffTable_);
    
    // Stats label
    statsLabel_ = new QLabel();
    layout_->addWidget(statsLabel_);
    
    // Connect signals
    connect(filterTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DiffViewerWidget::onFilterChanged);
    connect(filterPathEdit_, &QLineEdit::textChanged,
            this, &DiffViewerWidget::onFilterChanged);
    connect(exportButton_, &QPushButton::clicked,
            this, &DiffViewerWidget::onExportClicked);
}

void DiffViewerWidget::setDiff(const DeviceTreeDiff& diff) {
    diff_ = diff;
    populateDiffTable();
    updateStats();
}

void DiffViewerWidget::clearDiff() {
    diffTable_->setRowCount(0);
    statsLabel_->clear();
}

void DiffViewerWidget::onFilterChanged() {
    populateDiffTable();
}

void DiffViewerWidget::onExportClicked() {
    QString fileName = QFileDialog::getSaveFileName(this,
        "Export Diff", "",
        "JSON Files (*.json);;YAML Files (*.yaml);;Text Files (*.txt);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    std::string content;
    if (fileName.endsWith(".json")) {
        content = diff_.exportAsJSON();
    } else if (fileName.endsWith(".yaml") || fileName.endsWith(".yml")) {
        content = diff_.exportAsYAML();
    } else {
        content = diff_.exportAsPatch();
    }
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << QString::fromStdString(content);
        QMessageBox::information(this, "Export", "Diff exported successfully!");
    } else {
        QMessageBox::critical(this, "Export Error", "Failed to write file: " + fileName);
    }
}

void DiffViewerWidget::populateDiffTable() {
    diffTable_->setRowCount(0);
    
    auto changes = diff_.generateDiff();
    
    // Apply filters
    int filterType = filterTypeCombo_->currentData().toInt();
    QString filterPath = filterPathEdit_->text();
    
    std::vector<DiffEntry> filteredChanges;
    for (const auto& change : changes) {
        // Type filter
        if (filterType != -1 && static_cast<int>(change.type) != filterType) {
            continue;
        }
        
        // Path filter
        if (!filterPath.isEmpty() && 
            !QString::fromStdString(change.path).contains(filterPath, Qt::CaseInsensitive)) {
            continue;
        }
        
        filteredChanges.push_back(change);
    }
    
    // Populate table
    diffTable_->setRowCount(filteredChanges.size());
    
    for (int i = 0; i < filteredChanges.size(); ++i) {
        const auto& change = filteredChanges[i];
        
        // Type
        QString typeStr;
        QColor typeColor;
        switch (change.type) {
            case DiffType::Added:
                typeStr = "Added";
                typeColor = QColor(0, 128, 0); // Green
                break;
            case DiffType::Removed:
                typeStr = "Removed";
                typeColor = QColor(128, 0, 0); // Red
                break;
            case DiffType::Modified:
                typeStr = "Modified";
                typeColor = QColor(128, 128, 0); // Yellow
                break;
            default:
                typeStr = "Unknown";
                typeColor = QColor(128, 128, 128); // Gray
                break;
        }
        
        auto typeItem = new QTableWidgetItem(typeStr);
        typeItem->setBackground(typeColor);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        diffTable_->setItem(i, 0, typeItem);
        
        // Path
        auto pathItem = new QTableWidgetItem(QString::fromStdString(change.path));
        pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);
        diffTable_->setItem(i, 1, pathItem);
        
        // Property
        auto propItem = new QTableWidgetItem(QString::fromStdString(change.propertyName));
        propItem->setFlags(propItem->flags() & ~Qt::ItemIsEditable);
        diffTable_->setItem(i, 2, propItem);
        
        // Description
        auto descItem = new QTableWidgetItem(QString::fromStdString(change.description));
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        diffTable_->setItem(i, 3, descItem);
    }
}

void DiffViewerWidget::updateStats() {
    auto stats = diff_.getTotalChanges();
    QString statsText = QString("Total changes: %1 | Added: %2 | Removed: %3 | Modified: %4")
                       .arg(stats)
                       .arg(diff_.getAddedCount())
                       .arg(diff_.getRemovedCount())
                       .arg(diff_.getModifiedCount());
    
    statsLabel_->setText(statsText);
}

} // namespace DTE 