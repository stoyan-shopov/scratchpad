#include "scratchpad-widget.hxx"

#include <QDebug>

ScratchpadWidget::ScratchpadWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScratchpadWidget)
{
	ui->setupUi(this);
	connect(ui->pushButtonExecute, QPushButton::clicked, this, [=] { emit executeSforthCode(ui->plainTextEdit->toPlainText() + '\n'); });
}

ScratchpadWidget::~ScratchpadWidget()
{
	qDebug() << "destroyed";
	delete ui;
}
