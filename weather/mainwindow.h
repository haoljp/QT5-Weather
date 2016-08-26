#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QPixmap>
#include<QPaintEvent>
#include<QPainter>
#include<QPoint>
#include<QMouseEvent>
#include<QLabel>

namespace Ui {
class MainWindow;
}

struct Forecast
{
    QString fengxiang;
    QString fengli;
    QString high;
    QString type;
    QString low;
    QString date;
};

struct Today
{
    QString ganmao;
    QString city;
    QString updatetime;
    QString wendu;
    QString fengli;
    QString fengxiang;
    QString sunrise;
    QString sunset;
    QString shidu;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
protected slots:
    void replayFinished(QNetworkReply *reply);

private slots:
    void on_getButton_clicked();
    void on_updateButton_clicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QString URL_1;
    QString URL_2;

    Forecast forecast[5];
    Today today;

    void parseJson(QString Json);
    void parseXml(QString Xml);
    void parseCity(QString City);

    int choose;//0.查询IP 1.今天天气+指数 2.未来5天天气

    QPixmap pixmap;
    QPoint move_point;
    bool mouse_press;

    QList<QLabel *> forecast_date_list;
    QList<QLabel *> forecast_temp_list;
    QList<QLabel *> forecast_type_list;

    QString city;
    int uid;
};

#endif // MAINWINDOW_H
