#include "scratchpad.hxx"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDockWidget>
#include <QUuid>
#include <QProcess>

#include <windows.h>

#include "scratchpad-widget.hxx"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	QFile f(":/stylesheet.txt");
	f.open(QFile::ReadOnly);
	setStyleSheet(f.readAll());
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	ui->plainTextEditScratchpad->setFont(font);
	readSettings();

	ui->treeViewFileSystem->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeViewFileSystem, & QTreeView::customContextMenuRequested, [=] (const QPoint & point) { qDebug() << "context menu requested"; });
	connect(ui->treeViewFileSystem, & QTreeView::activated, [=] (const QModelIndex &index) { qDebug() << "activated: " << fileSystemModel.fileName(index); fileActivated(index); } );
	connect(ui->treeViewFileSystem, & QTreeView::pressed, [=] (const QModelIndex &index) { qDebug() << "pressed: " << fileSystemModel.fileName(index); } );
	connect(ui->treeViewFileSystem, & QTreeView::clicked, [=] (const QModelIndex &index) { qDebug() << "clicked: " << fileSystemModel.fileName(index); } );
	fileSystemModel.setRootPath("");
	fileSystemModel.iconProvider()->setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
	ui->treeViewFileSystem->setModel(& fileSystemModel);

	connect(ui->comboBoxExplorerFavorites, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=](const QString & text){ favoritesItemSelected(text); });
	connect(ui->pushButtonAddPathToFavorites, & QPushButton::clicked, [=] { addPathToFavorites(); });
	connect(ui->pushButtonRunShell, & QPushButton::clicked, [=] { runShell(); });
	connect(ui->pushButtonRunExplorer, & QPushButton::clicked, [=] { runExplorer(); });

	scratchpad_server.listen("vgacon");
	sforth.start();
	scratchpad_server.waitForNewConnection(-1);
	scratchpad_socket = scratchpad_server.nextPendingConnection();
	scratchpad_server.close();
	connect(scratchpad_socket, & QLocalSocket::readyRead, [=]{ ui->plainTextEditSforth->appendPlainText(scratchpad_socket->readAll());});
	scratchpad_socket->write("unused . cr\n");

	QDir dir;
	sforth_executable.setFileName(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + getTargetExecutableFileName());

	dir.mkdir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
	QFile(getBundledExecutableFileName()).copy(sforth_executable.fileName());
	sforth_executable.setPermissions(static_cast<QFile::Permissions>(0x7777));
	connect(& sforth_process, & QProcess::readyReadStandardError, [=] { ui->plainTextEditSforth->appendPlainText(sforth_process.readAllStandardError()); });
	connect(& sforth_process, & QProcess::readyReadStandardOutput, [=] { ui->plainTextEditSforth->appendPlainText(sforth_process.readAllStandardOutput()); });
	connect(& sforth_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(sforthProcessFinished(int,QProcess::ExitStatus)));
	connect(ui->lineEditSforth, QLineEdit::returnPressed, this, [=] { sforth_process.write(ui->lineEditSforth->text().toLocal8Bit() + '\n'); sforth_process.waitForBytesWritten(); ui->lineEditSforth->clear(); });
	connect(ui->pushButtonNewScratchpad, QPushButton::clicked, this, [=] { createNewScratchpadDockWidget(); });
	connect(ui->pushButtonNewScratchpad, QPushButton::clicked, this, [=] { writeSettings(); });
	startSforthProcess();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::readSettings()
{
	QSettings s("shopov", "scratchpad");
	QStringList dockWidgets = s.value("dock-widgets", QStringList()).toStringList();

	qDebug() << "dock names" << dockWidgets;
	for (auto & d : dockWidgets)
		createNewScratchpadDockWidget(d, s.value(d + "-contents", QString()).toString());

	ui->plainTextEditScratchpad->setPlainText(s.value("scratchpad-contents", QString()).toString());
	restoreGeometry(s.value("mainwindow-geometry", QByteArray()).toByteArray());
	restoreState(s.value("mainwindow-state", QByteArray()).toByteArray());

	ui->comboBoxExplorerFavorites->addItems(s.value("filesystem-favorites", QStringList()).toStringList());
}

void MainWindow::writeSettings()
{
	QSettings s("shopov", "scratchpad");
	s.setValue("scratchpad-contents", ui->plainTextEditScratchpad->toPlainText());
	s.setValue("mainwindow-geometry", saveGeometry());
	s.setValue("mainwindow-state", saveState());

	QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
	qDebug() << "total dock widgets" << dockWidgets.size();
	QStringList dockWidgetNames;
	for (auto & d : dockWidgets)
		if (d == ui->dockWidgetScratchpad || d == ui->dockWidgetFileSystem)
			continue;
		else
		{
			dockWidgetNames << d->objectName();
			s.setValue(d->objectName() + "-contents", dynamic_cast<ScratchpadWidget *>(d->widget())->plainTextEdit()->toPlainText());
		}
	s.setValue("dock-widgets", dockWidgetNames);
	QStringList favorites;
	for (auto i = 0; i < ui->comboBoxExplorerFavorites->count(); favorites << ui->comboBoxExplorerFavorites->itemText(i ++));
	s.setValue("filesystem-favorites", favorites);
	qDebug() << "dock names" << dockWidgetNames;
}

void MainWindow::createNewScratchpadDockWidget(const QString & name, const QString & contents)
{
	auto dw = new QDockWidget("test", this);
	dw->setAttribute(Qt::WA_DeleteOnClose);
	dw->setAllowedAreas(Qt::AllDockWidgetAreas);
	auto w = new ScratchpadWidget(dw);
	dw->setWidget(w);
	w->plainTextEdit()->setPlainText(contents);
	connect(w, & ScratchpadWidget::executeSforthCode, [=](const QString & sforthCode) { sforth_process.write(sforthCode.toLocal8Bit() + '\n'); sforth_process.waitForBytesWritten(); });
	addDockWidget(Qt::TopDockWidgetArea, dw);
	auto n = name;
	if (name.isEmpty())
		n = QUuid::createUuid().toString();
	if (!n.startsWith("dock-widget-"))
		n.prepend("dock-widget-");
	dw->setObjectName(n);
}

void MainWindow::addPathToFavorites()
{
auto fileName = getSelectedFilename();
	if (fileName.length() && ui->comboBoxExplorerFavorites->findText(fileName) == -1)
		ui->comboBoxExplorerFavorites->addItem(fileName);
}

void MainWindow::runExplorer()
{
auto f = getSelectedFilename();
	if (f.isEmpty())
		return;
	QFileInfo fi(f);
	QStringList args;
	if (!QProcess::startDetached("explorer",
			args
				<< QString("/select,%1").arg(QDir::toNativeSeparators(fi.absoluteFilePath()))
				     ))
		QMessageBox::critical(0, "error running explorer", "failed to start explorer");
	qDebug() << "explorer run arguments:" << args;
}

/*! \todo	debug and clean this up */
void MainWindow::runShell()
{
auto f = getSelectedFilename();
	if (f.isEmpty())
		return;

	QProcess * p = new QProcess(this);
	p->setCreateProcessArgumentsModifier([] (QProcess::CreateProcessArguments *args)
	{
		args->flags |= CREATE_NEW_CONSOLE;
		args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
		args->startupInfo->dwFlags |= STARTF_USEFILLATTRIBUTE;
		args->startupInfo->dwFillAttribute = BACKGROUND_BLUE | FOREGROUND_RED
				| FOREGROUND_INTENSITY;
	});
	/*! c\todo	?!?!?! startDetached does not show the console window */
	p->setWorkingDirectory(QFileInfo(f).absolutePath());
	p->start("C:\\Windows\\System32\\cmd.exe", QStringList() << "/k" << "title" << "The Child Process");
	//p->start("C:\\Windows\\System32\\cmd.exe", QStringList());
#if 0
	QFileInfo fi(f);
	QStringList args;
	if (!p.startDetached("cmd",
			args
				//<< QString("/select,%1").arg(QDir::toNativeSeparators(fi.absoluteFilePath()))
				     , fi.absolutePath()))
		QMessageBox::critical(0, "error running shell", "failed to start shell");
	qDebug() << "shell run arguments:" << args;
#endif
}

void MainWindow::fileActivated(const QModelIndex &index)
{
	if (!fileSystemModel.fileInfo(index).isFile())
		return;
	if (!QProcess::startDetached("explorer",
			QStringList()
				<< QString("%1").arg(QDir::toNativeSeparators(fileSystemModel.fileInfo(index).absoluteFilePath()))
				     ))
		QMessageBox::critical(0, "error running explorer", "failed to start explorer");
}

void MainWindow::favoritesItemSelected(const QString &filePath)
{
auto i = fileSystemModel.index(filePath);
	if (i.isValid())
	{
		ui->treeViewFileSystem->setCurrentIndex(i);
		ui->treeViewFileSystem->scrollTo(i, QAbstractItemView::PositionAtTop);
		ui->treeViewFileSystem->ensurePolished();
		//ui->treeViewFileSystem->scrollTo(i);
	}
}

void MainWindow::sforthProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	qDebug() << sforth_process.state();
	if(exitStatus == QProcess::CrashExit && QMessageBox::critical(this, "sforth process crashed", "the sforth process crashed\ndo you want to restart it?", QMessageBox::Yes, QMessageBox::No)
				== QMessageBox::Yes)
		startSforthProcess();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	writeSettings();
	disconnect(& sforth_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(sforthProcessFinished(int,QProcess::ExitStatus)));
	sforth_process.kill();
	sforth_process.waitForFinished();
	scratchpad_socket->close();
	sforth.wait();
	QMainWindow::closeEvent(event);
}
