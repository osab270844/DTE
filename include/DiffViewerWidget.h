#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include "DeviceTreeDiff.h"

namespace DTE {

class DiffViewerWidget : public QWidget {
    Q_OBJECT
    
public:
    DiffViewerWidget(QWidget* parent = nullptr);
    void setDiff(const DeviceTreeDiff& diff);
    void clearDiff();
    
private slots:
    void onFilterChanged();
    void onExportClicked();
    
private:
    QVBoxLayout* layout_;
    QHBoxLayout* filterLayout_;
    QComboBox* filterTypeCombo_;
    QLineEdit* filterPathEdit_;
    QPushButton* exportButton_;
    QTableWidget* diffTable_;
    QLabel* statsLabel_;
    
    DeviceTreeDiff diff_;
    void populateDiffTable();
    void updateStats();
};

} // namespace DTE 