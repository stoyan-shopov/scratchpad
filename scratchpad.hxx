#ifndef SCRATCHPAD_HXX
#define SCRATCHPAD_HXX

#include <QMainWindow>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QFile>
#include <QFileSystemModel>
#include <QFileIconProvider>

#include "sforth.hxx"
#include "ui_scratchpad.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	QLocalServer	scratchpad_server;
	QLocalSocket	* scratchpad_socket;
	SForth		sforth;
	QProcess	sforth_process;
	QFile		sforth_executable;
	void startSforthProcess(void) { sforth_process.start(sforth_executable.fileName()); sforth_process.write("true cr-echo !\n"); }
	void readSettings(void);
	void writeSettings(void);
	void createNewScratchpadDockWidget(const QString & name = QString(), const QString & contents = QString());
	QFileSystemModel fileSystemModel;
	void addPathToFavorites(void);
	void removePathFromFavorites(void);
	void runExplorer(void);
	void runShell(void);
	void fileActivated(const QModelIndex & index);
	void favoritesItemSelected(const QString & filePath);
	QString getSelectedFilename(void)
	{
		auto i = ui->treeViewFileSystem->currentIndex();
		QString fileName;
		if (i.isValid())
			fileName = fileSystemModel.fileInfo(i).absoluteFilePath();
		return fileName;
	}

	static const QString getBundledExecutableFileName(void)
	{
		return
#if defined Q_OS_WIN
	":/sf.exe"
#elif defined Q_OS_ANDROID
	":/sf-arm64"
#elif defined Q_OS_LINUX
#error "build the sforth executable for linux, and put it in the resources"
#else
#error "cannot identify build target"
#endif
		;
	}
	static const QString getTargetExecutableFileName(void)
	{
		return
#if defined Q_OS_WIN
	"sf.exe"
#elif defined Q_OS_ANDROID
	"sf-arm64"
#elif defined Q_OS_LINUX
#error "build the sforth executable for linux, and put it in the resources"
#else
#error "cannot identify build target"
#endif
		;
	}
private slots:
	void sforthProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
protected:
	void closeEvent(QCloseEvent * event);

};

#endif // SCRATCHPAD_HXX
