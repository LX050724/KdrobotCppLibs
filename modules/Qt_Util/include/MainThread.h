/**
 * @file MainThread.h
 * @author yao
 * @date 2021年1月13日
 */

#ifndef KDROBOTCPPLIBS_MAINTHREAD_H
#define KDROBOTCPPLIBS_MAINTHREAD_H

#include <QObject>
#include <QThread>
#include <spdlogger.h>
#include "JsonConfig.h"

/**
 * @class MainThread
 * @brief 程序人口模板
 * @details
 *     使用方法：主线程构造模板应继承该类然后重写main方法
 *     {@code
 *      class MyMainThread : public MainThread {
 *      Q_OBJECT
 *      public:
 *          using MainThread::MainThread;
 *      protected:
 *          void main(const QStringList &args) override {
 *              logger.info("Hello World");
 *          }
 *      };}
 *      主函数写法：
 *      {@code
 *      int main(int argc, char *argv[]) {
 *          QCoreApplication app(argc, argv);
 *          MyMainThread myMainThread(app.arguments(), &app);
 *          QObject::connect(&myMainThread, SIGNAL(finished()), &app, SLOT(quit()));
 *          return app.exec();
 *      }}
 * main方法运行于一个新的线程内，此时Qt框架已经启动
 */
class MainThread : public QThread {
Q_OBJECT
protected:
    spdlogger logger;       ///<@brief 日志器
    JsonConfig config;      ///<@brief json配置
    QStringList args;       ///<@brief 命令行参数
    volatile bool running = true;   ///<@brief 用于控制线程退出的标志位,初始默认值为true
public:

    /**
     * 构造函数
     * @details
     * 执行的功能有
     * 1. 检查命令行参数-l <file>或--log <file>指定全局日期输出到文件
     * 2. 检查命令行参数-c <file>或--config <file>指定配置文件路径
     * 3. 启动线程调用main方法
     * @param args 命令行参数
     * @param parent 父对象指针,可有可无
     */
    MainThread(const QStringList &args, QObject *parent = nullptr);

    /**
     * @brief 析构函数
     * @details 可以重写以达到自己的功能,但是必须确保线程退出
     *
     * 示例:{@code
     * if (this->isRunning() && running) {
     *     running = false;
     *     this->quit();
     *     this->wait();
     * }}
     */
    virtual ~MainThread();

protected:
    void run() override final;

    /**
     * 主方法纯虚函数,需要继承并实现
     * @details 方法退出时会发出信号量finished, running赋值false
     * @param args 命令行参数
     */
    virtual void main(const QStringList &args) = 0;

signals:

    /**
     * @brief 线程退出信号量
     */
    int finished();
};

#endif //KDROBOTCPPLIBS_MAINTHREAD_H
