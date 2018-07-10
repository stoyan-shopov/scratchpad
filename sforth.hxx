#ifndef SFORTH_HXX
#define SFORTH_HXX

#include <QThread>

extern class SForth : public QThread
{
	Q_OBJECT
	void run(void) override;
public:
	SForth(void);
} sforth;

#endif // SFORTH_HXX
