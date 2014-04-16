#ifndef _SECE_MAINWINDOW_H_
#define _SECE_MAINWINDOW_H_

#include <QObject>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <encloud/Common>
#include <encloud/Error>
#include <encloud/Progress>
#include <encloud/State>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow ();
    ~MainWindow ();

signals:
    void toggle ();

private slots:
    void _stateChanged (libencloud::State state);
    void _gotError (const libencloud::Error &error);
    void _gotProgress (const libencloud::Progress &error);
    void _gotNeed (const QString &need);

private:
    libencloud::State _state;
    libencloud::Progress _progress;
    libencloud::Error _error;

    QWidget *_widget;
    QVBoxLayout *_layout;
    QPushButton *_button;
    QLabel *_stateLabel;
    QTextEdit *_stateText;
    QLabel *_progressLabel;
    QTextEdit *_progressText;
    QLabel *_errorLabel;
    QTextEdit *_errorText;
};

#endif  // _SECE_MAINWINDOW_H_
