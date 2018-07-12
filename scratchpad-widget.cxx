#include "scratchpad-widget.hxx"
#include "ui_scratchpadwidget.h"

ScratchpadWidget::ScratchpadWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScratchpadWidget)
{
	ui->setupUi(this);
}

ScratchpadWidget::~ScratchpadWidget()
{
	delete ui;
}
