#pragma once

#include <QWidget>

namespace ggui {

/**
 * @brief About & Technical Architecture Page.
 * 
 * Displays project overview, academic team credits for Kathmandu Don Bosco College,
 * and detailed algorithm specifications for LZ77, Huffman, CRC32, and GCF1 format.
 */
class AboutPage : public QWidget {
    Q_OBJECT

public:
    explicit AboutPage(QWidget* parent = nullptr);
    ~AboutPage() override = default;

private:
    void setupUi();
};

}  // namespace ggui
