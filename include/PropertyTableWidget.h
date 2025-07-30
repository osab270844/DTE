#pragma once

#include <QTableWidget>
#include <QTableWidgetItem>
#include <memory>
#include "DeviceTree.h"

namespace DTE {

class PropertyTableWidget : public QTableWidget {
    Q_OBJECT
    
public:
    PropertyTableWidget(QWidget* parent = nullptr);
    void setNode(std::shared_ptr<DeviceTreeNode> node);
    void clearNode();
    
signals:
    void propertyChanged(const QString& propertyName, const QString& newValue);
    
private slots:
    void onItemChanged(QTableWidgetItem* item);
    
private:
    std::shared_ptr<DeviceTreeNode> currentNode_;
    void populateTable();
};

} // namespace DTE 