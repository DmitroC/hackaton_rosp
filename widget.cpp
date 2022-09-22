#include "widget.h"
#include "ui_widget.h"
#include <curl/curl.h>
#include <string.h>

theScreen* SCREEN;
#define BACKGROUND_COLOR_PATIENT "background-color: #C0C0C0; border-radius: 5px;"
#define OPEN_TAG "<span style=$background-color: #A0A0A0;$>"
#define CLOSE_TAG "</span>"
#define PORTAL_BUTTON_ACTIVE "background-color: #aed6f1; border-radius: 10px; border: 1px solid #212f3d;"// border-radius: 1px; border: 1px solid #212f3d;
#define PORTAL_BUTTON_INACTIVE "background-color: #f4f6f6; border-radius: 10px; border: 1px solid #212f3d; "
#define SIZE_PARSE_LANGS 3
const char*parse_langs[SIZE_PARSE_LANGS]{"\"ru\"", "\"en\"", "\"uk\""};

options::options(){
    date_publish = date_applicc = id_document = id_request = typesort= str = author = country = nullptr;
}
size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}
const char* findStart(const char*str, char c){
    const char*p = str;
    for(;(*p)!='\0';p++){
        if((*p) == c)
            return p;
    }
    return NULL;
}
const char* findEnd(const char*str, char c){
    const char*p = str;
    for(;(*p)!='\0';p++){
        if((*p) == c)
            return p;
    }
    return NULL;
}
const char* passUntill(const char*str){
    const char*p = str;
    int qtS = 1;
    if((*p) != '{')
        return nullptr;
    p++;
    for(;(*p)!='\0';p++){
        if((*p) == '{')
            qtS++;
        else if((*p) == '}'){
            qtS--;
            if(qtS == 0)
                return p;
        }
    }
    return nullptr;
}
void filter_add(std::string&filter, const std::string&str_to_add){
    if(filter.empty())
        filter = "\"filter\": {" + str_to_add;
    else
        filter+="," + str_to_add;
}
void grab_string(const char*p, const char*find, std::string&strtemp, const char*(*funcstrfind)(const char*, const char*)){
    strtemp = "";
    const char* g = funcstrfind(p,find);
    if(!g)
        return;
    for(;(*g) && (*g)!=':';g++);
    for(;(*g) && (*g)!='"';g++);
    if((*g)=='\0')
        return;
    g++;
    for(;(*g) && (*g)!='"';g++){
        if((*g) == '$')
            strtemp+='"';
        else
            strtemp+=(*g);
    }
}
int compare(const char*X, const char*Y){
    while(*X && *Y){
        if(*X != *Y)
            return 0;
        X++;
        Y++;
    }
    return (*Y == '\0');
}
std::string deleteTags(const std::string&str){
    std::string out;
    const char*p  =str.c_str();
    for(;*p != '\0';p++){
        if(*p == '<')
            if((*(p+1)  == '/' && compare(p, CLOSE_TAG)) || compare(p, OPEN_TAG)){
                while(*p != '>')
                    p++;
                continue;
            }
        out+=*p;
    }
    return out;
}
const char * backstr (const char * g, const char * find) {
    for(;;g--){
        if(*g == *find)
            if(compare(g, find))
                return g;
    }
    return nullptr;
}
void replaceTags(const std::string&str, std::string& newstr){
    const char*p = str.c_str();
    for(;*p != '\0';p++){
        if(*p == '<'){
            if(*(p+1)  == '/' && compare(p, "</em>")){
                while(*p != '>')
                    p++;
                newstr+=CLOSE_TAG;
                continue;
            }
            else if(compare(p, "<em>")){//yes, it's <em>
                while(*p != '>')
                    p++;
                newstr+=OPEN_TAG;
                continue;
            }
        }
        newstr+=*p;
    }
}
#include <stdio.h>
int search_pattern(options ops, std::vector<fastview_pattent>&fillIt){
    std::string poststr = "{";
    std::string filter = "";
    if(ops.str)
        poststr+=std::string("\"q\":") + std::string("\"") + std::string(ops.str) + std::string("\"");
    if(ops.country)
        filter_add(filter, std::string("\"country\":{\"values\":[") + std::string("\"") + std::string(ops.country) + std::string("\"]}"));
    if(ops.author)
        filter_add(filter, "\"authors\":" + std::string("\"") + std::string(ops.author) + std::string("\""));
    if(ops.date_publish)
        filter_add(filter, "\"date_published\":" + std::string("\"") + std::string(ops.date_publish) + std::string("\""));
    if(ops.date_applicc)
        filter_add(filter, "\"application.filing_date\":" + std::string("\"") + std::string(ops.date_applicc) + std::string("\""));
    if(!filter.empty()){
        filter+="}";
        if(poststr.empty())
            poststr+=filter;
        else
            poststr+="," + filter;
    }
    poststr+="}";
    CURL*curl = curl_easy_init();
    std::string data;
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, "https://searchplatform.rospatent.gov.ru/patsearch/v0.2/search");
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        struct curl_slist *headers= NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer a6c5c3bcb34c40c88c2c742725467c16");
        headers= curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, poststr.c_str());
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    else
        return H_FAIL;
    std::string newdata;
    newdata.reserve(data.size());
    replaceTags(data, newdata);
    data.clear();
    /*FILE*f = fopen("/home/rainbow/deleteit", "w");
    fwrite((void*)newdata.c_str(),  1, newdata.size(), f);
    fclose(f);*/
    const char*p = findStart(newdata.c_str(), '[');
    if(p == NULL)
        return H_FAIL;
    for(;;){
        for(;(*p)!='{';p++)
            if((*p) == ']')
                return H_OK;
        fillIt.push_back(fastview_pattent());
        fastview_pattent& fpat = fillIt[fillIt.size()-1];
        grab_string(p, "publication_date", fpat.date_publish, strstr);
        const char* g = strstr(p, "snippet");

        grab_string(g, "title", fpat.title, strstr);
        grab_string(g, "descr", fpat.descr, strstr);
        grab_string(g, "inventor", fpat.inventor, strstr);
        grab_string(g, "\"id\"", fpat.number, backstr);
        p = passUntill(p);
    }
}
void grab_manystrings(QString&textTo, const char**infpp, const char*comp){
    const char*infpointer = *infpp;
    for(;(*infpointer)&&(*infpointer)!='[';infpointer++);
    for(;(*infpointer)&&(*infpointer)!=']';infpointer++){
        if(*infpointer == *comp){
            const char* gc = comp;
            while(*infpointer && *gc){
                if(*infpointer != *gc || *infpointer == ']')
                    break;
                infpointer++;
                gc++;
            }
            if(*infpointer == ']')
                break;
            if(*gc == '\0'){
                for(;(*infpointer)&&(*infpointer)!=':';infpointer++);
                for(;(*infpointer)&&(*infpointer)!='"';infpointer++);
                if((*infpointer) == '\0')
                    break;
                infpointer++;
                std::string name = "";
                for(;(*infpointer)&&(*infpointer)!='"';infpointer++)
                    name+=(*infpointer);
                textTo+=QString::fromStdString(name) + QString("<br>");
            }
        }
    }
    *infpp = infpointer;

}
void clickableLabel::mouseReleaseEvent(QMouseEvent *event){
    SCREEN->clicked_pattern(daddy->number->text().toStdString());
}

int infoWindow::fillIt(const std::string&stdnumber){
    this->number = stdnumber;
    std::string link = "https://searchplatform.rospatent.gov.ru/patsearch/v0.2/docs/" + number;
    CURL* curl = curl_easy_init();
    std::string data;
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        struct curl_slist *headers= NULL;
        headers = curl_slist_append(headers, "Authorization: Bearer a6c5c3bcb34c40c88c2c742725467c16");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        CURLcode res = curl_easy_perform(curl);
        if(res!=CURLE_OK)
            return H_FAIL;
        curl_easy_cleanup(curl);
    }
    else{
        return H_FAIL;
    }
    QString leftText = "";
    QString rightText = "";
    QString description = "";
    std::string strtemp;
    const char* p = data.c_str();//...
    const char* appl = strstr(p, "\"application\"");
    const char* MPK = strstr(p, "\"ipc\"");
    if(MPK){
        grab_string(MPK, "\"fullname\"", strtemp, strstr);
        rightText+= QString("<br><p><b>МПК: </b>") + QString::fromStdString(strtemp) + QString("</p>");
    }
    if(appl){
        grab_string(appl, "number", strtemp, strstr);
        rightText+= QString("<p><b>Заявка: </b>") +QString::fromStdString(strtemp) + QString("</p><br>");
        grab_string(appl, "filing_date",strtemp, strstr);
        rightText+= QString("<p><b>Дата заявки: </b>") + QString(strtemp.c_str()) + QString("</p>");
    }
    strtemp = "";
    grab_string(appl, "\"publication_date\"", strtemp, strstr);
    rightText+= "<p><b>Дата публикации:</b>  " + QString::fromStdString(strtemp) + "</p>";
    const char* biblio = strstr(p, "\"biblio\"");
    if(biblio){
        const char* infpointer;
        for(int ji = 0; ji < SIZE_PARSE_LANGS; ji++){
            infpointer = strstr(biblio, parse_langs[ji]);
            if(infpointer){
                std::string strtemp = "";
                grab_string(infpointer, "\"title\"", strtemp, strstr);
                leftText = QString("<p style=\"font-size: 22px; color: green;\"><b>")
                        + QString::fromStdString(strtemp) + QString("</b></p><br><br>");
                leftText+="<p><b>Авторы:</b></p>";
                grab_manystrings(leftText, &infpointer, "\"name\"");
                leftText+="<p><b>Патентообладатели:</b></p>";
                grab_manystrings(leftText, &infpointer, "\"name\"");
                leftText+="<p><b>Заявители:</p></b>";
                grab_manystrings(leftText, &infpointer, "\"name\"");
                break;
            }
        }
    }
    const char* abstract = strstr(p, "\"abstract\"");
    if(abstract){
        const char* IAmTired[3]{"\"description\"", "\"claims\"", "@"};
        std::string toAdd[3]{"<p><b>Описание</b></p>", "<p><b>Требования: </b></p>", "<p><b>Абстрактно: </b></p>"};
        for(int tired = 0; tired < 3; tired++){
            const char*go;
            if(tired != 2)
                go = strstr(abstract, IAmTired[tired]);
            else
                go = abstract;
            if(go){
                for(int ji = 0; ji < SIZE_PARSE_LANGS; ji++){
                    const char* gogo = strstr(go, parse_langs[ji]);
                    if(gogo){
                        for(;(*gogo) && (*gogo)!=':';gogo++);
                        for(;(*gogo) && (*gogo)!='"';gogo++);
                        if((*gogo) == '\0')
                            break;
                        std::string strtemp = "";
                        gogo++;
                        bool notS = false;
                        for(;;gogo++){
                            if((*gogo) == '\\')
                                notS = true;
                            else{
                                if((*gogo) == '"' && !notS)
                                    break;
                                strtemp+=(*gogo);
                                notS = false;
                            }
                        }
                        description+= QString::fromStdString(toAdd[tired] + strtemp);
                        break;
                    }
                }
            }
        }
    }
    QString qtemp = "";
    const char*npointer = number.c_str();
    for(;((*npointer) > 'A' && (*npointer) < 'Z') or ((*npointer) > 'a' && (*npointer) < 'z'); npointer++)
        qtemp+=(*npointer);
    countryL = new QLabel(" C");
    countryL->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    countryL->setStyleSheet("color: #4d5656 ; font-size: 14px; ");
    countryN = new QLabel(qtemp);
    countryN->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    countryN->setStyleSheet("color: #00516f; font-size: 18px; ");
    qtemp = "";
    for(;((*npointer) >= '0' && (*npointer) <= '9'); npointer++)
        qtemp+=(*npointer);
    numberL = new QLabel("  Number");
    numberL->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    numberL->setStyleSheet("color: #4d5656 ; font-size: 14px; ");
    numberN = new QLabel(qtemp);
    numberN->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    numberN->setStyleSheet("color: #00516f; font-size: 18px; ");
    qtemp = "";
    for(;(*npointer)!='_'; npointer++)
        qtemp+=(*npointer);
    kindL = new QLabel(" K");
    kindL->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    kindL->setStyleSheet("color: #4d5656 ; font-size: 14px;");
    kindN = new QLabel(qtemp);
    kindN->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    kindN->setStyleSheet("color: #00516f; font-size: 18px;");
    numberGrid= new QGridLayout();
    numberGrid->addWidget(countryL, 0, 0);
    numberGrid->addWidget(numberL, 0, 1);
    numberGrid->addWidget(kindL, 0, 2);
    numberGrid->addWidget(countryN, 1, 0);
    numberGrid->addWidget(numberN, 1, 1);
    numberGrid->addWidget(kindN, 1, 2);
    numberGrid->setColumnStretch(3, 1);
    top = new QHBoxLayout();
    topRight = new QVBoxLayout();
    LT = new QLabel(leftText);
    LT->setWordWrap(true);
    RT = new QLabel(rightText);
    RT->setAlignment(Qt::AlignTop);
    descr = new QLabel(description);
    descr->setWordWrap(true);
    topRight->addLayout(numberGrid);
    topRight->addWidget(RT);
    top->addWidget(LT);
    top->addLayout(topRight);
    return H_OK;
}
void infoWindow::addToLayout(QVBoxLayout*layoutTo){
    layoutTo->addLayout(top);
    layoutTo->addWidget(descr);
}
line_button::line_button(QWidget*parent, const char*buttonText){
    this->button = new QPushButton(buttonText, parent);
    this->line = new QLineEdit(parent);
}
searchWindow::searchWindow(QWidget*w, QVBoxLayout*layout):search(w, "Search"){
    moreB = new QPushButton("More", w);
    slay = new QGridLayout(w);
    slay->addWidget(search.line, 0, 1);
    slay->addWidget(search.button, 0, 2);
    slay->addWidget(moreB, 0, 3);

    authorL = new QLabel("Автор: ");
    author_line = new QLineEdit();
    countryL = new QLabel("Страна: ");
    countryChoice = new QComboBox();
    countryChoice->addItem("ALL");
    countryChoice->addItem("RU");
    countryChoice->addItem("EN");
    countryChoice->setCurrentIndex(0);
    datetypChoice = new QComboBox();
    datetypChoice->addItem("Дата публикации документа");
    datetypChoice->addItem("Дата заявки");
    dateChoice = new QDateEdit();
    dateChoice->setDisplayFormat("dd.MM.yyyy");
    dateChoice->setMinimumDate(QDate(2077, 1, 1));
    numberChoice = new QComboBox();
    numberChoice->addItem("Номер документа");
    numberChoice->addItem("Номер заявки");
    numberLine = new QLineEdit();
    slay->addWidget(authorL, 1, 0);
    slay->addWidget(author_line, 1, 1);
    slay->addWidget(countryL, 1, 2);
    slay->addWidget(countryChoice, 1, 3);
    slay->addWidget(dateChoice, 2, 0);
    slay->addWidget(datetypChoice, 2, 1);
    slay->addWidget(numberChoice);
    slay->addWidget(numberLine);
    fast_turning.push_back(authorL);
    fast_turning.push_back(author_line);
    fast_turning.push_back(countryL);
    fast_turning.push_back(countryChoice);
    fast_turning.push_back(datetypChoice);
    fast_turning.push_back(dateChoice);
    fast_turning.push_back(numberChoice);
    fast_turning.push_back(numberLine);
    for(int i = 0; i < fast_turning.size(); i++)
        fast_turning[i]->setVisible(false);
    layout->addLayout(slay);
    this->state = false;
    connect(moreB, &QPushButton::clicked, this, &searchWindow::changeView);
}
void searchWindow::setAllVisible(bool visible){
    search.button->setVisible(visible);
    search.line->setVisible(visible);
    moreB->setVisible(visible);
    if(state){
        for(int i = 0; i < fast_turning.size(); i++)
            fast_turning[i]->setVisible(visible);
    }
}
void searchWindow::hide(){
    setAllVisible(false);
}
void searchWindow::unhide(){
    setAllVisible(true);
}
void searchWindow::changeView(){
    this->state^=1;
    for(int i = 0; i < fast_turning.size(); i++)
        fast_turning[i]->setVisible(state);
}
#define copy_if_not_empty(string, wops)if(string!=""){wops = strdup(string.toUtf8().data()); }
bool searchWindow::fillOps(options&ops){
    QString str = this->search.line->text();
    if(!state && str=="")
        return false;
    copy_if_not_empty(str, ops.str);
    if(state){
        str = this->numberLine->text();
        if(str != ""){
            std::string string = str.toStdString();
            SCREEN->clicked_pattern(string);
            return false;
        }
        str = this->author_line->text();
        copy_if_not_empty(str, ops.author);
        str = this->countryChoice->currentText();
        if(str!="ALL")
            ops.country = strdup(str.toUtf8().data());
        if(datetypChoice->currentIndex()==0){
            if(dateChoice->date().year()<=2023)
                ops.date_publish = strdup(dateChoice->text().toUtf8().data());
        }
        else{
            if(dateChoice->date().year()<=2023)
                ops.date_applicc = strdup(dateChoice->text().toUtf8().data());
        }
    }
    return true;
}
searchScroll::searchScroll(QWidget*parent, QVBoxLayout*screenLay):QScrollArea(parent){
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wlay = new QWidget();
    lay = new QVBoxLayout();
    wlay->setLayout(lay);
    this->setWidget(wlay);
    this->setWidgetResizable(true);
    search = new searchWindow(parent, screenLay);
    screenLay->addWidget(this);
};
patentWidget::patentWidget(const fastview_pattent&fp){
    this->setStyleSheet(BACKGROUND_COLOR_PATIENT);
    this->title = new clickableLabel(fp.title.c_str(), this, this);
    this->title->setWordWrap(true);
    this->descr = new clickableLabel(fp.descr.c_str(),this, this);
    this->descr->setWordWrap(true);
    this->date_publish = new clickableLabel(fp.date_publish.c_str(),this, this);
    this->inventor = new clickableLabel(fp.inventor.c_str(),this, this);
    this->inventor->setWordWrap(true);
    this->number = new clickableLabel(fp.number.c_str(),this, this);
    this->top = new QHBoxLayout();
    this->bottom = new QHBoxLayout();
    this->lay = new QVBoxLayout(this);
    top->addWidget(this->title);
    top->addStretch(1);
    //top->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    top->addWidget(this->number);
    bottom->addWidget(this->inventor);
    bottom->addStretch(1);
    bottom->addWidget(this->date_publish);
    lay->addLayout(top);
    lay->addWidget(this->descr);
    lay->addLayout(bottom);
    this->setLayout(lay);
}
void patentWidget::mouseReleaseEvent(QMouseEvent *event){
    SCREEN->clicked_pattern(this->number->text().toStdString());
}
theScreen::theScreen(){
    SCREEN = this;
    layout = new QVBoxLayout(this);
    mainScroll = new searchScroll(this, layout);
    screenPortal = new portal(layout, nullptr);
    this->setLayout(layout);
    connect(mainScroll->search->getSearchB(), &QPushButton::clicked, this, &theScreen::clicked_search);
}
patentScroll::patentScroll(const std::string&number){
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->iw = new infoWindow();
    this->iw->fillIt(number);
    this->container = new QWidget();
    this->lay  =new QVBoxLayout();
    this->butts = new QHBoxLayout();
    this->menuB = new QPushButton("<-");
    this->closeB = new QPushButton("X");
    this->butts->addWidget(menuB);
    this->butts->addStretch(1);
    this->butts->addWidget(closeB);
    this->lay->addLayout(this->butts);
    this->iw->addToLayout(this->lay);

    this->container->setLayout(this->lay);
    this->setWidget(this->container);
    connect(menuB, &QPushButton::clicked, this, &patentScroll::clickedMenu);
    connect(closeB, &QPushButton::clicked, this, &patentScroll::clickedClose);
}
void patentScroll::clickedClose(){
    SCREEN->removeScroll(this);
    delete this->iw;
    delete this;
}
void patentScroll::clickedMenu(){
    SCREEN->clicked_portal(nullptr);
}
void theScreen::clicked_pattern(const std::string&number){
    mainScroll->hide();//FIXME
    patentScroll* ps = new patentScroll(number);
    addScrolls.push_back(ps);
    this->layout->insertWidget(0, ps);
    screenPortal->createOne(true, ps);
}
void theScreen::removeScroll(patentScroll*ps){
    for(auto it = addScrolls.begin(); it != addScrolls.end(); ++it){
        if((*it) == ps){
            addScrolls.erase(it);
            break;
        }
    }
    this->layout->removeWidget(ps);
    screenPortal->removePortalTo(ps);
    this->mainScroll->unhide();
}
void theScreen::clicked_search(){
    options ops;
    if(!(mainScroll->search->fillOps(ops)))
        return;
    std::vector<fastview_pattent> fillTo;
    if(search_pattern(ops, fillTo) == H_OK){
        this->mainScroll->init_search_results(fillTo);
    }
    if(ops.str)
        free(ops.str); //free bcs it's strdup. WTFF???!
    if(ops.author)
        free(ops.author);
    if(ops.country)
        free(ops.country);
    if(ops.date_publish)
        free(ops.date_publish);
    if(ops.date_applicc)
        free(ops.date_applicc);
    if(ops.id_document)
        free(ops.id_document);
    if(ops.typesort)
        free(ops.typesort);
}
bool theScreen::clicked_portal(patentScroll*ps){
    patentScroll*lastActive = this->screenPortal->activePatent;
    if(ps == lastActive)
        return false;
    this->screenPortal->makeNoneActive();
    screenPortal->activePatent = ps;
    if(ps == nullptr){
       lastActive->hide();
       mainScroll->unhide();
    }
    else{
        if(lastActive==nullptr)
            mainScroll->hide();
        else
            lastActive->hide();
        ps->setVisible(true);
    }
    return true;
}
void searchScroll::init_search_results(const std::vector<fastview_pattent>& vfp){
    if(!list.empty()){
        QLayoutItem*item;
        while((item = lay->takeAt(0))!=nullptr){
            lay->removeItem(item);
            delete item;
        }
        for(patentWidget* x:list)
            delete x;
        list.clear();
        delete lay;
        lay = new QVBoxLayout(wlay);
        wlay->setLayout(lay);
    }
    for(int i = 0; i < vfp.size(); i++){
        patentWidget*pw =new patentWidget(vfp[i]);
        list.push_back(pw);
//        testla->addWidget(pw);
        lay->addWidget(pw);
    }
}
QPushButton* portal::at(int index){
    if(buttons.size()>index)
        return buttons[index];
    return nullptr;
}
void portalButton::portalClicked(){
    if(SCREEN->clicked_portal(myPatent)){
        this->active = true;
        this->setStyleSheet(PORTAL_BUTTON_ACTIVE);
    }
}
void portal::makeNoneActive(){
    for(auto x:buttons){
        if(x->active){
            x->setStyleSheet(PORTAL_BUTTON_INACTIVE);
            x->active = false;
            break;
        }
    }
}
void portal::createOne(bool isActive, patentScroll*ps){
    portalButton*pb = new portalButton(ps);
    buttons.push_back(pb);
    pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pb->setMinimumSize(QSize(30,30));
    pb->setMaximumSize(QSize(30, 30));
    if(isActive){
        if(ps)//it's not mainScroll
            makeNoneActive();
        pb->setStyleSheet(PORTAL_BUTTON_ACTIVE);
        this->activePatent = ps;
    }
    else
        pb->setStyleSheet(PORTAL_BUTTON_INACTIVE);
    pb->active = isActive;
    layout->addWidget(pb);
    layout->invalidate();
}
void portal::removePortalTo(patentScroll*ps){
    for(auto it = buttons.begin(); it != buttons.end(); it++){
        if((*it)->myPatent == ps){
            this->layout->removeWidget(*it);
            delete (*it);
            this->buttons.erase(it);
            break;
        }
    }
    this->buttons[0]->active = true;
    this->buttons[0]->setStyleSheet(PORTAL_BUTTON_ACTIVE);
    this->activePatent = nullptr;
}
portal::portal(QVBoxLayout*lay, patentScroll*ps){
    layout = new QHBoxLayout();
    this->activePatent = ps;
    createOne(true, ps);
    lay->addLayout(layout);
};
