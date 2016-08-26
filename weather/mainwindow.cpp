#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QVariant>
#include<QByteArray>
#include<QJsonParseError>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonValue>
#include<QXmlStreamReader>
#include<QMessageBox>
//#include<QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    pixmap.load(":/images/UI0");
    resize(pixmap.size());
    uid = 0;
    mouse_press = false;

    forecast_date_list<<ui->forecast_0_date<<ui->forecast_1_date<<ui->forecast_2_date<<ui->forecast_3_date<<ui->forecast_4_date;
    forecast_temp_list<<ui->forecast_0_temp<<ui->forecast_1_temp<<ui->forecast_2_temp<<ui->forecast_3_temp<<ui->forecast_4_temp;
    forecast_type_list<<ui->forecast_0_type<<ui->forecast_1_type<<ui->forecast_2_type<<ui->forecast_3_type<<ui->forecast_4_type;

    manager = new QNetworkAccessManager(this);
    //天气API
    URL_1 = "http://wthrcdn.etouch.cn/weather_mini?city=";
    URL_2 = "http://wthrcdn.etouch.cn/WeatherApi?city=";
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replayFinished(QNetworkReply*)));

    //设置组件样式
    ui->cityEdit->setTextMargins(0, 0, ui->getButton->width(), 0);
    ui->cityEdit->setStyleSheet("QLineEdit{border: 1px solid gray;border-radius: 3px;background:rgb(200, 231, 232);} QLineEdit:hover{border-color:transparent; }");
    ui->getButton ->setStyleSheet("background:transparent;");
    ui->closeButton->setStyleSheet("QPushButton {border-image:url(:/images/close);}QPushButton:hover{border-image:url(:/images/close_on);}QPushButton:hover:pressed{border-image:url(:/images/close);}");
    ui->minButton->setStyleSheet("QPushButton {border-image:url(:/images/min);}QPushButton:hover{border-image:url(:/images/min_on);}QPushButton:hover:pressed{border-image:url(:/images/min);}");
    ui->updateButton->setStyleSheet("QPushButton {border-image:url(:/images/update);}QPushButton:hover{border-image:url(:/images/update_on);}QPushButton:hover:pressed{border-image:url(:/images/update);}");

    QDateTime time = QDateTime::currentDateTime();
    ui->date->setText(tr("%1").arg(time.toString("yyyy-MM-dd")));

    //启动程序后先查询ip定位默认城市
    choose = 0; 
    manager->get(QNetworkRequest(QUrl("http://int.dpool.sina.com.cn/iplookup/iplookup.php")));
    //manager->get(QNetworkRequest(QUrl("http://pv.sohu.com/cityjson?ie=utf-8")));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);//绘制UI
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //qDebug()<<"press";
    if(event->button() == Qt::LeftButton)
    {
        //鼠标相对于窗体的位置）
        move_point = event->pos();
        mouse_press = true;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mouse_press)
    {
        //qDebug()<<"move";
        //鼠标相对于屏幕的位置
        QPoint move_pos = event->globalPos();
        //移动主窗体
        this->move(move_pos - move_point);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
    //qDebug()<<"release";
    mouse_press = false;
}

void MainWindow::replayFinished(QNetworkReply *reply)
{
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        //QString result(bytes);
        //qDebug()<<result;
        if(choose == 0)//查询IP
        { 
            QString result = QString::fromLocal8Bit(bytes);
            parseCity(result);
        }
        else if(choose == 1)//今天天气+指数
        {
            QString result(bytes);
            //qDebug()<<result;
            parseXml(result);
        }
        else if(choose == 2)//未来5天天气
        {
            QString result(bytes);
            //qDebug()<<result;
            parseJson(result);
        }
    }
    else
        QMessageBox::information(this,tr("出错啦"),tr("网络错误,请检查网络连接"),QMessageBox::Ok,QMessageBox::Ok);
        //qDebug()<<"网络出错\n";
}

//解析城市信息
void MainWindow::parseCity(QString City)
{
    city = City.split(tr("	")).at(5);
    if(city == "")
    {
        QMessageBox::information(this,tr("提示"),tr("无法定位城市,请手动查询"),QMessageBox::Ok,QMessageBox::Ok);
        return;
    }
    QString url = URL_1 + city;
    choose = 2;
    manager->get(QNetworkRequest(QUrl(url)));
}

//解析json格式的未来天气
void MainWindow::parseJson(QString Json)
{
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(Json),&json_error);

    if(json_error.error == QJsonParseError::NoError)
    {
        QJsonObject obj = parse_doucment.object();

        QJsonValue desc = obj.take("desc");
        if(desc.toString() != "OK")
        {
            //qDebug()<<"城市错误\n";
            QMessageBox::information(this,tr("抱歉"),tr("暂无此城市的天气情况"),QMessageBox::Ok,QMessageBox::Ok);
            return;
        }

        QJsonValue data = obj.take("data");
        today.ganmao = data.toObject().take("ganmao").toString();
        QJsonValue forecast1 = data.toObject().take("forecast");
        QJsonArray forecast2 = forecast1.toArray();
        for(int i=0; i<5; i++)
        {
            QJsonValue value = forecast2.at(i);
            QJsonObject object = value.toObject();
            forecast[i].fengxiang = object.take("fengxiang").toString();
            forecast[i].date = object.take("date").toString();
            forecast[i].fengli = object.take("fengli").toString();
            forecast[i].high = object.take("high").toString();
            forecast[i].low = object.take("low").toString();
            forecast[i].type = object.take("type").toString();
        }

        for(int i=0;i<5;i++)
        {
            forecast_date_list[i]->setText(tr("%1").arg(forecast[i].date));
            forecast_temp_list[i]->setText(tr("%1 - %2").arg(forecast[i].low.split(" ").at(1)).arg(forecast[i].high.split(" ").at(1)));
            forecast_type_list[i]->setPixmap(QPixmap(tr(":/images/%1").arg(forecast[i].type)));
            forecast_type_list[i]->setToolTip(tr("%1 : %2 - %3").arg(forecast[i].type).arg(forecast[i].fengli).arg(forecast[i].fengxiang));
        }
        ui->forecast_0_date->setText(tr("今天"));

        QString url = URL_2 + city;
        choose = 1;
        manager->get(QNetworkRequest(QUrl(url)));

    }
    else
    {
        //qDebug()<<"Json错误";
        QMessageBox::information(this,tr("出错啦"),tr("数据出错,请重试"),QMessageBox::Ok,QMessageBox::Ok);
        return;
    }
}

//解析XML格式的今天天气
void MainWindow::parseXml(QString Xml)
{
    QXmlStreamReader xml(Xml);

    while(!xml.atEnd())
    {
        if(xml.hasError())
        {
            QMessageBox::information(this,tr("出错啦"),tr("数据出错,请重试"),QMessageBox::Ok,QMessageBox::Ok);
            return;
        }
        else if(xml.isStartElement())
        {
            if(xml.name()=="city")
            {
                today.city = xml.readElementText();
            }
            else if(xml.name()=="updatetime")
            {
                today.updatetime = xml.readElementText();
            }
            else if(xml.name()=="wendu")
            {
                today.wendu = xml.readElementText();
            }
            else if(xml.name()=="fengli")
            {
                today.fengli = xml.readElementText();
            }
            else if(xml.name()=="shidu")
            {
                today.shidu = xml.readElementText();
            }
            else if(xml.name()=="fengxiang")
            {
                today.fengxiang = xml.readElementText();
            }
            else if(xml.name()=="sunrise_1")
            {
                today.sunrise = xml.readElementText();
            }
            else if(xml.name()=="sunset_1")
            {
                today.sunset = xml.readElementText();
                xml.clear();

                ui->city->setText(tr("%1").arg(today.city));
                ui->temp->setText(tr("%1℃").arg(today.wendu));
                ui->sunrise->setText(tr("%1").arg(today.sunrise));
                ui->sunset->setText(tr("%1").arg(today.sunset));
                ui->label->setText(tr("日出"));
                ui->label_2->setText(tr("日落"));
                ui->label_3->setText(tr("湿度"));
                ui->shidu->setText(tr("%1").arg(today.shidu));
                ui->fengli->setText(tr("%1").arg(today.fengli));
                ui->fengxiang->setText(tr("%1").arg(today.fengxiang));
                ui->label_4->setText(tr("感\n冒\n指\n数"));
                ui->ganmao->setText(tr("%1").arg(today.ganmao));

                return;
            }
            else
                xml.readNext();
        }
        else
            xml.readNext();
    }
    xml.clear();

}

//搜索框查询天气
void MainWindow::on_getButton_clicked()
{
    if(ui->cityEdit->text().isEmpty())
        return;
    city = ui->cityEdit->text();
    QString url = URL_1 + city;
    choose = 2;
    manager->get(QNetworkRequest(QUrl(url)));
}

//简单换肤
void MainWindow::on_updateButton_clicked()
{
    //选择UI的id
    if(uid == 3)
        uid = 0;
    else
        uid = uid + 1;
    //拼凑成UI路径
    QString UIpath = tr(":/images/UI%1").arg(uid);
    //qDebug()<<UIpath;
    //加载UI
    pixmap.load(UIpath);
    //产生paintEvent重绘UI
    update();
}
