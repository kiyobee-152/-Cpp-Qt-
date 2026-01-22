/********************************************************************************
** Form generated from reading UI file 'gamewin.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMEWIN_H
#define UI_GAMEWIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameWin
{
public:
    QLabel *chessboard;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QPushButton *back_btn;
    QPushButton *new_btn;
    QPushButton *exit_btn;
    QLabel *chess_label;

    void setupUi(QWidget *GameWin)
    {
        if (GameWin->objectName().isEmpty())
            GameWin->setObjectName("GameWin");
        GameWin->resize(1000, 800);
        GameWin->setMinimumSize(QSize(1000, 800));
        GameWin->setMaximumSize(QSize(1000, 800));
        chessboard = new QLabel(GameWin);
        chessboard->setObjectName("chessboard");
        chessboard->setGeometry(QRect(0, 0, 800, 800));
        QFont font;
        font.setPointSize(64);
        font.setBold(true);
        chessboard->setFont(font);
        chessboard->setTextFormat(Qt::AutoText);
        chessboard->setAlignment(Qt::AlignCenter);
        stackedWidget = new QStackedWidget(GameWin);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(750, 0, 250, 800));
        stackedWidget->setMinimumSize(QSize(250, 800));
        stackedWidget->setMaximumSize(QSize(250, 800));
        page = new QWidget();
        page->setObjectName("page");
        back_btn = new QPushButton(page);
        back_btn->setObjectName("back_btn");
        back_btn->setGeometry(QRect(50, 420, 150, 75));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("Agency FB")});
        font1.setPointSize(16);
        back_btn->setFont(font1);
        new_btn = new QPushButton(page);
        new_btn->setObjectName("new_btn");
        new_btn->setGeometry(QRect(50, 520, 150, 75));
        new_btn->setFont(font1);
        exit_btn = new QPushButton(page);
        exit_btn->setObjectName("exit_btn");
        exit_btn->setGeometry(QRect(50, 620, 150, 75));
        exit_btn->setFont(font1);
        chess_label = new QLabel(page);
        chess_label->setObjectName("chess_label");
        chess_label->setGeometry(QRect(75, 100, 100, 100));
        stackedWidget->addWidget(page);

        retranslateUi(GameWin);

        QMetaObject::connectSlotsByName(GameWin);
    } // setupUi

    void retranslateUi(QWidget *GameWin)
    {
        GameWin->setWindowTitle(QCoreApplication::translate("GameWin", "Form", nullptr));
        chessboard->setText(QString());
        back_btn->setText(QCoreApplication::translate("GameWin", "\346\202\224\346\243\213", nullptr));
        new_btn->setText(QCoreApplication::translate("GameWin", "\346\226\260\346\270\270\346\210\217", nullptr));
        exit_btn->setText(QCoreApplication::translate("GameWin", "\351\200\200\345\207\272\346\270\270\346\210\217", nullptr));
        chess_label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class GameWin: public Ui_GameWin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEWIN_H
