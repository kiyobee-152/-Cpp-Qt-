/********************************************************************************
** Form generated from reading UI file 'internet_game.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERNET_GAME_H
#define UI_INTERNET_GAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_internet_game
{
public:
    QStackedWidget *stackedWidget;
    QWidget *page_1;
    QPushButton *Button_prepare;
    QPushButton *Button_exit_room;
    QLabel *label_ip;
    QLabel *label_opponent;
    QLabel *label_prepare_;
    QLabel *label_fd;
    QWidget *page_2;
    QPushButton *btn_exit;
    QPushButton *btn_back;
    QPushButton *btn_surrender;
    QLabel *Label_your_color;
    QLineEdit *LE_send;
    QPushButton *Button_send;
    QLabel *label_msg;
    QTextEdit *LE_recv;
    QLabel *label;
    QPushButton *button_black;
    QPushButton *button_agree;
    QPushButton *button_refuse;
    QPushButton *button_white;
    QLabel *Title;
    QLabel *label_victory;
    QLabel *label_anwser;
    QLabel *label_prepare;
    QLabel *label_wait_answer;

    void setupUi(QWidget *internet_game)
    {
        if (internet_game->objectName().isEmpty())
            internet_game->setObjectName("internet_game");
        internet_game->resize(1000, 800);
        stackedWidget = new QStackedWidget(internet_game);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(750, 0, 250, 800));
        page_1 = new QWidget();
        page_1->setObjectName("page_1");
        Button_prepare = new QPushButton(page_1);
        Button_prepare->setObjectName("Button_prepare");
        Button_prepare->setGeometry(QRect(55, 480, 140, 75));
        Button_prepare->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        Button_exit_room = new QPushButton(page_1);
        Button_exit_room->setObjectName("Button_exit_room");
        Button_exit_room->setGeometry(QRect(55, 600, 140, 75));
        Button_exit_room->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        label_ip = new QLabel(page_1);
        label_ip->setObjectName("label_ip");
        label_ip->setGeometry(QRect(-5, 270, 250, 30));
        QFont font;
        font.setPointSize(16);
        font.setBold(false);
        font.setItalic(true);
        label_ip->setFont(font);
        label_ip->setAlignment(Qt::AlignCenter);
        label_opponent = new QLabel(page_1);
        label_opponent->setObjectName("label_opponent");
        label_opponent->setGeometry(QRect(50, 30, 150, 150));
        label_prepare_ = new QLabel(page_1);
        label_prepare_->setObjectName("label_prepare_");
        label_prepare_->setGeometry(QRect(50, 220, 150, 30));
        QFont font1;
        font1.setPointSize(20);
        font1.setBold(true);
        label_prepare_->setFont(font1);
        label_prepare_->setAlignment(Qt::AlignCenter);
        label_fd = new QLabel(page_1);
        label_fd->setObjectName("label_fd");
        label_fd->setGeometry(QRect(50, 320, 150, 30));
        label_fd->setFont(font);
        label_fd->setAlignment(Qt::AlignCenter);
        stackedWidget->addWidget(page_1);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        btn_exit = new QPushButton(page_2);
        btn_exit->setObjectName("btn_exit");
        btn_exit->setGeometry(QRect(50, 700, 150, 75));
        QFont font2;
        font2.setFamilies({QString::fromUtf8("Agency FB")});
        font2.setPointSize(16);
        btn_exit->setFont(font2);
        btn_exit->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        btn_back = new QPushButton(page_2);
        btn_back->setObjectName("btn_back");
        btn_back->setGeometry(QRect(50, 500, 150, 75));
        btn_back->setFont(font2);
        btn_back->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        btn_surrender = new QPushButton(page_2);
        btn_surrender->setObjectName("btn_surrender");
        btn_surrender->setGeometry(QRect(50, 600, 150, 75));
        btn_surrender->setFont(font2);
        btn_surrender->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        Label_your_color = new QLabel(page_2);
        Label_your_color->setObjectName("Label_your_color");
        Label_your_color->setGeometry(QRect(70, 310, 100, 100));
        LE_send = new QLineEdit(page_2);
        LE_send->setObjectName("LE_send");
        LE_send->setGeometry(QRect(30, 270, 130, 30));
        Button_send = new QPushButton(page_2);
        Button_send->setObjectName("Button_send");
        Button_send->setGeometry(QRect(160, 270, 50, 30));
        QFont font3;
        font3.setPointSize(9);
        font3.setBold(true);
        Button_send->setFont(font3);
        Button_send->setStyleSheet(QString::fromUtf8(""));
        label_msg = new QLabel(page_2);
        label_msg->setObjectName("label_msg");
        label_msg->setGeometry(QRect(35, 420, 180, 50));
        QFont font4;
        font4.setPointSize(14);
        font4.setBold(true);
        label_msg->setFont(font4);
        label_msg->setAlignment(Qt::AlignCenter);
        LE_recv = new QTextEdit(page_2);
        LE_recv->setObjectName("LE_recv");
        LE_recv->setGeometry(QRect(30, 10, 180, 250));
        LE_recv->setReadOnly(true);
        stackedWidget->addWidget(page_2);
        label = new QLabel(internet_game);
        label->setObjectName("label");
        label->setGeometry(QRect(0, 0, 800, 800));
        button_black = new QPushButton(internet_game);
        button_black->setObjectName("button_black");
        button_black->setGeometry(QRect(300, 60, 200, 200));
        QFont font5;
        font5.setPointSize(32);
        font5.setBold(true);
        button_black->setFont(font5);
        button_black->setStyleSheet(QString::fromUtf8("QPushButton{color:white;border-image: url(:/new/prefix1/img/kuro.png);}\n"
""));
        button_agree = new QPushButton(internet_game);
        button_agree->setObjectName("button_agree");
        button_agree->setGeometry(QRect(100, 330, 200, 150));
        button_agree->setFont(font5);
        button_agree->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        button_refuse = new QPushButton(internet_game);
        button_refuse->setObjectName("button_refuse");
        button_refuse->setGeometry(QRect(500, 330, 200, 150));
        button_refuse->setFont(font5);
        button_refuse->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        button_white = new QPushButton(internet_game);
        button_white->setObjectName("button_white");
        button_white->setGeometry(QRect(300, 540, 200, 200));
        button_white->setFont(font5);
        button_white->setStyleSheet(QString::fromUtf8("QPushButton{border-image: url(:/new/prefix1/img/shiro.png);}"));
        Title = new QLabel(internet_game);
        Title->setObjectName("Title");
        Title->setGeometry(QRect(150, 0, 600, 50));
        QFont font6;
        font6.setFamilies({QString::fromUtf8("Agency FB")});
        font6.setPointSize(20);
        font6.setBold(true);
        font6.setItalic(true);
        Title->setFont(font6);
        Title->setStyleSheet(QString::fromUtf8("QLabel{color:rgba(255, 0, 0, 1)}"));
        Title->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_victory = new QLabel(internet_game);
        label_victory->setObjectName("label_victory");
        label_victory->setGeometry(QRect(0, 0, 800, 200));
        QFont font7;
        font7.setPointSize(64);
        font7.setItalic(false);
        label_victory->setFont(font7);
        label_victory->setAlignment(Qt::AlignCenter);
        label_anwser = new QLabel(internet_game);
        label_anwser->setObjectName("label_anwser");
        label_anwser->setGeometry(QRect(0, 0, 800, 200));
        label_anwser->setFont(font7);
        label_anwser->setAlignment(Qt::AlignCenter);
        label_prepare = new QLabel(internet_game);
        label_prepare->setObjectName("label_prepare");
        label_prepare->setGeometry(QRect(0, 0, 800, 800));
        QFont font8;
        font8.setPointSize(64);
        font8.setBold(true);
        font8.setItalic(true);
        label_prepare->setFont(font8);
        label_prepare->setStyleSheet(QString::fromUtf8("QLabel{color:rgba(255, 0, 0, 0.6)}"));
        label_prepare->setTextFormat(Qt::AutoText);
        label_prepare->setAlignment(Qt::AlignCenter);
        label_wait_answer = new QLabel(internet_game);
        label_wait_answer->setObjectName("label_wait_answer");
        label_wait_answer->setGeometry(QRect(0, 0, 800, 800));
        QFont font9;
        font9.setPointSize(44);
        font9.setItalic(false);
        label_wait_answer->setFont(font9);
        label_wait_answer->setAlignment(Qt::AlignCenter);

        retranslateUi(internet_game);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(internet_game);
    } // setupUi

    void retranslateUi(QWidget *internet_game)
    {
        internet_game->setWindowTitle(QCoreApplication::translate("internet_game", "Form", nullptr));
        Button_prepare->setText(QCoreApplication::translate("internet_game", "\345\207\206\345\244\207", nullptr));
        Button_exit_room->setText(QCoreApplication::translate("internet_game", "\351\200\200\345\207\272\346\210\277\351\227\264", nullptr));
        label_ip->setText(QCoreApplication::translate("internet_game", "IP: ", nullptr));
        label_opponent->setText(QString());
        label_prepare_->setText(QCoreApplication::translate("internet_game", "\346\234\252\345\207\206\345\244\207", nullptr));
        label_fd->setText(QCoreApplication::translate("internet_game", "FD: ", nullptr));
        btn_exit->setText(QCoreApplication::translate("internet_game", "\351\200\200\345\207\272\346\270\270\346\210\217", nullptr));
        btn_back->setText(QCoreApplication::translate("internet_game", "\346\202\224\346\243\213", nullptr));
        btn_surrender->setText(QCoreApplication::translate("internet_game", "\350\256\244\350\276\223", nullptr));
        Label_your_color->setText(QString());
        Button_send->setText(QCoreApplication::translate("internet_game", "\345\217\221\351\200\201", nullptr));
        label_msg->setText(QString());
        label->setText(QString());
        button_black->setText(QCoreApplication::translate("internet_game", "\345\205\210\346\211\213", nullptr));
        button_agree->setText(QCoreApplication::translate("internet_game", "\345\220\214\346\204\217", nullptr));
        button_refuse->setText(QCoreApplication::translate("internet_game", "\346\213\222\347\273\235", nullptr));
        button_white->setText(QCoreApplication::translate("internet_game", "\345\220\216\346\211\213", nullptr));
        Title->setText(QCoreApplication::translate("internet_game", "\346\210\277\351\227\264\345\220\215", nullptr));
        label_victory->setText(QCoreApplication::translate("internet_game", "\351\273\221\346\243\213\350\203\234\345\210\251", nullptr));
        label_anwser->setText(QCoreApplication::translate("internet_game", "ADSFASDFA", nullptr));
        label_prepare->setText(QCoreApplication::translate("internet_game", "\350\257\267\345\207\206\345\244\207...", nullptr));
        label_wait_answer->setText(QCoreApplication::translate("internet_game", "\347\255\211\345\276\205\345\257\271\346\226\271\345\223\215\345\272\224\350\257\267\346\261\202", nullptr));
    } // retranslateUi

};

namespace Ui {
    class internet_game: public Ui_internet_game {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERNET_GAME_H
