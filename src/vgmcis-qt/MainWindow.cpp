/*
 * VGMCis (c) 2002-2019
 * Licensed under the zlib license,
 * refer to the included LICENSE.txt file
 */

#include <QFileInfo>
#include <QMimeData>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QFileDialog>

#include "QtVGMRoot.h"
#include "MainWindow.h"
#include "MusicPlayer.h"
#include "util/HeaderContainer.h"

#include "About.h"
#include "components/VGMMiscFile.h"
#include "components/seq/VGMSeq.h"
#include "components/instr/VGMInstrSet.h"
#include "components/VGMSampColl.h"

MainWindow::MainWindow() : QMainWindow(nullptr) {
    setWindowTitle("VGMCis");
    setWindowIcon(QIcon(":/images/logo.png"));

    setUnifiedTitleAndToolBarOnMac(true);
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::NoContextMenu);

    CreateElements();
    RouteSignals();

    m_statusbar->showMessage("Ready", 3000);
}

void MainWindow::CreateElements() {
    m_menu_bar = new MenuBar(this);
    setMenuBar(m_menu_bar);
    m_iconbar = new ToolBar(this);
    addToolBar(m_iconbar);
    m_mdiarea = MdiArea::Init();

    m_statusbar = new QStatusBar(this);
    m_statusbar_offset = new QLabel();
    m_statusbar_length = new QLabel();
    m_statusbar->addPermanentWidget(m_statusbar_offset);
    m_statusbar->addPermanentWidget(m_statusbar_length);
    setStatusBar(m_statusbar);

    m_rawfiles_list = new RawFileListView();
    auto *m_rawfiles_list_container =
        new HeaderContainer(m_rawfiles_list, QStringLiteral("Imported files"));

    m_vgmfiles_list = new VGMFilesList();
    auto *m_vgmfiles_list_container =
        new HeaderContainer(m_vgmfiles_list, QStringLiteral("Detected files"));

    m_colls_list = new VGMCollListView();
    auto *m_colls_list_container =
        new HeaderContainer(m_colls_list, QStringLiteral("Detected collections"));

    m_coll_view = new VGMCollView(m_colls_list->selectionModel());
    auto *m_coll_view_container =
        new HeaderContainer(m_coll_view, QStringLiteral("Selected collection"));

    auto coll_wrapper = new QWidget();
    auto coll_layout = new QGridLayout();
    coll_layout->addWidget(m_colls_list_container, 0, 0, 1, 1);
    coll_layout->addWidget(m_coll_view_container, 0, 1, 1, 3);
    coll_wrapper->setLayout(coll_layout);

    m_logger = new Logger();
    LogManager::instance().addSink(m_logger);
    addDockWidget(Qt::RightDockWidgetArea, m_logger);
    m_logger->hide();

    vertical_splitter = new QSplitter(Qt::Vertical, this);
    horizontal_splitter = new QSplitter(Qt::Horizontal, vertical_splitter);
    vertical_splitter_left = new QSplitter(Qt::Vertical, horizontal_splitter);

    vertical_splitter->addWidget(coll_wrapper);

    horizontal_splitter->addWidget(m_mdiarea);
    horizontal_splitter->setSizes(QList<int>() << 450 << 1200);

    vertical_splitter_left->addWidget(m_rawfiles_list_container);
    vertical_splitter_left->addWidget(m_vgmfiles_list_container);
    vertical_splitter_left->setContentsMargins(9, 0, 10, 0);

    setCentralWidget(vertical_splitter);
}

void MainWindow::RouteSignals() {
    connect(m_menu_bar, &MenuBar::OpenFile, this, &MainWindow::OpenFile);
    connect(m_menu_bar, &MenuBar::Exit, this, &MainWindow::close);
    connect(m_menu_bar, &MenuBar::ShowAbout, [=]() {
        About about(this);
        about.exec();
    });

    connect(m_iconbar, &ToolBar::OpenPressed, this, &MainWindow::OpenFile);
    connect(m_iconbar, &ToolBar::PlayToggle, m_colls_list, &VGMCollListView::HandlePlaybackRequest);
    connect(m_iconbar, &ToolBar::StopPressed, m_colls_list, &VGMCollListView::HandleStopRequest);
    connect(m_iconbar, &ToolBar::SeekingTo, &MusicPlayer::Instance(), &MusicPlayer::Seek);

    connect(&MusicPlayer::Instance(), &MusicPlayer::StatusChange, m_iconbar,
            &ToolBar::OnPlayerStatusChange);

    connect(m_menu_bar, &MenuBar::LoggerToggled,
            [=] { m_logger->setHidden(!m_menu_bar->IsLoggerToggled()); });
    connect(m_logger, &Logger::closeEvent, [=] { m_menu_bar->SetLoggerHidden(); });

    connect(m_vgmfiles_list, &VGMCollListView::clicked, [=] {
        if (!m_vgmfiles_list->currentIndex().isValid())
            return;

        auto clicked_item = qtVGMRoot.vVGMFile[m_vgmfiles_list->currentIndex().row()];

        std::visit([this](auto clicked_item) {
            m_statusbar_offset->setText("Offset: 0x" +
                                        QString::number(clicked_item->dwOffset, 16).toUpper());
            m_statusbar_length->setText("Length: 0x" +
                                        QString::number(clicked_item->unLength, 16).toUpper());
        }, clicked_item);
    });
}

void MainWindow::OpenFile() {
    auto filenames = QFileDialog::getOpenFileNames(
        this, "Select a file...", QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        "All files (*)");

    if (filenames.isEmpty())
        return;

    for (QString &filename : filenames) {
        m_statusbar->showMessage("Scanning \"" + filename + "\"...", 1000);
        qtVGMRoot.OpenRawFile(filename.toStdString());
    }

    m_statusbar->showMessage("Operation completed");
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    const auto &files = event->mimeData()->urls();

    if (files.isEmpty())
        return;

    for (const auto &file : files) {
        qtVGMRoot.OpenRawFile(QFileInfo(file.toLocalFile()).filePath().toStdString());
    }
}
