#ifndef SCRATCHPADWIDGET_HXX
#define SCRATCHPADWIDGET_HXX

#include <QWidget>
#include "ui_scratchpadwidget.h"

namespace Ui {
class ScratchpadWidget;
}

class ScratchpadWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScratchpadWidget(QWidget *parent = 0);
	~ScratchpadWidget();
	QPlainTextEdit * plainTextEdit(void) { return ui->plainTextEdit; }

signals:
	void executeSforthCode(const QString & sforthCode);
private:
	Ui::ScratchpadWidget *ui;
};

#endif // SCRATCHPADWIDGET_HXX
