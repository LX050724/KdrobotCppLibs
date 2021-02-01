#ifndef KDROBOTCPPLIBS_MAINTHREAD_H
#define KDROBOTCPPLIBS_MAINTHREAD_H

#include <QObject>
#include <QThread>
#include <spdlogger.h>
#include "JsonConfig.h"

/**
 * 主线程构造模板应继承该类然后重写main方法
 * @brief 使用方法：
 * <code>
 *  class MyMainThread : public MainThread {
 *  Q_OBJECT
 *  public:
 *      using MainThread::MainThread;
 *  protected:
 *      int main(const QStringList &args) override {
 *          logger.info("Hello World");
 *          return 0;
 *      }
 *  };
 *
 *  int main(int argc, char *argv[]) {
 *      QCoreApplication app(argc, argv);
 *      MyMainThread myMainThread(app.arguments(), &app);
 *      QObject::connect(&myMainThread, SIGNAL(finished()), &app, SLOT(quit()));
 *      return app.exec();
 *  }
 * </code>
 * main方法运行于一个新的线程内，此时Qt框架已经启动
 */
class MainThread : public QThread {
Q_OBJECT
protected:
    spdlogger logger;
    JsonConfig config;
    QStringList args;
    volatile bool running = true;
public:
    MainThread(const QStringList &args, QObject *parent = nullptr);

    virtual ~MainThread();

protected:
    void run() override final;

    virtual void main(const QStringList &args) = 0;

signals:
    int finished();
};

#endif //KDROBOTCPPLIBS_MAINTHREAD_H
