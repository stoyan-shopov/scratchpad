#include "scratchpad-widget.hxx"
#include "ui_scratchpadwidget.h"

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
