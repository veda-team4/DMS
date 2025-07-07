#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <QWidget>

class BasePage : public QWidget {
  Q_OBJECT
public:
  BasePage(QWidget* parent = nullptr);
  virtual void activate();
  virtual void deactivate();
  virtual ~BasePage() = default;

signals:
  void moveToNext();
  void moveToPrevious();
};

#endif // BASEPAGE_H
