#include "MainWindow.h"
#include <QTreeWidget>

namespace DTE {

DeviceTreeWidgetItem::DeviceTreeWidgetItem(std::shared_ptr<DeviceTreeNode> node, QTreeWidget* parent)
    : QTreeWidgetItem(parent), node_(node) {
    updateDisplay();
}

void DeviceTreeWidgetItem::updateDisplay() {
    if (!node_) return;
    setText(0, QString::fromStdString(node_->getName()));
    // TODO: Add appropriate icons for different node types
    
    // Remove existing children
    while (childCount() > 0) {
        delete takeChild(0);
    }
    
    // Add new children
    for (const auto& child : node_->getChildren()) {
        auto childItem = new DeviceTreeWidgetItem(child, treeWidget());
        addChild(childItem);
    }
}

} // namespace DTE 