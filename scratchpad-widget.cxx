#include "scratchpad-widget.hxx"

#include <QDebug>

ScratchpadWidget::ScratchpadWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScratchpadWidget)
{
	ui->setupUi(this);
}

ScratchpadWidget::~ScratchpadWidget()
{
	qDebug() << "destroyed";
	delete ui;
}
