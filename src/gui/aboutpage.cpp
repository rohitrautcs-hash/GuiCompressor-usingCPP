#include "aboutpage.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QFrame>

namespace ggui {

AboutPage::AboutPage(QWidget* parent)
    : QWidget(parent) {
    setupUi();
}

void AboutPage::setupUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 24, 28, 24);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background-color: transparent; border: none; }");

    auto* contentWidget = new QWidget(scroll);
    contentWidget->setStyleSheet("background-color: transparent;");

    auto* layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // Title Section
    auto* titleLabel = new QLabel("ABOUT & ARCHITECTURE", contentWidget);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #00f0ff; letter-spacing: 1.5px;");

    auto* descLabel = new QLabel("GUI Compressor v1.0 — Custom DEFLATE Lossless Folder Compressor", contentWidget);
    descLabel->setStyleSheet("color: #888899; font-size: 13px;");

    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);

    // Academic Team Card
    auto* teamCard = new QWidget(contentWidget);
    teamCard->setStyleSheet("background-color: #14141e; border: 1px solid #1a1a2e; border-radius: 10px;");
    auto* teamLayout = new QVBoxLayout(teamCard);
    teamLayout->setContentsMargins(20, 20, 20, 20);
    teamLayout->setSpacing(12);

    auto* teamHeader = new QLabel("🎓 ACADEMIC PROJECT TEAM", teamCard);
    teamHeader->setStyleSheet("color: #00f0ff; font-weight: bold; font-size: 14px; letter-spacing: 1px;");

    auto* teamSub = new QLabel("BIT 2nd Semester • Kathmandu Don Bosco College", teamCard);
    teamSub->setStyleSheet("color: #aaaaaa; font-size: 12px; margin-bottom: 8px;");

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(12);

    struct Member {
        QString name;
        QString role;
        QString desc;
    };

    const Member members[] = {
        {"Rohit Kumar Raut", "Team Lead & Core Engine", "Designed DEFLATE pipeline, LZ77 window, and archive format."},
        {"Anuj Kumar Sah", "Contributor", "Contributed to project testing, UI evaluation, and academic documentation."},
        {"Prativa Rimal", "Contributor", "Contributed to project testing, UI evaluation, and academic documentation."}
    };

    int row = 0;
    for (const auto& m : members) {
        auto* memberCard = new QWidget(teamCard);
        memberCard->setStyleSheet("background-color: #1a1a24; border: 1px solid #2a2a3a; border-radius: 8px;");
        auto* mLayout = new QVBoxLayout(memberCard);
        mLayout->setContentsMargins(14, 10, 14, 10);

        auto* nameL = new QLabel(m.name, memberCard);
        nameL->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 13px;");

        auto* roleL = new QLabel(m.role, memberCard);
        roleL->setStyleSheet("color: #39ff14; font-weight: 600; font-size: 11px;");

        auto* descL = new QLabel(m.desc, memberCard);
        descL->setStyleSheet("color: #888899; font-size: 11px;");
        descL->setWordWrap(true);

        mLayout->addWidget(nameL);
        mLayout->addWidget(roleL);
        mLayout->addWidget(descL);

        grid->addWidget(memberCard, row / 3, row % 3);
        row++;
    }

    teamLayout->addWidget(teamHeader);
    teamLayout->addWidget(teamSub);
    teamLayout->addLayout(grid);

    layout->addWidget(teamCard);

    // Algorithm Specs Card
    auto* specsCard = new QWidget(contentWidget);
    specsCard->setStyleSheet("background-color: #14141e; border: 1px solid #1a1a2e; border-radius: 10px;");
    auto* specsLayout = new QVBoxLayout(specsCard);
    specsLayout->setContentsMargins(20, 20, 20, 20);
    specsLayout->setSpacing(14);

    auto* specsHeader = new QLabel("⚙️ ALGORITHM & ARCHITECTURE SPECIFICATIONS", specsCard);
    specsHeader->setStyleSheet("color: #00f0ff; font-weight: bold; font-size: 14px; letter-spacing: 1px;");

    struct SpecItem {
        QString title;
        QString detail;
    };

    const SpecItem specs[] = {
        {"LZ77 Sliding Window Engine", "Employs a 4096-byte sliding window with 258-byte lookahead buffer and minimum match length of 3 bytes for dictionary compression."},
        {"Huffman Entropy Coder", "Constructs a canonical frequency tree using a min-heap priority queue. Features single-symbol root edge case protection."},
        {"IEEE 802.3 CRC-32 Verification", "Computes 32-bit cyclic redundancy check values using standard polynomial 0xEDB88320 lookup table for byte-for-byte integrity validation."},
        {"GCF1 Archive File Format", "Custom binary container specification featuring magic bytes ('GCF1'), atomic temporary file writes, folder metadata preservation, and automatic raw storage fallback for incompressible assets."}
    };

    for (const auto& s : specs) {
        auto* itemWidget = new QWidget(specsCard);
        itemWidget->setStyleSheet("background-color: #181824; border-left: 3px solid #00f0ff; border-radius: 4px;");
        auto* iLayout = new QVBoxLayout(itemWidget);
        iLayout->setContentsMargins(12, 8, 12, 8);

        auto* tLabel = new QLabel(s.title, itemWidget);
        tLabel->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 12px;");

        auto* dLabel = new QLabel(s.detail, itemWidget);
        dLabel->setStyleSheet("color: #aaaaaa; font-size: 11px;");
        dLabel->setWordWrap(true);

        iLayout->addWidget(tLabel);
        iLayout->addWidget(dLabel);

        specsLayout->addWidget(itemWidget);
    }

    specsLayout->insertWidget(0, specsHeader);
    layout->addWidget(specsCard);

    scroll->setWidget(contentWidget);
    rootLayout->addWidget(scroll);
}

}  // namespace ggui
