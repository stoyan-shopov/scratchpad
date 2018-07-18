#include "scratchpad.hxx"
#include "ui_scratchpad.h"

#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDockWidget>
#include <QUuid>

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

	scratchpad_server.listen("vgacon");
	sforth.start();
	scratchpad_server.waitForNewConnection(-1);
	scratchpad_socket = scratchpad_server.nextPendingConnection();
	scratchpad_server.close();
	//connect(vgacon_socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
	scratchpad_socket->write("unused . cr\n");

	QDir dir;
	sforth_executable.setFileName(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + getTargetExecutableFileName());

	dir.mkdir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
	QFile(getBundledExecutableFileName()).copy(sforth_executable.fileName());
	sforth_executable.setPermissions(static_cast<QFile::Permissions>(0x7777));
	connect(& sforth_process, & QProcess::readyReadStandardError, [=] { ui->sforthTextEdit->appendPlainText(sforth_process.readAllStandardError()); });
	connect(& sforth_process, & QProcess::readyReadStandardOutput, [=] { ui->sforthTextEdit->appendPlainText(sforth_process.readAllStandardOutput()); });
	connect(& sforth_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(sforthProcessFinished(int,QProcess::ExitStatus)));
	connect(ui->sforthLineEdit, QLineEdit::returnPressed, this, [=] { sforth_process.write(ui->sforthLineEdit->text().toLocal8Bit() + '\n'); sforth_process.waitForBytesWritten(); ui->sforthLineEdit->clear(); });
	connect(ui->pushButtonNewScratchpad, QPushButton::clicked, this, [=] { createNewScratchpadDockWidget(); });
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
		if (d == ui->dockWidgetScratchpad)
			continue;
		else
		{
			dockWidgetNames << d->objectName();
			s.setValue(d->objectName() + "-contents", dynamic_cast<ScratchpadWidget *>(d->widget())->plainTextEdit()->toPlainText());
		}
	s.setValue("dock-widgets", dockWidgetNames);
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
