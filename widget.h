#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <string>
#include <vector>
#include <QLabel>
#include <QComboBox>
#include <QDateEdit>
#include <QSpacerItem>
#define int64_t long long

extern const char* MYAPI;// = "a6c5c3bcb34c40c88c2c742725467c16";

struct options{
    char* str;
    char*id_document;
    char*id_request;
    char* country;
    char* date_publish;
    char* date_applicc;
    char* author;
    char* typesort;
    options();
};
struct mmdate{
    short year;
    short month;
    short day;
};
struct fastview_pattent{
    std::string number;
    std::string date_publish;
    std::string title;
    std::string descr;
    std::string inventor;
};
enum states{
    H_FAIL = -1, H_OK = 0
};
struct line_button{
    QPushButton*button;
    QLineEdit*line;
    line_button(QWidget*parent, const char*buttonText);
};
class searchWindow: public QObject{
    Q_OBJECT

    int sizeLay;
    line_button search;
    QPushButton*moreB;
    QGridLayout*slay;
    QLabel* authorL;
    QLineEdit* author_line;
    QLabel* countryL;
    QComboBox* numberChoice;
    QLineEdit* numberLine;
    QComboBox* countryChoice;
    QComboBox* datetypChoice;
    QDateEdit* dateChoice;
    std::vector<QWidget*> fast_turning;
    bool state;
public:
    QPushButton* getSearchB(){return search.button; }
    QLineEdit  * getSearchE(){return search.line; }
    bool fillOps(options&ops);
    bool getState()const{return state; }
    searchWindow(QWidget*w, QVBoxLayout*layout);
    void setAllVisible(bool b);
    void hide();
    void unhide();
public slots:
    void changeView();
};
class patentWidget;
class clickableLabel: public QLabel{
    patentWidget*daddy;
public:
    clickableLabel(const QString&str, patentWidget*d, QWidget*parent=nullptr):QLabel(str, parent), daddy(d){};
    void mouseReleaseEvent(QMouseEvent *event) override;
};

class patentWidget: public QWidget{
    clickableLabel*descr;
    clickableLabel*title;
    clickableLabel*date_publish;
    clickableLabel*inventor;
    QVBoxLayout*lay;
    QHBoxLayout*top;
    QHBoxLayout*bottom;
public:
    clickableLabel*number;
    patentWidget(const fastview_pattent&fp);
    QLabel* getTitle(){return title; }
    QLabel* getDatePublish(){return date_publish; }
    QLabel* getNumber(){return number; }
    void mouseReleaseEvent(QMouseEvent *event) override;
};

class searchScroll: public QScrollArea{
    QWidget*wlay;
    QVBoxLayout*lay;
    std::vector<patentWidget*> list;
public:
    searchWindow*search;
    void init_search_results(const std::vector<fastview_pattent>& fp);
    searchScroll(QWidget*parent, QVBoxLayout*screenLay);
    void hide(){
        this->QScrollArea::hide();
        search->hide();
    }
    void unhide(){
        this->QScrollArea::setVisible(true);
        search->unhide();
    }
};
class infoWindow{
    int id;
    std::string number;
    QHBoxLayout* top;
    QVBoxLayout* topRight;
    QGridLayout* numberGrid;
    QLabel* countryL;
    QLabel* numberL;
    QLabel* kindL;
    QLabel* countryN;
    QLabel* numberN;
    QLabel* kindN;
    QLabel* LT;
    QLabel* RT;
    QLabel* descr;
public:
    infoWindow():id(rand()){};
    int fillIt(const std::string&stdnumber);
    void addToLayout(QVBoxLayout*layoutTo);
};
class patentScroll: public QScrollArea{
    Q_OBJECT

public:
    infoWindow* iw;
    QWidget* container;
    QVBoxLayout* lay;
    QHBoxLayout* butts;
    QPushButton* menuB;
    QPushButton* closeB;
    patentScroll(const std::string&number);
public slots:
    void clickedClose();
    void clickedMenu();
};
class portalButton: public QPushButton{
    Q_OBJECT

public:
    bool active;
    patentScroll* myPatent;
    portalButton(patentScroll*myp):QPushButton(), myPatent(myp){
        connect((QPushButton*)this, &QPushButton::clicked, this, &portalButton::portalClicked);
    };
public slots:
    void portalClicked();
};
class portal{

    std::vector<portalButton*> buttons;
    QHBoxLayout* layout;
public:
    patentScroll* activePatent;
    QPushButton* at(int index);
    void createOne(bool isActive, patentScroll*ps);
    void makeNoneActive();
    void removePortalTo(patentScroll*ps);
    portal(QVBoxLayout*lay, patentScroll*ps);
};
class theScreen: public QWidget{
    Q_OBJECT

    QVBoxLayout*layout;
    searchScroll* mainScroll;
    std::vector<patentScroll*> addScrolls;
    portal*screenPortal;
public:
    theScreen();
    void clicked_pattern(const std::string&number);
    bool clicked_portal(patentScroll*ps);
    void removeScroll(patentScroll*ps);
public slots:
    void clicked_search();
};
extern theScreen* SCREEN;
int search_pattern(options ops, std::vector<fastview_pattent>&fillIt);
#endif // WIDGET_H
