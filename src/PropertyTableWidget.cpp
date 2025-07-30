#include "PropertyTableWidget.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMessageBox>

namespace DTE {

PropertyTableWidget::PropertyTableWidget(QWidget* parent)
    : QTableWidget(parent), currentNode_(nullptr) {
    
    // Set up table
    setColumnCount(3);
    setHorizontalHeaderLabels({"Property", "Type", "Value"});
    
    // Set table properties
    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::DoubleClicked);
    
    // Resize columns
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    
    // Connect signals
    connect(this, &QTableWidget::itemChanged, this, &PropertyTableWidget::onItemChanged);
}

void PropertyTableWidget::setNode(std::shared_ptr<DeviceTreeNode> node) {
    currentNode_ = node;
    populateTable();
}

void PropertyTableWidget::clearNode() {
    currentNode_ = nullptr;
    setRowCount(0);
}

void PropertyTableWidget::populateTable() {
    setRowCount(0);
    
    if (!currentNode_) {
        return;
    }
    
    auto properties = currentNode_->getProperties();
    setRowCount(properties.size());
    
    for (int i = 0; i < properties.size(); ++i) {
        const auto& prop = properties[i];
        
        // Property name
        auto nameItem = new QTableWidgetItem(QString::fromStdString(prop.getName()));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        setItem(i, 0, nameItem);
        
        // Property type
        QString typeStr;
        if (prop.isString()) {
            typeStr = "String";
        } else if (prop.isBinary()) {
            typeStr = "Binary";
        } else if (prop.isCells()) {
            typeStr = "Cells";
        } else if (prop.isCells64()) {
            typeStr = "Cells64";
        } else {
            typeStr = "Unknown";
        }
        
        auto typeItem = new QTableWidgetItem(typeStr);
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        setItem(i, 1, typeItem);
        
        // Property value
        auto valueItem = new QTableWidgetItem(QString::fromStdString(prop.getValueAsString()));
        setItem(i, 2, valueItem);
    }
}

void PropertyTableWidget::onItemChanged(QTableWidgetItem* item) {
    if (!currentNode_ || !item) {
        return;
    }
    
    int row = item->row();
    int column = item->column();
    
    if (column == 2) { // Value column
        QString propertyName = this->item(row, 0)->text();
        QString newValue = item->text();
        
        emit propertyChanged(propertyName, newValue);
    }
}

} // namespace DTE 