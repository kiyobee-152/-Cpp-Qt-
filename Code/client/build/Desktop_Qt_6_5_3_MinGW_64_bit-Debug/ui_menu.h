/********************************************************************************
** Form generated from reading UI file 'menu.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MENU_H
#define UI_MENU_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Menu
{
public:
    QStackedWidget *stackedWidget;
    QWidget *page;
    QPushButton *local_game_btn;
    QPushButton *exit_btn;
    QToolButton *about_btn;
    QPushButton *net_game_btn;
    QWidget *page_2;
    QTableView *tableView;
    QLineEdit *LineEdit;
    QLabel *connect_stat_label;
    QPushButton *create_btn;
    QPushButton *refresh_btn;
    QLabel *room_name_label;
    QPushButton *exit_net_btn;
    QPushButton *reconnect_btn;
    QLabel *people_label;

    void setupUi(QWidget *Menu)
    {
        if (Menu->objectName().isEmpty())
            Menu->setObjectName("Menu");
        Menu->resize(600, 600);
        stackedWidget = new QStackedWidget(Menu);
        stackedWidget->setObjectName("stackedWidget");
        stackedWidget->setGeometry(QRect(0, 0, 600, 600));
        stackedWidget->setMinimumSize(QSize(600, 600));
        page = new QWidget();
        page->setObjectName("page");
        page->setStyleSheet(QString::fromUtf8("#page{border-image: url(:/new/prefix1/img/menubg1.png);}"));
        local_game_btn = new QPushButton(page);
        local_game_btn->setObjectName("local_game_btn");
        local_game_btn->setGeometry(QRect(200, 180, 200, 80));
        QFont font;
        font.setFamilies({QString::fromUtf8("Agency FB")});
        font.setPointSize(14);
        font.setBold(true);
        local_game_btn->setFont(font);
        local_game_btn->setCursor(QCursor(Qt::ArrowCursor));
        local_game_btn->setAutoFillBackground(false);
        local_game_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        exit_btn = new QPushButton(page);
        exit_btn->setObjectName("exit_btn");
        exit_btn->setGeometry(QRect(200, 440, 200, 80));
        exit_btn->setFont(font);
        exit_btn->setAutoFillBackground(false);
        exit_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        exit_btn->setAutoDefault(false);
        about_btn = new QToolButton(page);
        about_btn->setObjectName("about_btn");
        about_btn->setGeometry(QRect(0, 0, 200, 30));
        about_btn->setAutoRaise(true);
        net_game_btn = new QPushButton(page);
        net_game_btn->setObjectName("net_game_btn");
        net_game_btn->setGeometry(QRect(200, 310, 200, 80));
        net_game_btn->setFont(font);
        net_game_btn->setCursor(QCursor(Qt::ArrowCursor));
        net_game_btn->setAutoFillBackground(false);
        net_game_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName("page_2");
        page_2->setStyleSheet(QString::fromUtf8("#page_2{border-image: url(:/new/prefix1/img/menubg1.png);}"));
        tableView = new QTableView(page_2);
        tableView->setObjectName("tableView");
        tableView->setGeometry(QRect(25, 5, 550, 400));
        tableView->setAutoFillBackground(false);
        LineEdit = new QLineEdit(page_2);
        LineEdit->setObjectName("LineEdit");
        LineEdit->setGeometry(QRect(120, 474, 300, 50));
        LineEdit->setMinimumSize(QSize(300, 50));
        LineEdit->setMaximumSize(QSize(300, 50));
        QFont font1;
        font1.setPointSize(10);
        LineEdit->setFont(font1);
        LineEdit->setMaxLength(64);
        LineEdit->setCursorPosition(8);
        connect_stat_label = new QLabel(page_2);
        connect_stat_label->setObjectName("connect_stat_label");
        connect_stat_label->setGeometry(QRect(20, 418, 400, 50));
        connect_stat_label->setMinimumSize(QSize(400, 50));
        connect_stat_label->setMaximumSize(QSize(400, 50));
        QFont font2;
        font2.setPointSize(16);
        font2.setBold(true);
        connect_stat_label->setFont(font2);
        connect_stat_label->setAlignment(Qt::AlignCenter);
        create_btn = new QPushButton(page_2);
        create_btn->setObjectName("create_btn");
        create_btn->setGeometry(QRect(426, 474, 150, 50));
        create_btn->setMinimumSize(QSize(150, 50));
        create_btn->setMaximumSize(QSize(150, 50));
        QFont font3;
        font3.setPointSize(16);
        font3.setBold(false);
        create_btn->setFont(font3);
        create_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        refresh_btn = new QPushButton(page_2);
        refresh_btn->setObjectName("refresh_btn");
        refresh_btn->setGeometry(QRect(426, 418, 150, 50));
        refresh_btn->setMinimumSize(QSize(150, 50));
        refresh_btn->setMaximumSize(QSize(150, 50));
        refresh_btn->setFont(font3);
        refresh_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        room_name_label = new QLabel(page_2);
        room_name_label->setObjectName("room_name_label");
        room_name_label->setGeometry(QRect(20, 474, 100, 50));
        QFont font4;
        font4.setPointSize(14);
        font4.setBold(true);
        room_name_label->setFont(font4);
        exit_net_btn = new QPushButton(page_2);
        exit_net_btn->setObjectName("exit_net_btn");
        exit_net_btn->setGeometry(QRect(426, 530, 150, 50));
        exit_net_btn->setMinimumSize(QSize(150, 50));
        exit_net_btn->setMaximumSize(QSize(150, 50));
        exit_net_btn->setFont(font3);
        exit_net_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        reconnect_btn = new QPushButton(page_2);
        reconnect_btn->setObjectName("reconnect_btn");
        reconnect_btn->setGeometry(QRect(20, 530, 400, 50));
        reconnect_btn->setMinimumSize(QSize(400, 50));
        reconnect_btn->setMaximumSize(QSize(400, 50));
        QFont font5;
        font5.setPointSize(19);
        font5.setBold(false);
        reconnect_btn->setFont(font5);
        reconnect_btn->setStyleSheet(QString::fromUtf8("background-image: url(:/new/prefix1/img/btnbg.jpg);"));
        people_label = new QLabel(page_2);
        people_label->setObjectName("people_label");
        people_label->setGeometry(QRect(430, 580, 141, 16));
        QFont font6;
        font6.setBold(true);
        people_label->setFont(font6);
        people_label->setTextFormat(Qt::AutoText);
        stackedWidget->addWidget(page_2);

        retranslateUi(Menu);

        stackedWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Menu);
    } // setupUi

    void retranslateUi(QWidget *Menu)
    {
        Menu->setWindowTitle(QCoreApplication::translate("Menu", "Menu", nullptr));
        local_game_btn->setText(QCoreApplication::translate("Menu", "\346\234\254\345\234\260\346\270\270\346\210\217", nullptr));
        exit_btn->setText(QCoreApplication::translate("Menu", "\351\200\200\345\207\272\346\270\270\346\210\217", nullptr));
        about_btn->setText(QCoreApplication::translate("Menu", "\345\205\263\344\272\216\351\241\271\347\233\256", nullptr));
        net_game_btn->setText(QCoreApplication::translate("Menu", "\347\275\221\347\273\234\346\270\270\346\210\217", nullptr));
        LineEdit->setText(QCoreApplication::translate("Menu", "XXX\346\243\213\347\216\213\344\272\211\351\234\270\350\265\233", nullptr));
        connect_stat_label->setText(QString());
        create_btn->setText(QCoreApplication::translate("Menu", "\345\210\233\345\273\272\346\210\277\351\227\264", nullptr));
        refresh_btn->setText(QCoreApplication::translate("Menu", "\345\214\271\351\205\215", nullptr));
        room_name_label->setText(QCoreApplication::translate("Menu", "\346\210\277\351\227\264\346\240\207\351\242\230\357\274\232", nullptr));
        exit_net_btn->setText(QCoreApplication::translate("Menu", "\350\277\224\345\233\236\344\270\273\351\241\265\351\235\242", nullptr));
        reconnect_btn->setText(QCoreApplication::translate("Menu", "\350\277\236\346\216\245\346\234\215\345\212\241\345\231\250", nullptr));
        people_label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Menu: public Ui_Menu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MENU_H
