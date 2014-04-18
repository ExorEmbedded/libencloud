#include <QInputDialog>
#include <QLayout>
#include <QLineEdit>
#include "common.h"
#include "mainwindow.h"

//
// public methods
//

MainWindow::MainWindow()
    : _state(libencloud::StateNone)
    , _widget(NULL)
    , _layout(NULL)
    , _button(NULL)
    , _stateLabel(NULL)
    , _stateText(NULL)
    , _progressLabel(NULL)
    , _progressText(NULL)
    , _errorLabel(NULL)
    , _errorText(NULL)
    , _waitingForUser(false)
{
    SECE_TRACE;

    //
    // main widget
    //
    _widget = new QWidget(this);
    SECE_ERR_IF (_widget == NULL);

    _layout = new QVBoxLayout(_widget);
    SECE_ERR_IF (_layout == NULL);

    //
    // button
    //
    _button = new QPushButton(this);
    SECE_ERR_IF (_button == NULL);

    _button->setGeometry(QRect(QPoint(10,10), QSize(100,50)));
    _button->setText("Connect");
    _button->setDefault(true);
    _button->setAutoDefault(true);
    _layout->addWidget(_button);

    connect(_button, SIGNAL(released()), this, SIGNAL(toggle()));

    //
    // state label and text field
    //
    _stateLabel = new QLabel(this);
    _stateLabel->setText("state:");
    _layout->addWidget(_stateLabel);

    _stateText = new QTextEdit(this);
    SECE_ERR_IF (_stateText == NULL);
    _stateText->setDisabled(true);
    _stateText->setText(libencloud::stateToString(_state));
    _layout->addWidget(_stateText);

    //
    // progress label and text field
    //
    _progressLabel = new QLabel(this);
    _progressLabel->setText("progress:");
    _layout->addWidget(_progressLabel);

    _progressText = new QTextEdit(this);
    SECE_ERR_IF (_progressText == NULL);
    _progressText->setDisabled(true);
    _progressText->setText(_progress.toString());
    _layout->addWidget(_progressText);

    //
    // error label and text field
    //
    _errorLabel = new QLabel(this);
    _errorLabel->setText("error:");
    _layout->addWidget(_errorLabel);

    _errorText = new QTextEdit(this);
    SECE_ERR_IF (_errorText == NULL);
    _errorText->setDisabled(true);
    _layout->addWidget(_errorText);

    //
    // window settings
    //
    setFixedSize(300,300);
    setStyleSheet("background-color: white;");
    setCentralWidget(_widget);
    setWindowTitle("SECE Test");

err:
    return;
}

MainWindow::~MainWindow()
{
    SECE_TRACE;
}

//
// private slots
//

void MainWindow::_stateChanged (libencloud::State state)
{
    switch (state) 
    {
        case libencloud::StateIdle:
            _button->setText("Connect");
            setStyleSheet("background-color: white;");
            break;
        case libencloud::StateError:
            _button->setText("Disconnect");
            setStyleSheet("background-color: red;");
            break;
        case libencloud::StateSetup:
            _button->setText("Disconnect");
            setStyleSheet("background-color: yellow;");
            break;
        case libencloud::StateConnect:
            setStyleSheet("background-color: orange;");
            break;
        case libencloud::StateCloud:
            setStyleSheet("background-color: green;");
            break;
        default:
            break;
    }

    _stateText->setText(libencloud::stateToString(state));
}

void MainWindow::_gotError (const libencloud::Error &error)
{
    _error = error;

    if (!error.isValid())
        _errorText->setText("");
    else
        _errorText->setText(_error.getDesc() + "\n" +_error.getExtra());
}

void MainWindow::_gotProgress (const libencloud::Progress &progress)
{
    _progress = progress;

    _progressText->setText(_progress.toString());
}

void MainWindow::licenseRequest ()
{
    bool ok;
    QString lic;
    QUuid uuid;

    // avoid duplicate signals due to modal dialog
    if (_waitingForUser)
        return;

    _waitingForUser = true;

    do
    {
        lic = QInputDialog::getText(this,
                    tr("License required"),
                    tr("Please enter a valid license UUID:"),
                    QLineEdit::Normal,
                    "01234567-89ab-cdef-0123-456789abcdef",
                    &ok);

        uuid = QUuid(lic);
    } 
    while (ok && uuid.isNull());

    // if OK is pressed and license is valid, we send out license info to
    // Encloud Service via Setup API
    if (ok)
        emit licenseSupply(uuid);

    _waitingForUser = false;
}
