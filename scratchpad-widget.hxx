#ifndef SCRATCHPADWIDGET_HXX
#define SCRATCHPADWIDGET_HXX

#include <QWidget>

namespace Ui {
class ScratchpadWidget;
}

class ScratchpadWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ScratchpadWidget(QWidget *parent = 0);
	~ScratchpadWidget();

private:
	Ui::ScratchpadWidget *ui;
};

#endif // SCRATCHPADWIDGET_HXX
